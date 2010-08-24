/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
#include "scan2d_dyn_obj_recog.h"
#include <main/argopts.h>

//
// data structures
//

arg_table_t argTab[] = {

    { ARGOPT_OPT, "scan2dSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the scan2d module", { 0 } },

    { ARGOPT_REQ, "scan2dInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the scan2d module", { -1 } },

    { ARGOPT_OPT, "objRecogSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the objRecog module", { 0 } },

    { ARGOPT_REQ, "objRecogInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the objRecog module", { -1 } },

    { ARGOPT_OPT, "vMin", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The minimum velocity for dynamic classification", { 300 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};

/*******************************************************************************
 *   !!! REALTIME CONTEXT !!!
 *
 *   moduleOn,
 *   moduleOff,
 *   moduleLoop,
 *   moduleCommand,
 *
 *   own realtime user functions
 ******************************************************************************/
 int  Scan2dDynObjRecog::moduleOn(void)
{
    int ret;

    // get dynamic module parameter
    vMin                  = getInt32Param("vMin");

    GDOS_DBG_INFO("Turn on Scan2d(%d/%d)...\n", scan2dSys, scan2dInst);
    ret = scan2d->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on Scan2d(%d/%d), code = %d \n", scan2dSys, scan2dInst, ret);
        return ret;
    }

    GDOS_DBG_INFO("Turn on ObjRecog(%d/%d)...\n", objRecogSys, objRecogInst);
    ret = objRecog->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on ObjRecog(%d/%d), code = %d\n", objRecogSys, objRecogInst, ret);
        return ret;
    }


    dataMbx.clean();
    GDOS_DBG_INFO("Requesting continuous data from Scan2d(%d/%d)...\n", scan2dSys, scan2dInst);
    ret = scan2d->getContData(0, &dataMbx, &dataBufferPeriodTime);
    if (ret)
    {
        GDOS_ERROR("Can't get continuous data from Scan2d(%d/%d), "
                   "code = %d \n", scan2dSys, scan2dInst, ret);
        return ret;
    }

    return RackDataModule::moduleOn();  // has to be last command in moduleOn();
}

void Scan2dDynObjRecog::moduleOff(void)
{
    RackDataModule::moduleOff();        // has to be first command in moduleOff();

    scan2d->stopContData(&dataMbx);
}

int  Scan2dDynObjRecog::moduleLoop(void)
{
    int             ret;
    scan2d_data     *scan2dIn;
    scan2d_data     *scan2dOut;
    RackMessage     msgInfo;

    // get datapointer from rackdatabuffer
    scan2dOut = (scan2d_data *)getDataBufferWorkSpace();

    // get scan2d data
    ret = dataMbx.peekTimed(rackTime.toNano(2 * dataBufferPeriodTime), &msgInfo);
    if (ret)
    {
        GDOS_ERROR("Can't receive scan2d data on DATA_MBX, code = %d \n", ret);
        return ret;
    }

    if ((msgInfo.getType() != MSG_DATA) ||
        (msgInfo.getSrc()  != scan2d->getDestAdr()))
    {
        GDOS_ERROR("Received unexpected message from %n to %n type %d on "
                   "data mailbox\n", msgInfo.getSrc(), msgInfo.getDest(), msgInfo.getType());

        dataMbx.peekEnd();
        return -EINVAL;
    }

    scan2dIn = Scan2dData::parse(&msgInfo);

    if (scan2dIn->pointNum > SCAN2D_POINT_MAX)
    {
        GDOS_ERROR("PointNum (%d) exceeds max array size (%d)\n", scan2dIn->pointNum,
                   SCAN2D_POINT_MAX);
        dataMbx.peekEnd();
        return -EINVAL;
    }

    memcpy(scan2dOut, scan2dIn, sizeof(scan2d_data) + scan2dIn->pointNum * sizeof(scan_point));
    dataMbx.peekEnd();

    ret = objRecog->getData(&objRecogMsg.data, sizeof(obj_recog_data_msg), scan2dIn->recordingTime);
    if (ret)
    {
        GDOS_ERROR("Can't get data from ObjRecog(%d/%d), code = %d\n", objRecogSys, objRecogInst, ret);
        return ret;
    }

    classifyDynamic(scan2dOut, &objRecogMsg.data, vMin);

    GDOS_DBG_DETAIL("RecordingTime %u pointNum %d\n",
                    scan2dOut->recordingTime, scan2dOut->pointNum);

    putDataBufferWorkSpace(Scan2dData::getDatalen(scan2dOut));
    return 0;
}

