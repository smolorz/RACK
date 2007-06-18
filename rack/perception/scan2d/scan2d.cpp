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
#include "scan2d.h"

#include <main/argopts.h>

// init_flags
#define INIT_BIT_DATA_MODULE        0
#define INIT_BIT_MBX_WORK           1
#define INIT_BIT_MBX_LADAR          2
#define INIT_BIT_PROXY_LADAR        3

typedef struct {
    ladar_data    data;
    int32_t       buffer[LADAR_DATA_MAX_DISTANCE_NUM];
} __attribute__((packed)) ladar_data_msg;

typedef struct {
    scan2d_data     data;
    scan_point      point[SCAN2D_POINT_MAX];
} __attribute__((packed)) scan2d_msg;

//
// data structures
//

Scan2d *p_inst;

argTable_t argTab[] = {

    { ARGOPT_REQ, "ladarInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the ladar driver", { -1 } },

    { ARGOPT_OPT, "ladarOffsetX", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Ladar X offset (default 0)", { 0 } },

    { ARGOPT_OPT, "ladarOffsetY", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Ladar Y offset (default 0)", { 0 } },

    { ARGOPT_OPT, "ladarOffsetRho", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Ladar rho offset (default 0)", { 0 } },

    { ARGOPT_OPT, "maxRange", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Maximal ladar range", { 30000 } },

    { ARGOPT_OPT, "reduce", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "reduce (default 1)", { 1 } },

    { ARGOPT_OPT, "angleMin", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "minimum angle (default -180)", { -180 } },

    { ARGOPT_OPT, "angleMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "maximum angle (default 180)", { 180 } },

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

 int  Scan2d::moduleOn(void)
{
    int ret;
    rack_time_t realPeriodTime = 0;

    GDOS_DBG_DETAIL("Turning on Ladar(%d)\n", ladarInst);
    ret = ladar->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on Ladar(%d), code = %d \n", ladarInst, ret);
        return ret;
    }

    GDOS_DBG_DETAIL("Request continuous data from Ladar(%d)\n", ladarInst);
    ret = ladar->getContData(0, &ladarMbx, &realPeriodTime);
    if (ret)
    {
        GDOS_ERROR("Can't get continuous data from Ladar(%d), "
                   "code = %d \n", ladarInst, ret);
        return ret;
    }

    setDataBufferPeriodTime(realPeriodTime);

    return RackDataModule::moduleOn();  // have to be last command in moduleOn();
}

void Scan2d::moduleOff(void)
{
    RackDataModule::moduleOff();        // have to be first command in moduleOff();

    ladar->stopContData(&ladarMbx);
}

int  Scan2d::moduleLoop(void)
{
    scan2d_data*    data2D    = NULL;
    ladar_data*     dataLadar = NULL;
    message_info     msgInfo;
    ssize_t         datalength = 0;
    double          angle, x, y;
    int             i, j, ret;

    // get datapointer from rackdatabuffer
    data2D = (scan2d_data *)getDataBufferWorkSpace();

    // get Ladar data
    ret = ladarMbx.peekTimed(1000000000llu, &msgInfo); // 1s
    if (ret)
    {
        GDOS_ERROR("Can't receive ladar data on DATA_MBX, "
                   "code = %d \n", ret);
        return ret;
    }

    if ((msgInfo.type != MSG_DATA) ||
        (msgInfo.src  != ladar->getDestAdr()))
    {
        GDOS_ERROR("Received unexpected message from %n to %n type %d on "
                   "data mailbox\n", msgInfo.src, msgInfo.dest, msgInfo.type);

        ladarMbx.peekEnd();
        return -EINVAL;
    }

    dataLadar = LadarData::parse(&msgInfo);

    if (dataLadar->distanceNum > LADAR_DATA_MAX_DISTANCE_NUM)
    {
        GDOS_ERROR("DistanceNum (%d) too great\n", dataLadar->distanceNum);
        ladarMbx.peekEnd();
        return -EINVAL;
    }

    data2D->recordingTime = dataLadar->recordingTime;
    data2D->duration      = dataLadar->duration;

    if (dataLadar->maxRange < maxRange)
        data2D->maxRange      = dataLadar->maxRange;
    else
        data2D->maxRange      = maxRange;

    data2D->pointNum = 0;
    angle = dataLadar->startAngle;

    for (i=0; i < dataLadar->distanceNum; i += reduce)
    {
        if ((angle >= angleMinFloat) &&
            (angle <= angleMaxFloat))
        {
            j = data2D->pointNum;
            data2D->point[j].type      = TYPE_UNKNOWN;
            data2D->point[j].segment   = 0;
            data2D->point[j].intensity = 0;

            if (dataLadar->distance[i] < 0)
            {
                dataLadar->distance[i]  = -dataLadar->distance[i];
                data2D->point[j].type  |= TYPE_REFLECTOR;
            }

            if ((dataLadar->distance[i] >= data2D->maxRange) || (dataLadar->distance[i] == 0))
            {
                dataLadar->distance[i]  = data2D->maxRange;
                data2D->point[j].type  |= TYPE_MAX_RANGE;
                data2D->point[j].type  |= TYPE_INVALID;
            }

            x = (double)dataLadar->distance[i] *
                        cos(angle + ladarOffsetRhoFloat);
            y = (double)dataLadar->distance[i] *
                        sin(angle + ladarOffsetRhoFloat);

            data2D->point[j].x = (int)x + ladarOffsetX;
            data2D->point[j].y = (int)y + ladarOffsetY;
            data2D->point[j].z = dataLadar->distance[i];

            data2D->pointNum++;
        }
        angle += reduce * dataLadar->angleResolution;
    }

    datalength = sizeof(scan2d_data) +
                 sizeof(scan_point) * data2D->pointNum; // points

//    GDOS_DBG_DETAIL("RecordingTime %u pointNum %d\n",
//                    data2D->recordingTime, data2D->pointNum);

    ladarMbx.peekEnd();
    putDataBufferWorkSpace(datalength);
    return 0;
}

int  Scan2d::moduleCommand(message_info *msgInfo)
{
    // not for me -> ask RackDataModule
    return RackDataModule::moduleCommand(msgInfo);
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

int Scan2d::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    GDOS_DBG_DETAIL("Scan2d::moduleInit ... \n");

    // work mailbox
    ret = createMbx(&workMbx, 1, 128, MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // ladar-data mailbox
    ret = createMbx(&ladarMbx, 1, sizeof(ladar_data_msg),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_LADAR);

    // create Ladar Proxy
    ladar = new LadarProxy(&workMbx, 0, ladarInst);
    if (!ladar)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_LADAR);

    return 0;

init_error:
    // !!! call local cleanup function !!!
    Scan2d::moduleCleanup();
    return ret;
}

void Scan2d::moduleCleanup(void)
{
    GDOS_DBG_DETAIL("Scan2d::moduleCleanup ... \n");

    // free proxies
    if (initBits.testAndClearBit(INIT_BIT_PROXY_LADAR))
    {
        delete ladar;
    }

    // delete mailboxes
    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_LADAR))
    {
        destroyMbx(&ladarMbx);
    }

    // call RackDataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }
}

Scan2d::Scan2d(void)
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s cmdtask error sleep time
                    5000000000llu,    // 5s datatask error sleep time
                     100000000llu,    // 100ms datatask disable sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    10,               // max buffer entries
                    10)               // data buffer listener
{
    //
    // get values
    //
    ladarInst      = getIntArg("ladarInst", argTab);
    ladarOffsetX   = getIntArg("ladarOffsetX", argTab);
    ladarOffsetY   = getIntArg("ladarOffsetY", argTab);
    ladarOffsetRho = getIntArg("ladarOffsetRho", argTab);
    maxRange       = getIntArg("maxRange", argTab);
    reduce         = getIntArg("reduce", argTab);
    angleMin       = getIntArg("angleMin", argTab);
    angleMax       = getIntArg("angleMax", argTab);

    angleMinFloat       = (double)angleMin       * M_PI / 180.0;
    angleMaxFloat       = (double)angleMax       * M_PI / 180.0;
    ladarOffsetRhoFloat = (double)ladarOffsetRho * M_PI / 180.0;

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(scan2d_msg));
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "Scan2d");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new Scan2d
    p_inst = new Scan2d();
    if (!p_inst)
    {
        printf("Can't create new Scan2d -> EXIT\n");
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
