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
 *      Marko Reimer <reimer@rts.uni-hannover.de>
 *
 */
#ifndef __SCAN2D_LAB_H__
#define __SCAN2D_LAB_H__

#include <main/rack_data_module.h>
#include <perception/scan2d_proxy.h>

#define MODULE_CLASS_ID             SCAN2D

// scan_2d data message (use max message size)
typedef struct {
    scan2d_data     data;
    scan_point      point[SCAN2D_POINT_MAX];
} __attribute__((packed)) scan2d_data_msg;

//######################################################################
//# class Scan2dLab
//######################################################################

class Scan2dLab : public RackDataModule {
    private:
        //input values

        // additional mailboxes
        RackMailbox         workMbx;

        // proxies

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
        Scan2dLab();
        ~Scan2dLab() {};

        // -> non realtime context
        int moduleInit(void);
};

#endif // __SCAN2D_LAB_H__
