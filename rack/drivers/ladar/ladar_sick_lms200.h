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
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#ifndef __LADAR_SICK_H__
#define __LADAR_SICK_H__

#include <main/rack_datamodule.h>
#include <main/serial_port.h>

#include <drivers/ladar_proxy.h>

#define MODULE_CLASS_ID         LADAR

// rack configuration
enum PROTOCOL { normal, interlaced, fast };

typedef struct {
    int         serDev;

    char*       cmd_baudrate;
    char*       cmd_sendData;

    int         baudrate;
    enum PROTOCOL protocol;

    int         serial_buffer_size;

    int32_t     periodTime;
    int         messageDistanceNum;
    float       startAngle;
    float       angleResolution;

    int         duration;
    int         maxRange;

    uint64_t    wait_for_ack;
    uint64_t    wait_for_modechange;
    uint64_t    wait_after_modechange;

} ladar_config_t;

// realtime context
#define MKSHORT(a,b) ((unsigned short)(a)|((unsigned short)(b)<<8))


typedef int     (*ladar_exchangeCommand_t)(char* command, int commandLen);

typedef int     (*ladar_loopCheckHead_t)(unsigned char* serialBuffer);

typedef unsigned int (*ladar_createLadarData_t)(unsigned char* serialBuffer,
                                                ladar_data *p_data,
                                                int distanceUnit,
                                                rack_time_t time,
                                                int64_t buffer_period_time_ns);



//
// values for *cmd_baudrate
//
static char ladar_cmd_baudrate_38400[] =
                                     {0x02,0x00,0x02,0x00,0x20,0x40,0x50,0x08};

static char ladar_cmd_baudrate_500K[] =
                                     {0x02,0x00,0x02,0x00,0x20,0x48,0x58,0x08};

// values for *cmd_sendData
static char ladar_cmd_sendData_norm[] =
                                     {0x02,0x00,0x02,0x00,0x20,0x24,0x34,0x08};

static char ladar_cmd_sendData_int[] =
                                     {0x02,0x00,0x02,0x00,0x20,0x2A,0x3A,0x08};

static char ladar_cmd_baudrate_9600[] = {0x02,          // STX
                                         0x00,          // Adresse
                                         0x02,0x00,     // Datenlänge
                                         0x20,          // Betriebmodus wechseln
                                         0x42,          //
                                         0x52,0x08};

static char ladar_cmd_config[] = {0x02,                 // STX
                                  0x00,                 // Adresse
                                  0x0A,0x00,            // Datenlänge
                                  0x20,                 // Betriebmodus wechseln
                                  0x00,                 // Einrichtmodus
                                  0x53,0x49,0x43,0x4B,  // Passwort:
                                  0x5F,0x4C,0x4D,0x53,  //   SICK_LMS
                                  0xBE,0xC5 };          // Checksumme

/*
static char ladar_cmd_hardreset[] = {0x02,0x00,0x01,0x00,0x10,0x34,0x12};
*/

//######################################################################
//# class LadarSick
//######################################################################

class LadarSickLms200 : public DataModule {
    private:

        SerialPort  serialPort;

        unsigned short crc_check(unsigned char* data, int len);

        int connect(void);
        int disconnect(void);
        int resetLadar(void);

        int norm_exchangeCommand(char* command, int commandLen);
        int norm_loopCheckHead(unsigned char* serialBuffer);
        unsigned int norm_createLadarData(unsigned char* serialBuffer,
                                          ladar_data *p_data, int distanceUnit,
                                          rack_time_t time);

        int int_exchangeCommand(char* command, int commandLen);
        int int_loopCheckHead(unsigned char* serialBuffer);
        unsigned int int_createLadarData(unsigned char* serialBuffer,
                                         ladar_data *p_data, int distanceUnit,
                                         rack_time_t time);

        int fast_exchangeCommand(char* command, int commandLen);
        int fast_loopCheckHead(unsigned char* serialBuffer);
        unsigned int fast_createLadarData(unsigned char* serialBuffer,
                                          ladar_data *p_data, int distanceUnit,
                                          rack_time_t time);

        int exchangeCommand(char* command, int commandLen);
        int loopCheckHead(unsigned char* serialBuffer);
        unsigned int createLadarData(unsigned char* serialBuffer,
                                     ladar_data *p_data, int distanceUnit,
                                     rack_time_t time);

    protected:
        // -> realtime context
        int      moduleOn(void);
        void     moduleOff(void);
        int      moduleLoop(void);
        int      moduleCommand(MessageInfo *p_msginfo);

        // -> non realtime context
        void     moduleCleanup(void);

    public:
        // ladar configuration struct
        ladar_config_t*    conf;

        // constructor und destructor
        LadarSickLms200();
        ~LadarSickLms200() {};

        // -> non realtime context
        int  moduleInit(void);
};

#endif
