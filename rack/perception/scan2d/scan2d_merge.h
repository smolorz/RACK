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
#ifndef __SCAN2D_MERGE_H__
#define __SCAN2D_MERGE_H__

#include <main/rack_data_module.h>
#include <drivers/odometry_proxy.h>
#include <perception/scan2d_proxy.h>

#define MODULE_CLASS_ID             SCAN2D

#define SCAN2D_SENSOR_NUM_MAX       4
#define SCAN2D_SECTOR_NUM_MAX       8

// scan_2d data message (use max message size)
typedef struct {
    scan2d_data     data;
    scan_point      point[SCAN2D_POINT_MAX];
} __attribute__((packed)) scan2d_data_msg;

//######################################################################
//# class Scan2dMerge
//######################################################################

class Scan2dMerge : public RackDataModule {
    private:
        int32_t             odometryInst;
        int32_t             scan2dInst[SCAN2D_SENSOR_NUM_MAX];
        int                 scan2dTimeout[SCAN2D_SENSOR_NUM_MAX];
        int                 scan2dSectorNum[SCAN2D_SECTOR_NUM_MAX];

        odometry_data       odometryBuffer[SCAN2D_SENSOR_NUM_MAX][SCAN2D_SECTOR_NUM_MAX];
        scan2d_data_msg     scanBuffer[SCAN2D_SENSOR_NUM_MAX][SCAN2D_SECTOR_NUM_MAX];

        // additional mailboxes
        RackMailbox         workMbx;
        RackMailbox         dataMbx;

        // proxies
        OdometryProxy       *odometry;
        Scan2dProxy         *scan2d[SCAN2D_SENSOR_NUM_MAX];

    protected:
        // -> realtime context
        int  moduleOn(void);
        void moduleOff(void);
        int  moduleLoop(void);
        int  moduleCommand(message_info *msgInfo);

        // -> non realtime context
        void moduleCleanup(void);

    public:
        // constructor und destructor
        Scan2dMerge();
        ~Scan2dMerge() {};

        // -> non realtime context
        int moduleInit(void);
};

#endif // __SCAN2D_MERGE_H__
