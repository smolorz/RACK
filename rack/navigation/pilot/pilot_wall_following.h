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
 */

#ifndef __PILOT_WALL_FOLLOWING_H__
#define __PILOT_WALL_FOLLOWING_H__

#include <main/rack_data_module.h>

#include <navigation/pilot_proxy.h>
#include <main/pilot_tool.h>

#include <perception/scan2d_proxy.h>
#include <navigation/position_proxy.h>
#include <drivers/chassis_proxy.h>
#include <drivers/joystick_proxy.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>


// define module class
#define MODULE_CLASS_ID             PILOT


// scan_2d data message (use max message size)
typedef struct {
    scan2d_data   data;
    scan_point    point[SCAN2D_POINT_MAX];
} __attribute__((packed)) scan2d_data_msg;

// pilot data message (use no splines)
typedef struct {
     pilot_data        data;
     polar_spline      spline[0];
} __attribute__((packed)) pilot_data_msg;


//######################################################################
//# class PilotWallFollowing
//######################################################################

class PilotWallFollowing : public RackDataModule {
      private:

        int                 scan2dInst;
        int                 chassisInst;
        int                 positionInst;
        int                 mode;
        int                 maxSpeed;
        int                 omegaMax;
        int                 distance;
        int                 testDis;

        int                 globalState;
        int                 preState;
        int                 subState;
        int                 preMode;
        int                 rightRot;
        int                 leftRot;
        float               omegaMaxRad;
        float               angle;
        float               angleStart;
        float               omega;

        int                 radius;
        int                 globalSpeed;


        // mailboxes
        RackMailbox         scan2dDataMbx;          // scan2d data
        RackMailbox         workMbx;                // communication

        // proxies
        Scan2dProxy*        scan2d;
        PositionProxy*      position;
        ChassisProxy*       chassis;

        // buffer
        chassis_param_data  chasParData;
        chassis_param_data  chasParDataTransForward;
        chassis_param_data  chasParDataTransBackward;

        scan2d_data_msg     scan2dMsg;

      protected:
        // -> realtime context
        int      moduleOn(void);
        int      moduleLoop(void);
        void     moduleOff(void);
        int      moduleCommand(message_info *msgInfo);
        int      testRec(int x, int y, int xSize, int ySize, scan2d_data *scan);
        int      safeRot(float omega, scan2d_data *scan, chassis_param_data *param);
        int      controlSpeed(int oldSpeed);
        double   funAngle(double x);
        double   funDistance(int x);
        double   funDis(int x);
        float    funRadius(int x);
        float    radiusTest(int splineRadius, float length, scan2d_data *scan, chassis_param_data *param);
        int      modeRandomAngle(int* state);
        int      modeFreeSpace(int* state);
        int      modeWallFollowing(int* state);
        int      modeWallFollowingBraitenberger(int* state);
        int      modeDynamicWindowApproach(int* state);

        // -> non realtime context
        void     moduleCleanup(void);

      public:
        // constructor und destructor
        PilotWallFollowing();
        ~PilotWallFollowing() {};

        // -> non realtime context
        int  moduleInit(void);
};

#endif // __PILOT_WALL_FOLLOWING_H__
