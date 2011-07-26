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

#ifndef __LADAR_SIM_H__
#define __LADAR_SIM_H__

#include <main/rack_data_module.h>
#include <main/dxf_map.h>
#include <drivers/ladar_proxy.h>
#include <navigation/odometry_proxy.h>

#define MODULE_CLASS_ID             LADAR

typedef struct
{
    ladar_data          data;
    ladar_point         point[LADAR_DATA_MAX_POINT_NUM];
} __attribute__((packed)) ladar_data_msg;

//######################################################################
//# class LadarSim
//######################################################################

class LadarSim : public RackDataModule {
    private:
        int          odometrySys;
        int          odometryInst;
        int          maxRange;
        int          mapOffsetX;
        int          mapOffsetY;
        DxfMap       dxfMap;
        char         *dxfMapFile;
        int          angleRes;
        int          mapScaleFactor;
        float        angleMin;
        float        angleMax;

        // additional mailboxes
        RackMailbox workMbx;
        RackMailbox odometryMbx;

        // proxies
        OdometryProxy  *odometry;

        // buffer
        odometry_data  odometryData;

    protected:
        // -> realtime context
        int  moduleOn(void);
        void moduleOff(void);
        int  moduleLoop(void);
        int  moduleCommand(RackMessage *msgInfo);

        // -> non realtime context
        void moduleCleanup(void);

      public:
        // constructor und destructor
        LadarSim();
        ~LadarSim() {};

        // -> non realtime context
        int  moduleInit(void);
};

#endif // __LADAR_SIM_H__

