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
#ifndef __POSITION_H__
#define __POSITION_H__

#include <main/rack_datamodule.h>
#include <navigation/position_proxy.h>
#include <drivers/odometry_proxy.h>

// define module class
#define MODULE_CLASS_ID                 POSITION

//######################################################################
//# class Position
//######################################################################

class Position : public DataModule {
    private:
        position_3d         refPos;
        double              sinRefPos, cosRefPos;
        position_3d         refOdo;
        double              sinRefOdo, cosRefOdo;
        rack_time_t           refTime;
        RackMutex           refPosMtx;

        uint32_t            odometryInst;

        // mailboxes
        RackMailbox         odometryMbx;
        RackMailbox         workMbx;

        // proxies
        OdometryProxy*      odometry;

    protected:
        // -> realtime context
        int     moduleOn(void);
        int     moduleLoop(void);
        void    moduleOff(void);
        int     moduleCommand(MessageInfo *msgInfo);

        // -> non realtime context
        void    moduleCleanup(void);

    public:
        // constructor und destructor
        Position();
        ~Position() {};

        // -> non realtime context
        int  moduleInit(void);
};

#endif // __POSITION_H__
