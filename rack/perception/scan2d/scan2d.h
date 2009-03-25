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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
#ifndef __SCAN_2D_H__
#define __SCAN_2D_H__

#include <main/rack_data_module.h>
#include <perception/scan2d_proxy.h>
#include <drivers/ladar_proxy.h>
#include <drivers/camera_proxy.h>

typedef struct {
    camera_data     data;
    uint8_t         byteStream[LADAR_DATA_MAX_POINT_NUM * ((CAMERA_MAX_DEPTH+7)/8)];
} __attribute__((packed)) camera_data_ladar_msg;

#define MODULE_CLASS_ID             SCAN2D

//######################################################################
//# class Scan2d
//######################################################################

class Scan2d : public RackDataModule {
    private:

        // own vars
        int          ladarInst;
        int          cameraInst;

        int          ladarOffsetX;
        int          ladarOffsetY;
        int          ladarOffsetRho;
        int          ladarOffsetRhoDivider;
        int          ladarUpsideDown;
        int          maxRange;
        int          reduce;
        int          angleMin;
        int          angleMax;
        unsigned int dataSrcMbxAdr;

        float        angleMinFloat;
        float        angleMaxFloat;
        float        ladarOffsetRhoFloat;

        int          medianFilter;
        int          reflectorFilterMode;

        // additional mailboxes
        RackMailbox workMbx;
        RackMailbox ladarMbx;

        camera_data_ladar_msg cameraMsg;

        // proxies
        LadarProxy  *ladar;
        CameraProxy *camera;

        void turnUpsideDown(ladar_data* dataLadar);
        void filterMedian(ladar_data* dataLadar);

        int  filterReflector(scan2d_data* data, int filter);
        int  getNeighborhood(scan2d_data* data, int refIdx, int *left, int *right);
        int  getRegressLine(scan2d_data* data, int left, int right, double *m, double *n);

        int  addScanIntensity(scan2d_data* data2D);

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
        Scan2d();
        ~Scan2d() {};

        // -> non realtime context
        int  moduleInit(void);
};

#endif // __SCAN_2D_H__
