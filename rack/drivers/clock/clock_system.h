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
 *
 */
#ifndef __CLOCK_SYSTEM_H__
#define __CLOCK_SYSTEM_H__

#include <main/rack_data_module.h>
#include <drivers/clock_proxy.h>
#include <sys/time.h>

// define module class
#define MODULE_CLASS_ID                     CLOCK

//######################################################################
//# class ClockSystem
//######################################################################

class ClockSystem : public RackDataModule {
    private:
        // own vars
        int                 clockSys;
        int                 clockInst;
        int                 systemClockUpdate;
        int                 systemClockUpdateTime;

        clock_data          clockData;
        rack_time_t         lastUpdateTime;
        int                 currSyncMode;

        ClockProxy          *clock;

        // additional mailboxes
        RackMailbox         workMbx;

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
        ClockSystem();
        ~ClockSystem() {};

        // -> realtime context
        int  moduleInit(void);
};

#endif // __CLOCK_SYSTEM_H__
