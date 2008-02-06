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
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#include "scan2d_merge.h"

#include <main/argopts.h>

// init_flags
#define INIT_BIT_DATA_MODULE        0
#define INIT_BIT_MBX_WORK           1
#define INIT_BIT_MBX_DATA           2
#define INIT_BIT_PROXY_ODOMETRY     5
#define INIT_BIT_PROXY_SCAN2D       6       // has to be highest bit!


//
// data structures
//

Scan2dMerge *p_inst;

argTable_t argTab[] = {

    { ARGOPT_OPT, "odometryInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the odometry module", { 0 } },

    { ARGOPT_REQ, "scan2dInstA", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of a scan2d module", { -1 } },

    { ARGOPT_OPT, "scan2dInstB", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of a scan2d module", { -1 } },

    { ARGOPT_OPT, "scan2dInstC", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of a scan2d module", { -1 } },

    { ARGOPT_OPT, "scan2dInstD", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of a scan2d module", { -1 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};


/*******************************************************************************
 *   !!! REALTIME CONTEXT !!!
 *
 *   moduleOn,
 *   moduleOff,
 *   moduleLoop,
 *   moduleCommand,
 *   initDataBuffer
 *
 *   own realtime user functions
 ******************************************************************************/
int  Scan2dMerge::moduleOn(void)
{
    int ret, k;

    // turn on odometry
    GDOS_DBG_DETAIL("Turn on odometry(%d)\n", odometryInst);
    ret = odometry->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on odometry(%d), code = %d\n",
                   odometryInst, ret);
        return ret;
    }

    // turn on scan2d modules
    for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
    {
        if (scan2dInst[k] >= 0)
        {
            GDOS_DBG_DETAIL("Turn on scan2d(%d)\n", scan2dInst);
            ret = scan2d[k]->on();
            if (ret)
            {
                GDOS_ERROR("Can't turn on scan2d(%i), code = %d\n",
                           scan2dInst[k], ret);
                return ret;
            }
        }
    }


    // get continuous data from odometry
    ret = odometry->getContData(0, &dataMbx, &dataBufferPeriodTime);
    if (ret)
    {
        GDOS_ERROR("Can't get continuous data from odometry(%i), code = %d\n",
                   odometryInst, ret);
        return ret;
    }

    // get continuous data from scan2d modules
    for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
    {
        if (scan2dInst[k] >= 0)
        {
            ret = scan2d[k]->getContData(0, &dataMbx, NULL);
            if (ret)
            {
                GDOS_ERROR("Can't get continuous data from scan2d(%i), "
                           "code = %d\n", scan2dInst[k], ret);
                return ret;
            }
        }
        scanBuffer[k].data.pointNum = 0;
        scan2dTimeout[k]            = 0;
    }

    return RackDataModule::moduleOn();  // has to be last command in moduleOn();
}

void Scan2dMerge::moduleOff(void)
{
    int     k;

    RackDataModule::moduleOff();        // has to be first command in moduleOff();

    odometry->stopContData(&dataMbx);

    for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
    {
        if (scan2dInst[k] >= 0)
        {
            scan2d[k]->stopContData(&dataMbx);
        }
    }
}


// realtime context
int  Scan2dMerge::moduleLoop(void)
{
    message_info    dataInfo;
    odometry_data   *odoData   = NULL;
    scan2d_data     *scanData  = NULL;
    scan2d_data     *mergeData = NULL;
    ssize_t         datalength = 0;
    int             ret;
    int             i, j, k;
    int             x, y;
    int             posDiffX, posDiffY;
    double          sinRho, cosRho;

    // receive data
    ret = dataMbx.peekTimed(1000000000llu, &dataInfo); // 1s
    if (ret)
    {
        GDOS_ERROR("Can't receive data on MBX, code = %d\n", ret);
        return ret;
    }

    if (dataInfo.type == MSG_DATA)
    {
        // store new scan2d data message from scan2dInst 0, 1, 2, ...
        for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
        {
            if (scan2dInst[k] >= 0)
            {
                // message received
                if ((dataInfo.src == RackName::create(SCAN2D, scan2dInst[k])) &&
                    (dataInfo.type == MSG_DATA))
                {
                    // message parsing
                    scanData = Scan2dData::parse(&dataInfo);

                    ret = odometry->getData(&odometryBuffer[k],
                                            sizeof(odometry_data),
                                            scanData->recordingTime);

                    sinRho = sin(odometryBuffer[k].pos.rho);
                    cosRho = cos(odometryBuffer[k].pos.rho);

                    j = 0;

                    for (i = 0; i < scanData->pointNum; i++)
                    {
                        if ((scanData->point[i].type & TYPE_MASK) != TYPE_LANDMARK)
                        {
                            scanBuffer[k].point[j].x  = (int)(scanData->point[i].x *
                                                              cosRho) -
                                                        (int)(scanData->point[i].y *
                                                              sinRho);
                            scanBuffer[k].point[j].y  = (int)(scanData->point[i].x *
                                                              sinRho) +
                                                        (int)(scanData->point[i].y *
                                                              cosRho);
                            scanBuffer[k].point[j].z  = scanData->point[i].z;
                            scanBuffer[k].point[j].type      = scanData->point[i].type;
                            scanBuffer[k].point[j].segment   = k + 1;
                            scanBuffer[k].point[j].intensity = scanData->point[i].intensity;

                            j++;
                        }
                    }

                    scanBuffer[k].data.recordingTime = scanData->recordingTime;
                    scanBuffer[k].data.duration      = scanData->duration;
                    scanBuffer[k].data.maxRange      = scanData->maxRange;
                    scanBuffer[k].data.pointNum      = j;
                    scan2dTimeout[k] = 0;

                    GDOS_DBG_DETAIL("Buffer Scan2D(%i) recordingtime %i "
                                    "pointNum %i x %i y %i\n", scan2dInst[k],
                                    scanData->recordingTime,
                                    scanData->pointNum,
                                    odometryBuffer[k].pos.x,
                                    odometryBuffer[k].pos.y);
                }
            }
        }


        // new odometry data, build new scan2d_data_msg
        if ((dataInfo.src == RackName::create(ODOMETRY, odometryInst)) &&
            (dataInfo.type == MSG_DATA))
        {
            odoData = OdometryData::parse(&dataInfo);

            // get datapointer from rackdatabuffer
            mergeData = (scan2d_data *)getDataBufferWorkSpace();

            mergeData->recordingTime = odoData->recordingTime;
            mergeData->duration      = dataBufferPeriodTime;
            mergeData->maxRange      = scanBuffer[0].data.maxRange;
            mergeData->sectorNum     = 1;
            mergeData->sectorIndex   = 0;
            mergeData->pointNum      = 0;

            for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
            {
                if (scan2dInst[k] >= 0)
                {
                    // scan2d timeout 5s
                    if (scan2dTimeout[k] > 5 * ((float)1000.0f / dataBufferPeriodTime))
                    {
                        GDOS_ERROR("Data timeout Scan2d(%i)\n", scan2dInst[k]);

                        dataMbx.peekEnd();
                        return -ETIMEDOUT;
                    }
                    scan2dTimeout[k]++;

                    sinRho = sin(odoData->pos.rho);
                    cosRho = cos(odoData->pos.rho);

                    for (i = 0; i < scanBuffer[k].data.pointNum; i++)
                    {
                        if (mergeData->pointNum >= SCAN2D_POINT_MAX)
                        {
                            GDOS_ERROR("Merged scan exceeds SCAN2D_POINT_MAX %i\n", SCAN2D_POINT_MAX);

                            for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
                            {
                                if (scan2dInst[k] >= 0)
                                {
                                    GDOS_WARNING("Scan2d(%i) pointNum %i", scan2dInst[k], scanBuffer[k].data.pointNum);
                                }
                            }
                            dataMbx.peekEnd();
                            return -EOVERFLOW;
                        }

                        j = mergeData->pointNum;

                        posDiffX = odometryBuffer[k].pos.x - odoData->pos.x;
                        posDiffY = odometryBuffer[k].pos.y - odoData->pos.y;

                        x = scanBuffer[k].point[i].x + posDiffX;
                        y = scanBuffer[k].point[i].y + posDiffY;

                        mergeData->point[j].x         =   (int)(x * cosRho)
                                                        + (int)(y * sinRho);
                        mergeData->point[j].y         = - (int)(x * sinRho)
                                                        + (int)(y * cosRho);
                        mergeData->point[j].z         =   (int)sqrt(mergeData->point[j].x * mergeData->point[j].x +
                                                                    mergeData->point[j].y * mergeData->point[j].y);
                        mergeData->point[j].type      = scanBuffer[k].point[i].type;
                        mergeData->point[j].segment   = scanBuffer[k].point[i].segment;
                        mergeData->point[j].intensity = scanBuffer[k].point[i].intensity;
                        mergeData->pointNum++;
                    }
                }
            }

            GDOS_DBG_DETAIL("recordingtime %i pointNum %i x %i y %i\n",
                            mergeData->recordingTime, mergeData->pointNum,
                            odometryBuffer[0].pos.x, odometryBuffer[0].pos.y);

            datalength = sizeof(scan2d_data) +
                         sizeof(scan_point) * mergeData->pointNum;

            putDataBufferWorkSpace(datalength);
        }
    }
    else
    {
        GDOS_ERROR("Received unexpected message from %n to %n type %d on "
                   "data mailbox\n", dataInfo.src, dataInfo.dest,
                   dataInfo.type);
        dataMbx.peekEnd();
        return -EINVAL;
    }

    dataMbx.peekEnd();

    return 0;
}

int  Scan2dMerge::moduleCommand(message_info *msgInfo)
{
    // not for me -> ask RackDataModule
    return RackDataModule::moduleCommand(msgInfo);
}

/*******************************************************************************
 *   !!! NON REALTIME CONTEXT !!!
 *
 *   own non realtime user functions,
 *   moduleInit,
 *   moduleCleanup,
 *   Constructor,
 *   Destructor,
 *   main
 *
 ******************************************************************************/
int Scan2dMerge::moduleInit(void)
{
    int     ret, k;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    // get static parameter
    odometryInst  = getInt32Param("odometryInst");
    scan2dInst[0] = getInt32Param("scan2dInstA");
    scan2dInst[1] = getInt32Param("scan2dInstB");
    scan2dInst[2] = getInt32Param("scan2dInstC");
    scan2dInst[3] = getInt32Param("scan2dInstD");

    //
    // create mailboxes
    //

    // work Mbx
    ret = createMbx(&workMbx, 1, 128, MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // data Mbx
    ret = createMbx(&dataMbx, 10, sizeof(scan2d_data_msg),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_DATA);


    //
    // create Proxies
    //

    // odometry
    odometry = new OdometryProxy(&workMbx, 0, odometryInst);
    if (!odometry)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_ODOMETRY);

    // scan2d
    for (k = 0; k < SCAN2D_SENSOR_NUM_MAX; k++)
    {
        if (scan2dInst[k] >= 0)
        {
            scan2d[k] = new Scan2dProxy(&workMbx, 0, scan2dInst[k]);
            if (!scan2d)
            {
                ret = -ENOMEM;
                goto init_error;
            }
            initBits.setBit(INIT_BIT_PROXY_SCAN2D + k);
        }
    }

    return 0;

init_error:

    // !!! call local cleanup function !!!
    Scan2dMerge::moduleCleanup();
    return ret;
}


// non realtime context
void Scan2dMerge::moduleCleanup(void)
{
    int     k;

    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // free scan2d proxies
    for (k = SCAN2D_SENSOR_NUM_MAX - 1; k >= 0; k--)
    {
        if (scan2dInst[k] >= 0)
        {
            if (initBits.testAndClearBit(INIT_BIT_PROXY_SCAN2D + k))
            {
                delete scan2d[k];
            }
        }
    }

    // free odometry proxy
    if (initBits.testAndClearBit(INIT_BIT_PROXY_ODOMETRY))
    {
        delete odometry;
    }

    // delete data mailbox
    if (initBits.testAndClearBit(INIT_BIT_MBX_DATA))
    {
        destroyMbx(&dataMbx);
    }

    // delete work mailbox
    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }
}

Scan2dMerge::Scan2dMerge(void)
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    240,              // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    10,               // max buffer entries
                    10)               // data buffer listener
{
    dataBufferMaxDataSize   =sizeof(scan2d_data_msg);
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "Scan2dMerge");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new Scan2dMerge
    p_inst = new Scan2dMerge();
    if (!p_inst)
    {
        printf("Can't create new Scan2dMerge -> EXIT\n");
        return -ENOMEM;
    }

    // init
    ret = p_inst->moduleInit();
    if (ret)
        goto exit_error;

    p_inst->run();

    return 0;

exit_error:
    delete (p_inst);
    return ret;
}
