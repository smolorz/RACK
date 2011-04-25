/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 Leibniz Universität Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Marco Langerwisch <langerwisch@rts.uni-hannover.de>
 *      Oliver Wulf       <wulf@rts.uni-hannover.de>
 *
 */

#include "ladar_sim.h"
#include <main/argopts.h>

// init_flags
#define INIT_BIT_DATA_MODULE        0
#define INIT_BIT_MBX_WORK           1
#define INIT_BIT_MBX_ODOMETRY       2
#define INIT_BIT_PROXY_ODOMETRY     3

//
// data structures
//

LadarSim *p_inst;

arg_table_t argTab[] = {

    { ARGOPT_OPT, "odometrySys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the odometry module", { 0 } },

    { ARGOPT_OPT, "odometryInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the odometry module", { 0 } },

    { ARGOPT_OPT, "maxRange", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "maximum laser range", { 10000 } },

    { ARGOPT_OPT, "angleMin", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "minimum angle (default -180)", { -180 } },

    { ARGOPT_OPT, "angleMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "maximum angle (default 180)", { 180 } },

    { ARGOPT_OPT, "angleRes", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "angle resolution, default 1 deg", { 1 } },

    { ARGOPT_OPT, "mapFile", ARGOPT_REQVAL, ARGOPT_VAL_STR,
      "filename of the DXF map to load", { 0 } },

    { ARGOPT_OPT, "mapScaleFactor", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "map scale factor", { 1000 } },

    { ARGOPT_OPT, "mapOffsetX", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "mapOffsetX for DXF maps in GK coordinates", { 0 } },

    { ARGOPT_OPT, "mapOffsetY", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "mapOffsetY for DXF maps in GK coordinates", { 0 } },

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

 int  LadarSim::moduleOn(void)
{
    int ret;

    // get dynamic module parameter
    maxRange       = getInt32Param("maxRange");
    angleMin       = (double)getInt32Param("angleMin") * M_PI / 180.;
    angleMax       = (double)getInt32Param("angleMax") * M_PI / 180.;
    angleRes       = getInt32Param("angleRes");
    dxfMapFile     = getStringParam("mapFile");
    mapScaleFactor = getInt32Param("mapScaleFactor");
    mapOffsetX     = getInt32Param("mapOffsetX");
    mapOffsetY     = getInt32Param("mapOffsetY");

    RackTask::disableRealtimeMode();
    ret = dxfMap.load(dxfMapFile, mapOffsetX, mapOffsetY, mapScaleFactor);
    RackTask::enableRealtimeMode();
    if (ret)
    {
        GDOS_WARNING("Can't load DXF map. (%i)\n", ret);
        dxfMap.featureNum = 0;
    }
    GDOS_PRINT("Using DXF map with %i features\n", dxfMap.featureNum);

    GDOS_DBG_DETAIL("Turning on Odometry(%d/%d) \n", odometrySys, odometryInst);
    ret = odometry->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on Odometry(%d/%d), code = %d \n", odometrySys, odometryInst, ret);
        return ret;
    }
    GDOS_DBG_DETAIL("Odometry(%d/%d) has been turned on \n", odometrySys, odometryInst);

    GDOS_DBG_DETAIL("Request continuous data from Odometry(%d/%d)\n", odometrySys, odometryInst);
    ret = odometry->getContData(0, &odometryMbx, &dataBufferPeriodTime);
    if (ret)
    {
        GDOS_ERROR("Can't get continuous data from Odometry(%d/%d), "
                   "code = %d \n", odometrySys, odometryInst, ret);
        return ret;
    }

    return RackDataModule::moduleOn();  // has to be last command in moduleOn();
}

void LadarSim::moduleOff(void)
{
    RackDataModule::moduleOff();        // has to be first command in moduleOff();

    odometry->stopContData(&odometryMbx);
}

int  LadarSim::moduleLoop(void)
{
    ladar_data*     dataLadar    = NULL;
    odometry_data*  dataOdometry = NULL;
    RackMessage     msgInfo;
    double          angle, angleResolution, distance;
    double          cosRho, sinRho, featureDistance, a;
    double          x1, x2, x3, x4, y1, y2, y3, y4, denominator;
    int             i, j, ret;

    // get datapointer from rackdatabuffer
    dataLadar = (ladar_data *)getDataBufferWorkSpace();

    // get Odometry data
    ret = odometryMbx.recvDataMsgTimed(rackTime.toNano(2 * dataBufferPeriodTime), &odometryData,
                                       sizeof(odometryData), &msgInfo);
    if (ret)
    {
        GDOS_ERROR("Can't receive odometry data on DATA_MBX, "
                   "code = %d \n", ret);
        return ret;
    }

    if ((msgInfo.getType() != MSG_DATA) ||
        (msgInfo.getSrc()  != odometry->getDestAdr()))
    {
        GDOS_ERROR("Received unexpected message from %n to %n type %d on "
                   "data mailbox\n", msgInfo.getSrc(), msgInfo.getDest(), msgInfo.getType());

        return -EINVAL;
    }

    dataOdometry = OdometryData::parse(&msgInfo);

    dataLadar->recordingTime = dataOdometry->recordingTime;
    dataLadar->duration      = dataBufferPeriodTime;
    dataLadar->maxRange      = maxRange;
    dataLadar->startAngle    = angleMin;
    dataLadar->endAngle      = angleMax;
    dataLadar->pointNum      = ((angleMax - angleMin) * 180. / M_PI / angleRes) + 1;

    angle           = dataLadar->startAngle;
    angleResolution = (double)angleRes * M_PI / 180.0;

    for (i = 0; i < dataLadar->pointNum; i ++)
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

        dataLadar->point[i].angle     = (float)angle;
        dataLadar->point[i].distance  = (int)distance;
        dataLadar->point[i].type      = LADAR_POINT_TYPE_UNKNOWN;
        dataLadar->point[i].intensity = 0;

        if(distance >= maxRange)
        {
            dataLadar->point[i].type |= LADAR_POINT_TYPE_INVALID;
        }
        angle += angleResolution;
    }

    GDOS_DBG_DETAIL("RecordingTime %u pointNum %d\n",
                    dataLadar->recordingTime, dataLadar->pointNum);

    putDataBufferWorkSpace(sizeof(ladar_data) + (dataLadar->pointNum * sizeof(ladar_point)));

    return 0;
}

int  LadarSim::moduleCommand(RackMessage *msgInfo)
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

int LadarSim::moduleInit(void)
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
    odometry = new OdometryProxy(&workMbx, odometrySys, odometryInst);
    if (!odometry)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_ODOMETRY);

    return 0;

init_error:
    // !!! call local cleanup function !!!
    LadarSim::moduleCleanup();
    return ret;
}

void LadarSim::moduleCleanup(void)
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

LadarSim::LadarSim(void)
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    240,              // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    10,               // max buffer entries
                    10)               // data buffer listener
      , dxfMap(300)
{
    // get static module parameter
    odometrySys  = getIntArg("odometrySys", argTab);
    odometryInst = getIntArg("odometryInst", argTab);

    dataBufferMaxDataSize   = sizeof(ladar_data_msg);
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "LadarSim");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new LadarSim
    p_inst = new LadarSim();
    if (!p_inst)
    {
        printf("Can't create new LadarSim -> EXIT\n");
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

