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
 *      Matthias Hentschel   <hentschel@rts.uni-hannover.de>
 *
 */
#ifndef __GYRO_XSENS_H__
#define __GYRO_XSENS_H__

#include <termios.h>
#include <main/rack_data_module.h>
#include <main/angle_tool.h>
#include <main/serial_port.h>
//#include "main/linux/serial_port_linux.cpp"
#include <drivers/gyro_proxy.h>

#define MODULE_CLASS_ID         GYRO


#define GYRO_XSENS_MESSAGE_PREAMBLE 0xFA

#define GYRO_XSENS_MESSAGE_MTDATA   0x32


typedef struct
{
    uint8_t         preamble;
    uint8_t         bid;
    uint8_t         mid;
    uint8_t         len;
    uint16_t        extLen;
    uint8_t         data[2048];
    uint8_t         checksum;
} gyro_xsens_message;


/**
 * Gyro Xsens.
 *
 * @ingroup modules_gyro
 */
class GyroXsens : public RackDataModule
{
    private:
        // module parameter
        int                 serialDev;
        int                 baudrate;

        // global variables
        SerialPort          serialPort;
        gyro_xsens_message  gyroMessage;

        // own functions
        int      readMessage(gyro_xsens_message *message, rack_time_t *recordingTime);
        int      parseMTDataMessage(gyro_xsens_message *message, gyro_data *data);
        uint16_t getDataShort(uint8_t *data, uint16_t offset);
        float    getDataFloat(uint8_t *data, uint16_t offset);

    protected:
        // -> realtime context
        int      moduleOn(void);
        void     moduleOff(void);
        int      moduleLoop(void);
        int      moduleCommand(RackMessage *p_msginfo);

        // -> non realtime context
        void     moduleCleanup(void);

    public:
        // constructor und destructor
        GyroXsens();
        ~GyroXsens() {};

        // -> non realtime context
        int  moduleInit(void);
};

#endif // __GYRO_XSENS_H__