int  Scan2dDynObjRecog::moduleCommand(RackMessage *msgInfo)
{
    // not for me -> ask RackDataModule
    return RackDataModule::moduleCommand(msgInfo);
}

void Scan2dDynObjRecog::classifyDynamic(scan2d_data *scan2dData, obj_recog_data *objRecogData, int vMin)
{
    int                     i, j, k, kParent;
    int                     vCurr;
    double                  rotMat[2][2];
    point_2d                boxCorners[4];
    int                     crossL, crossR;
    bool                    stradL, stradR;
    double                  x;
    point_2d                point, pointParent;

    // loop for all objects
    for (i = 0; i < objRecogData->objectNum; i++)
    {
        // get current object speed
        vCurr = (int)sqrt((double)objRecogData->object[i].vel.x * (double)objRecogData->object[i].vel.x +
                          (double)objRecogData->object[i].vel.y * (double)objRecogData->object[i].vel.y);

        // proceed if object is dynamic
        if (vCurr >= vMin)
        {
            // calculate edge points of bounding box

        	// rotate unit vectors to orientation of box
        	rotMat[0][0] =  cos(objRecogData->object[i].pos.rho);
        	rotMat[0][1] = -sin(objRecogData->object[i].pos.rho);
        	rotMat[1][0] =  sin(objRecogData->object[i].pos.rho);
        	rotMat[1][1] =  cos(objRecogData->object[i].pos.rho);

            // point left-up
        	point.x         = -objRecogData->object[i].dim.x / 2;
        	point.y         =  objRecogData->object[i].dim.y / 2;
        	boxCorners[0].x = (int)(rotMat[0][0] * (double)point.x + rotMat[0][1] * (double)point.y) ;
        	boxCorners[0].y = (int)(rotMat[1][0] * (double)point.x + rotMat[1][1] * (double)point.y) ;
        	boxCorners[0].x += objRecogData->object[i].pos.x;
        	boxCorners[0].y += objRecogData->object[i].pos.y;

        	// point right-up
        	point.x         =  objRecogData->object[i].dim.x / 2;
        	point.y         =  objRecogData->object[i].dim.y / 2;
        	boxCorners[1].x = (int)(rotMat[0][0] * (double)point.x + rotMat[0][1] * (double)point.y) ;
        	boxCorners[1].y = (int)(rotMat[1][0] * (double)point.x + rotMat[1][1] * (double)point.y) ;
        	boxCorners[1].x += objRecogData->object[i].pos.x;
        	boxCorners[1].y += objRecogData->object[i].pos.y;

            // point right-down
        	point.x         =  objRecogData->object[i].dim.x / 2;
        	point.y         = -objRecogData->object[i].dim.y / 2;
        	boxCorners[2].x = (int)(rotMat[0][0] * (double)point.x + rotMat[0][1] * (double)point.y) ;
        	boxCorners[2].y = (int)(rotMat[1][0] * (double)point.x + rotMat[1][1] * (double)point.y) ;
        	boxCorners[2].x += objRecogData->object[i].pos.x;
        	boxCorners[2].y += objRecogData->object[i].pos.y;

            // point left-down
        	point.x         = -objRecogData->object[i].dim.x / 2;
        	point.y         = -objRecogData->object[i].dim.y / 2;
        	boxCorners[3].x = (int)(rotMat[0][0] * (double)point.x + rotMat[0][1] * (double)point.y) ;
        	boxCorners[3].y = (int)(rotMat[1][0] * (double)point.x + rotMat[1][1] * (double)point.y) ;
        	boxCorners[3].x += objRecogData->object[i].pos.x;
        	boxCorners[3].y += objRecogData->object[i].pos.y;

            // loop for all scan points
            for (j = 0; j < scan2dData->pointNum; j++)
            {
                // reinit variables
                crossL = 0;
                crossR = 0;

                // loop for all four edges
                for (k = 0; k < 4; k++)
                {
                    kParent = k - 1;
                    if (kParent < 0)
                    {
                        kParent += 4;
                    }

                    // shift the points so that scan point is the origin
                    point.x       = boxCorners[k].x       - scan2dData->point[j].x;
                    point.y       = boxCorners[k].y       - scan2dData->point[j].y;
                    pointParent.x = boxCorners[kParent].x - scan2dData->point[j].x;
                    pointParent.y = boxCorners[kParent].y - scan2dData->point[j].y;

                    if ((point.x != 0) && (point.y != 0))
                    {
                        // check if edge straddles x axis with bias above/below
                        stradR = ((point.y > 0) != (pointParent.y > 0));
                        stradL = ((point.y < 0) != (pointParent.y < 0));

                        if ((stradL) ||(stradR))
                        {
                            x = ((double)point.x * (double)pointParent.y -
                                 (double)point.y * (double)pointParent.x) /
                                 (double)(pointParent.y - point.y);

                            if ((stradL) && (x < 0))
                            {
                                crossL++;
                            }
                            if ((stradR) && (x > 0))
                            {
                                crossR++;
                            }
                        }
                    }
                }

                // pos is on an edge of the object if crossL/R counts are not of the same parity
                if ((crossL % 2) != (crossR % 2))
                {
                    scan2dData->point[j].type |= SCAN_POINT_TYPE_DYN_OBSTACLE;
                }

                // pos is inside object if the number of crossings is odd
                if ((crossR % 2) == 1)
                {
                    scan2dData->point[j].type |= SCAN_POINT_TYPE_DYN_OBSTACLE;
                }
            }
        }
    }
}

