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
 *      Matthias Hentschel      <hentschel@rts.uni-hannover.de>
 *
 */

#ifndef __OBJ_RECOG_RELAY_LADAR_H__
#define __OBJ_RECOG_RELAY_LADAR_H__

#include <main/rack_data_module.h>
#include <main/rack_module.h>

#include <perception/obj_recog_proxy.h>
#include <navigation/position_proxy.h>

// define module class
#define MODULE_CLASS_ID     OBJ_RECOG

typedef struct
{
    obj_recog_data      data;
    obj_recog_object    object[OBJ_RECOG_OBJECT_MAX];
} __attribute__((packed)) obj_recog_data_msg;

//######################################################################
//# class ObjRecogRelayLadar
//######################################################################
class ObjRecogRelayLadar : public RackDataModule
{
    private:

    int                 positionInst;
    int                 ladarOffsetX;
    int                 ladarOffsetY;

    RackMailbox         workMbx;
    RackMailbox         dataMbx;
    uint64_t            dataTimeOut;

    PositionProxy       *position;

  protected:

    // -> non realtime context
    int  moduleOn(void);
    void moduleOff(void);
    int  moduleLoop(void);
    int  moduleCommand(message_info *msgInfo);

    // -> non realtime context
    void moduleCleanup(void);

  public:

    // constructor und destructor
    ObjRecogRelayLadar();
    ~ObjRecogRelayLadar() {};

    // -> non realtime context
    int  moduleInit(void);
    bool inited;

};

#endif // __OBJ_RECOG_RELAY_LADAR_H__
