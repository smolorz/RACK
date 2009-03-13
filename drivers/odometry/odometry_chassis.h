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
 *      Oliver Wulf      <wulf@rts.uni-hannover.de>
 *
 */
#ifndef __ODOMETRY_CHASSIS_H__
#define __ODOMETRY_CHASSIS_H__

#include <main/rack_data_module.h>
#include <drivers/odometry_proxy.h>

// define module class
#define MODULE_CLASS_ID                 ODOMETRY

//######################################################################
//# class OdometryChassis
//######################################################################

class OdometryChassis : public RackDataModule {
    private:
        float               oldPositionX;
        float               oldPositionY;
        float               oldPositionRho;

        int                 chassisInst;

        // mailboxes
        RackMailbox         chassisMbx;
        RackMailbox         workMbx;

        // proxies
        ChassisProxy*       chassis;

        // buffer
        chassis_data        chassisData;

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
        OdometryChassis();
        ~OdometryChassis() {};

        // -> non realtime context
        int  moduleInit(void);
};

#endif // __ODOMETRY_CHASSIS_H__
