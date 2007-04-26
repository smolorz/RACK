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
#ifndef __PILOT_JOYSTICK_H__
#define __PILOT_JOYSTICK_H__

#include <main/rack_datamodule.h>

#include <navigation/pilot_proxy.h>
#include <main/pilot_tool.h>

#include <perception/scan2d_proxy.h>
#include <drivers/chassis_proxy.h>
#include <drivers/joystick_proxy.h>

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
//# class PilotJoystick
//######################################################################

class PilotJoystick : public RackDataModule {
      private:

        int32_t             scan2dInst;
        int32_t             chassisInst;
        int32_t             joystickInst;
        int32_t             joystickSys;
        int32_t             mode;

        int                 maxSpeed;
        int                 joystikDataMissing;
        int                 scan2dDataMissing;

        int                 joystickSpeed;
        float               joystickCurve;
        float               joystickOmega;
        int                 joystickForce;

        // mailboxes
        RackMailbox         joystickMbx;    // joystick data in
        RackMailbox         scan2dMbx;      // scan2d data in
        RackMailbox         workMbx;        // communication

        // proxies
        JoystickProxy*      joystick;
        Scan2dProxy*        scan2d;
        ChassisProxy*       chassis;

        // buffer
        joystick_data       jstkData;
        chassis_param_data  chasParData;
        scan2d_data_msg     scan2dMsg;

      protected:
        // -> realtime context
        int      moduleOn(void);
        int      moduleLoop(void);
        void     moduleOff(void);
        int      moduleCommand(message_info *msgInfo);

        // -> non realtime context
        void     moduleCleanup(void);

      public:
        // constructor und destructor
        PilotJoystick();
        ~PilotJoystick() {};

        // -> non realtime context
        int  moduleInit(void);
};

#endif // __PILOT_JOYSTICK_H__
