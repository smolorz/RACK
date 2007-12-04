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
#include "scan2d_sim.h"

#include <main/argopts.h>

// init_flags
#define INIT_BIT_DATA_MODULE        0
#define INIT_BIT_MBX_WORK           1
#define INIT_BIT_MBX_ODOMETRY       2
#define INIT_BIT_PROXY_ODOMETRY     3

typedef struct {
      scan2d_data     data;
      scan_point      point[SCAN2D_POINT_MAX];
} __attribute__((packed)) scan2d_msg;

//
// data structures
//

Scan2DSim *p_inst;

argTable_t argTab[] = {

    { ARGOPT_OPT, "odometryInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the odometry module", { -1 } },

    { ARGOPT_OPT, "maxRange", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "maximum laser range", { 10000 } },

    { ARGOPT_OPT, "mapOffsetX", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "mapOffsetX for DXF maps in GK coordinates", { 0 } },

    { ARGOPT_OPT, "mapOffsetY", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "mapOffsetY for DXF maps in GK coordinates", { 0 } },

    { ARGOPT_REQ, "mapFile", ARGOPT_REQVAL, ARGOPT_VAL_STR,
      "filename of the DXF map to load", { 0 } },

    { ARGOPT_OPT, "angleRes", ARGOPT_REQVAL, ARGOPT_VAL_INT,
        "angle resolution, default 1 deg", { 1 } },

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

 int  Scan2DSim::moduleOn(void)
{
    int ret;
    rack_time_t realPeriodTime;

    GDOS_DBG_DETAIL("Turning on Odometry(%d) \n", odometryInst);
    ret = odometry->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on Odometry(%d), code = %d \n", odometryInst, ret);
        return ret;
    }
    GDOS_DBG_DETAIL("Odometry(%d) has been turned on \n", odometryInst);

    GDOS_DBG_DETAIL("Request continuous data from Odometry(%d)\n", odometryInst);
    ret = odometry->getContData(0, &odometryMbx, &realPeriodTime);
    if (ret)
    {
        GDOS_ERROR("Can't get continuous data from Odometry(%d), "
                   "code = %d \n", odometryInst, ret);
        return ret;
    }

    setDataBufferPeriodTime(realPeriodTime);

    return RackDataModule::moduleOn();  // has to be last command in moduleOn();
}

void Scan2DSim::moduleOff(void)
{
    RackDataModule::moduleOff();        // has to be first command in moduleOff();

    odometry->stopContData(&odometryMbx);
}

