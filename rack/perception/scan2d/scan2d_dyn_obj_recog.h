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
#ifndef __SCAN_2D_DYN_OBJ_RECOG_H__
#define __SCAN_2D_DYN_OBJ_RECOG_H__

#include <main/rack_data_module.h>
#include <main/defines/point2d.h>
#include <perception/scan2d_proxy.h>
#include <perception/obj_recog_proxy.h>

typedef struct {
    scan2d_data     data;
    scan_point      point[SCAN2D_POINT_MAX];
} __attribute__((packed)) scan2d_data_msg;

typedef struct {
    obj_recog_data     data;
    obj_recog_object   object[OBJ_RECOG_OBJECT_MAX];
} __attribute__((packed)) obj_recog_data_msg;


#define MODULE_CLASS_ID             SCAN2D

/**
 * Scan2d Dyn Obj Recog
 *
 * @ingroup modules_scan2d
 */
class Scan2dDynObjRecog : public RackDataModule {
    private:

        // own vars
        int          scan2dSys;
        int          scan2dInst;
        int          objRecogSys;
        int          objRecogInst;

        int          vMin;

        obj_recog_data_msg  objRecogMsg;

        // additional mailboxes
        RackMailbox workMbx;
        RackMailbox dataMbx;

        // proxies
        Scan2dProxy     *scan2d;
        ObjRecogProxy   *objRecog;

    protected:
        // -> realtime context
        int  moduleOn(void);
        void moduleOff(void);
        int  moduleLoop(void);
        int  moduleCommand(RackMessage *msgInfo);

        // -> non realtime context
        void moduleCleanup(void);

        void classifyDynamic(scan2d_data *scan2dData, obj_recog_data *objRecogData, int vMin);

    public:
        // constructor und destructor
        Scan2dDynObjRecog();
        ~Scan2dDynObjRecog() {};

        // -> non realtime context
        int  moduleInit(void);
};

#endif // __SCAN_2D_DYN_OBJ_RECOG_H__