/*******************************************************************************
 *   !!! NON REALTIME CONTEXT !!!
 *
 *   moduleInit,
 *   moduleCleanup,
 *   Constructor,
 *   Destructor,
 *   main,
 *
 *   own non realtime user functions
 ******************************************************************************/
// init_flags
#define INIT_BIT_DATA_MODULE        0
#define INIT_BIT_MBX_WORK           1
#define INIT_BIT_MBX_DATA           2
#define INIT_BIT_PROXY_SCAN2D       3
#define INIT_BIT_PROXY_OBJ_RECOG    4

int Scan2dDynObjRecog::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    // work mailbox
    ret = createMbx(&workMbx, 1, sizeof(obj_recog_data_msg),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // data mailbox
    ret = createMbx(&dataMbx, 1, sizeof(scan2d_data_msg),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_DATA);

    // create Scan2d Proxy
    scan2d = new Scan2dProxy(&workMbx, scan2dSys, scan2dInst);
    if (!scan2d)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_SCAN2D);

    // create ObjRecog Proxy
    objRecog = new ObjRecogProxy(&workMbx, objRecogSys, objRecogInst);
    if (!objRecog)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_OBJ_RECOG);

    return 0;

init_error:
    // !!! call local cleanup function !!!
    Scan2dDynObjRecog::moduleCleanup();
    return ret;
}

void Scan2dDynObjRecog::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // free proxies
    if (initBits.testAndClearBit(INIT_BIT_PROXY_OBJ_RECOG))
    {
        delete objRecog;
    }

    if (initBits.testAndClearBit(INIT_BIT_PROXY_SCAN2D))
    {
        delete scan2d;
    }

    // delete mailboxes

    if (initBits.testAndClearBit(INIT_BIT_MBX_DATA))
    {
        destroyMbx(&dataMbx);
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }
}

Scan2dDynObjRecog::Scan2dDynObjRecog(void)
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    240,              // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    10,               // max buffer entries
                    10)               // data buffer listener
{
    // get static module parameter
    scan2dSys       = getIntArg("scan2dSys", argTab);
    scan2dInst      = getIntArg("scan2dInst", argTab);
    objRecogSys     = getIntArg("objRecogSys", argTab);
    objRecogInst    = getIntArg("objRecogInst", argTab);

    dataBufferMaxDataSize = sizeof(scan2d_data_msg);
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "Scan2dDynObjRecog");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new Scan2dDynObjRecog
    Scan2dDynObjRecog *pInst;

    pInst = new Scan2dDynObjRecog();
    if (!pInst)
    {
        printf("Can't create new Scan2dDynObjRecog -> EXIT\n");
        return -ENOMEM;
    }

    // init
    ret = pInst->moduleInit();
    if (ret)
        goto exit_error;

    pInst->run();

    return 0;

exit_error:
    delete (pInst);
    return ret;
}

