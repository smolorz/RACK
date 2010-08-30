/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
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
 *
 */
#ifndef __OBJ_RECOG_IBEO_LUX_H__
#define __OBJ_RECOG_IBEO_LUX_H__

#include <main/rack_data_module.h>
#include <main/can_port.h>
#include <drivers/chassis_proxy.h>
#include <perception/obj_recog_proxy.h>

// define module class
#define MODULE_CLASS_ID                     OBJ_RECOG

typedef struct
{
    obj_recog_data      data;
    obj_recog_object    object[OBJ_RECOG_OBJECT_MAX];
} __attribute__((packed)) obj_recog_data_msg;

/**
 * Object Recognition Ibeo Lux
 *
 * @ingroup modules_obj_recog
 */
class ObjRecogIbeoLux : public RackDataModule {
    private:

        // own vars
        int                 canDev;
        int                 baseCanId;
        int                 chassisSys;
        int                 chassisInst;

        chassis_data        chassisData;
        chassis_param_data  chassisParam;

        // additional mailboxes
        RackMailbox         workMbx;

        // proxies
        ChassisProxy        *chassis;
        CanPort             canPort;

    protected:
        // -> realtime context
        int      moduleOn(void);
        void     moduleOff(void);
        int      moduleLoop(void);
        int      moduleCommand(RackMessage *msgInfo);

        // -> non realtime context
        void     moduleCleanup(void);

    public:
        // constructor und destructor
        ObjRecogIbeoLux();
        ~ObjRecogIbeoLux() {};

        // -> realtime context
        int  moduleInit(void);
};

#endif // __OBJ_RECOG_IBEO_LUX_H__