int  Scan2DSim::moduleLoop(void)
{
    scan2d_data*    data2D       = NULL;
    odometry_data*  dataOdometry = NULL;
    message_info    msgInfo;
    ssize_t         datalength;
    double          angle, angleResolution, distance;
    double          cosRho, sinRho, featureDistance, a;
    double          x1, x2, x3, x4, y1, y2, y3, y4, denominator;
    int             i, j, ret;

    // get datapointer from rackdatabuffer
    data2D = (scan2d_data *)getDataBufferWorkSpace();

    // get Odometry data
    ret = odometryMbx.recvDataMsgTimed(1000000000llu, &odometryData,
                                       sizeof(odometryData), &msgInfo);
    if (ret)
    {
        GDOS_ERROR("Can't receive odometry data on DATA_MBX, "
                   "code = %d \n", ret);
        return ret;
    }

    if ((msgInfo.type != MSG_DATA) ||
        (msgInfo.src  != odometry->getDestAdr()))
    {
        GDOS_ERROR("Received unexpected message from %n to %n type %d on "
                   "data mailbox\n", msgInfo.src, msgInfo.dest, msgInfo.type);

        return -EINVAL;
    }

    dataOdometry = OdometryData::parse(&msgInfo);

    data2D->recordingTime = dataOdometry->recordingTime;
    data2D->duration = getDataBufferPeriodTime(0);
    data2D->maxRange = maxRange;
    data2D->sectorNum     = 1;
    data2D->sectorIndex   = 0;
    data2D->pointNum = (360 / angleRes) + 1;

    angle           = -180.0 * M_PI / 180.0;
    angleResolution = (double)angleRes * M_PI / 180.0;

    for (i = 0; i < data2D->pointNum; i ++)
    {
        distance = maxRange;   // initialization with maxRange
        cosRho = cos(dataOdometry->pos.rho + angle);
        sinRho = sin(dataOdometry->pos.rho + angle);

        // calculating the minimum distance to the intersections with all lines
        for (j = 0; j < dxfMap.featureNum; j ++)
        {
            // calculating parameters of a line in the map
            x1 = dxfMap.feature[j].x;
            y1 = dxfMap.feature[j].y;

            x2 = dxfMap.feature[j].l * dxfMap.feature[j].cos;
            y2 = dxfMap.feature[j].l * dxfMap.feature[j].sin;

            // calculating parameters of the laser beam line
            x3 = dataOdometry->pos.x;
            y3 = dataOdometry->pos.y;

            // adding the orientation of the candidate sample
            x4 = cosRho;
            y4 = sinRho;

            // calculating distance = b
            denominator = (x4 * y2) - (y4 * x2);

            if ( ( denominator > 0.0001 ) || ( denominator < -0.0001 ) )
            {
                featureDistance = ((x2 * y3) + (x1 * y2) - (y1 * x2) - (x3 * y2)) / denominator;   // intersection

                if ( (featureDistance < distance) && (featureDistance >= 0) )    // considering the working range of the scanner
                {
                    if ( (x2 > -0.5 ) && (x2 < 0.5) )
                    {
                        a = ( y3 + (featureDistance * y4) - y1 ) / y2;
                    }
                    else
                    {
                        a = ( x3 + (featureDistance * x4) - x1 ) / x2;
                    }

                    // to determine if the intersection is out of the start or end point
                    if ( (a >= 0.0) && (a <= 1.0) )
                    {
                        distance = featureDistance;
                    }
                }
            }
        }

        x1 = distance * cos(angle);
        y1 = distance * sin(angle);

        data2D->point[i].x = (int)x1;
        data2D->point[i].y = (int)y1;
        data2D->point[i].z = (int)distance;
        data2D->point[i].type      = TYPE_UNKNOWN;
        data2D->point[i].segment   = 0;
        data2D->point[i].intensity = 0;

        if(distance >= maxRange)
        {
            data2D->point[i].type |= TYPE_MAX_RANGE;
            data2D->point[i].type |= TYPE_INVALID;
        }
        angle += angleResolution;
    }

    datalength = sizeof(scan2d_data) +
                 sizeof(scan_point) * data2D->pointNum; // points

    GDOS_DBG_DETAIL("RecordingTime %u pointNum %d\n",
                    data2D->recordingTime, data2D->pointNum);

    putDataBufferWorkSpace(datalength);
    return 0;
}

int  Scan2DSim::moduleCommand(message_info *msgInfo)
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

int Scan2DSim::moduleInit(void)
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
    ret = createMbx(&workMbx, 1, 128, MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // odometry-data mailbox
    ret = createMbx(&odometryMbx, 1, sizeof(odometry_data),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_ODOMETRY);

    // create Odometry Proxy
    odometry = new OdometryProxy(&workMbx, 0, odometryInst);
    if (!odometry)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_ODOMETRY);

    ret = dxfMap.load(dxfMapFile, mapOffsetX, mapOffsetY);
    if (ret)
    {
        GDOS_ERROR("Can't load DXF map. (%i)\n", ret);
    }
    GDOS_PRINT("Using DXF map with %i features\n", dxfMap.featureNum);

    return 0;

init_error:
    // !!! call local cleanup function !!!
    Scan2DSim::moduleCleanup();
    return ret;
}

void Scan2DSim::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // free proxies
    if (initBits.testAndClearBit(INIT_BIT_PROXY_ODOMETRY))
    {
        delete odometry;
    }

    // delete mailboxes
    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_ODOMETRY))
    {
        destroyMbx(&odometryMbx);
    }
}

Scan2DSim::Scan2DSim(void)
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    10,               // max buffer entries
                    10)               // data buffer listener
      , dxfMap(200)
{
    //
    // get values
    //
    odometryInst = getIntArg("odometryInst", argTab);
    maxRange     = getIntArg("maxRange", argTab);
    mapOffsetX   = getIntArg("mapOffsetX", argTab);
    mapOffsetY   = getIntArg("mapOffsetY", argTab);
    dxfMapFile   = getStrArg("mapFile", argTab);
    angleRes     = getIntArg("angleRes", argTab);


    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(scan2d_msg));
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "Scan2dSim");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new Scan2DSim
    p_inst = new Scan2DSim();
    if (!p_inst)
    {
        printf("Can't create new Scan2DSim -> EXIT\n");
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
