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
#ifndef __COMPASS_CMPS03_H__
#define __COMPASS_CMPS03_H__

#include <main/rack_data_module.h>
#include <drivers/compass_proxy.h>
#include <main/serial_port.h>

// define module class
#define MODULE_CLASS_ID                     COMPASS

typedef struct
{
    rack_time_t        recordingTime;
    char               data[1024];
} compass_serial_data;


/**
 * System Compass
 *
 * @ingroup modules_compass
 */
class CompassCmps03 : public RackDataModule {
    private:
        // own vars
        int                 serialDev;
        int                 baudrate;
        int                 periodTime;

        SerialPort          serialPort;
        compass_serial_data serialData;

        // additional mailboxes
        RackMailbox         workMbx;

        int readSerialMessage(compass_serial_data *serialData);
        int analyseSerialMessage(compass_serial_data *serialData, compass_data *data);

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
        CompassCmps03();
        ~CompassCmps03() {};

        // -> realtime context
        int  moduleInit(void);
};

#endif // __COMPASS_CMPS03_H__
