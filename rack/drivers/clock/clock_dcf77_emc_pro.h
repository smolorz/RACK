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
#ifndef __CLOCK_DCF77_EMC_PRO_H__
#define __CLOCK_DCF77_EMC_PRO_H__

#include <main/rack_data_module.h>
#include <main/serial_port.h>
#include <drivers/clock_proxy.h>

// define module class
#define MODULE_CLASS_ID                     CLOCK

typedef struct
{
    rack_time_t        recordingTime;
    char               data[1024];
} clock_serial_data;



/**
 * Clock DCF77 EMC pro
 *
 * @ingroup modules_clock
 */
class ClockDcf77EmcPro : public RackDataModule {
    private:
        // own vars
        int                 serialDev;
        int                 periodTime;
        int                 realtimeClockUpdate;
        int                 realtimeClockUpdateTime;

        clock_serial_data   serialData;
        rack_time_t         lastUpdateTime;

        SerialPort          serialPort;

        // additional mailboxes
        RackMailbox         workMbx;

        int readSerialMessage(clock_serial_data *serialData);
        int analyseSerialMessage(clock_serial_data *serialData, clock_data *data);

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
        ClockDcf77EmcPro();
        ~ClockDcf77EmcPro() {};

        // -> realtime context
        int  moduleInit(void);
};

#endif // __CLOCK_DCF77_EMC_PRO_H__
