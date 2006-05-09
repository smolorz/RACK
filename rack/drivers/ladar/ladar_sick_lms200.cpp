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
#include "ladar_sick_lms200.h"

// init_flags
#define INIT_BIT_DATA_MODULE            0
#define INIT_BIT_RTSERIAL_OPENED        1

//
// init data structures
//

LadarSickLms200 *p_inst;

argTable_t argTab[] = {

  {ARGOPT_REQ, "serialDev", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "The number of the local serial device", (int)-1 },

  {ARGOPT_OPT, "protocol", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "Protocol (normal = 0, interlaced(raw) = 1, fast = 2), default normal(0)",
   (int)0 },

  {ARGOPT_OPT, "baudrate", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "Working serial baudrate [ 38400 | 500000 ], default 38400", (int)38400 },

  {0,"",0,0,""}
};

/* serial configuration ( written in non realtime context -> Module::Init() )*/
const struct rtser_config ladar_serial_config =
{
    config_mask       : 0xFFFF,
    baud_rate         : 9600,
    parity            : RTSER_NO_PARITY,
    data_bits         : RTSER_8_BITS,
    stop_bits         : RTSER_1_STOPB,
    handshake         : RTSER_DEF_HAND,
    fifo_depth        : RTSER_FIFO_DEPTH_4, //RTSER_DEF_FIFO_DEPTH,
    rx_timeout        : RTSER_DEF_TIMEOUT,
    tx_timeout        : RTSER_DEF_TIMEOUT,
    event_timeout     : RTSER_DEF_TIMEOUT,
    timestamp_history : RTSER_RX_TIMESTAMP_HISTORY,
    event_mask        : RTSER_EVENT_RXPEND
};

static ladar_config_t config_sick_norm =
{
    serDev:                 1,
    cmd_baudrate:           NULL,
    cmd_sendData:           ladar_cmd_sendData_norm,
    baudrate:               9600,
    protocol:               normal,
    serial_buffer_size:     725 + 7,
    periodTime:             213, // sampling_rate = 4.7
    messageDistanceNum:     361,
    startAngle:             (90.0 * M_PI/180.0),
    angleResolution:        (-0.5 * M_PI/180.0),
    duration:               20,
    maxRange:               80000,
    wait_for_ack:             60000000ll,   // 60ms
    wait_for_modechange:    1000000000ll,   //  1 s
    wait_after_modechange:    30000000ll    // 30ms
};

static ladar_config_t config_sick_interlaced =
{
    serDev:                 -1,
    cmd_baudrate:           NULL,
    cmd_sendData:           ladar_cmd_sendData_int,
    baudrate:               9600,
    protocol:               interlaced,
    serial_buffer_size:     365 + 7,
    periodTime:             13,    // sampling_rate = 75.0,
    messageDistanceNum:     181,
    startAngle:             0.0,
    angleResolution:        0.0,
    duration:               6,
    maxRange:               80000,
    wait_for_ack:            60000000ll,
    wait_for_modechange:    3000000000ll,
    wait_after_modechange:    30000000ll
};

static ladar_config_t config_sick_fast =
{
    serDev:                 -1,
    cmd_baudrate:           NULL,
    cmd_sendData:           ladar_cmd_sendData_norm,
    baudrate:               9600,
    protocol:               fast,
    serial_buffer_size:     365 + 7,
    periodTime:             13,    // sampling_rate = 75.0,
    messageDistanceNum:     181,
    startAngle:             (45.0 * M_PI/180.0),
    angleResolution:        (-0.5 * M_PI/180.0),
    duration:               6,
    maxRange:               80000,
    wait_for_ack:             60000000ll, // 60ms
    wait_for_modechange:    1000000000ll, // 1s
    wait_after_modechange:    30000000ll  // 30ms
};

/*******************************************************************************
 *   !!! REALTIME CONTEXT !!!
 *
 *   moduleOn,
 *      moduleOff,
 *      moduleLoop,
 *   moduleCommand,
 *
 *   own realtime user functions
 ******************************************************************************/

int  LadarSickLms200::moduleOn(void)
{
    int ret;

    // connecting to scanner
    ret = connect();
    if (ret) {
        return ret;
    }

    return DataModule::moduleOn();  // have to be last command in moduleOn();
}

void LadarSickLms200::moduleOff(void)
{
  DataModule::moduleOff();          // have to be first command in moduleOff();

  disconnect();
}

int  LadarSickLms200::moduleLoop(void)
{
    int         ret           = 0;
    int         dataFormat    = 0;
    int         distanceUnit  = 0;
    RACK_TIME   time          = 0;
    uint32_t    bytes         = 0;
    ladar_data  *p_data       = NULL;

    unsigned char serialBuffer[conf->serial_buffer_size];

    // get datapointer from databuffer
    p_data = (ladar_data *)getDataBufferWorkSpace();

    // init values
    p_data->startAngle      = conf->startAngle;
    p_data->angleResolution = conf->angleResolution;
    p_data->distanceNum     = conf->messageDistanceNum;
    p_data->duration        = conf->duration;
    p_data->maxRange        = conf->maxRange;

    // read head with timestamp
    ret = serialPort.recv(serialBuffer, 7, &time,
                          2 * rack_time_to_ns(getDataBufferPeriodTime(0)));
    if (ret)
    {
        GDOS_ERROR("loop: ERROR: can't read data head from rtser%d (7 bytes), "
                   "code = %d\n", conf->serDev, ret );
        return ret;
    }

    // check head
    ret = loopCheckHead(serialBuffer);
    if (ret)
    {
        GDOS_ERROR("loop: ERROR: command head wrong\n");
        return ret;
    }

    // get data format and set distance unit
    dataFormat = MKSHORT(serialBuffer[5], serialBuffer[6]);
    if ((dataFormat & 0xc000) == 0x0000)        // distance unit cm
    {
        distanceUnit = 10;
    }
    else if ((dataFormat & 0xc000) == 0x8000)   // distance unit 10cm
    {
        distanceUnit = 100;
    }
    else // distance unit mm
    {
        distanceUnit = 1;
    }

    bytes = createLadarData(serialBuffer, p_data, distanceUnit, time);

    if (bytes > 0 && bytes <= getDataBufferMaxDataSize() )
    {
        putDataBufferWorkSpace(bytes);
        return 0;
    }

    GDOS_ERROR("Can't create ladar data -> too many bytes\n");
    return -ENOSPC;
}

int  LadarSickLms200::moduleCommand(MessageInfo *p_msginfo)
{
  // not for me -> ask DataModule
  return DataModule::moduleCommand(p_msginfo);
}

// functions for the normal protocol

int  LadarSickLms200::norm_exchangeCommand(char* command, int commandLen)
{
    unsigned char buffer = 0;
    int ret              = 0;
    int i                = 0;
    int byteCount        = 0;
    int totalByteCount   = 0;

    for (i=0; i<3; i++)
    {
        GDOS_DBG_INFO("Send command 0x%x 0x%x 0x%x 0x%x to rtser%d (try %d)\n",
                      command[4], command[5], command[6], command[7],
                      conf->serDev, i);


        // send command
        ret = serialPort.send(command, commandLen);
        if (ret)
        {
            GDOS_ERROR("Can't send command 0x%x 0x%x to rtser%d\n",
                       command[4], command[5], conf->serDev );
            return -ECOMM;
        }

        // set ack receive timeout
        ret = serialPort.setRxTimeout(conf->wait_for_ack);
        if (ret)
        {
            GDOS_ERROR("Can't set read timeout for ack, code = %d \n", ret);
            return ret;
        }

        // waiting for ACK
        while (1) {
            ret = serialPort.recv(&buffer, 1);
            if (ret)
            {
                if (ret == -ETIMEDOUT)
                {
                    GDOS_WARNING("Can't read ACK from rtser%d - TIMEOUT \n",
                                 conf->serDev);
                    break;
                }
                else
                {
                    switch(-ret)
                    {
                        case 32:
                            GDOS_ERROR("Can't read ACK from rtser%d, "
                                       "ERROR = Broken Pipe\n", conf->serDev);
                            break;

                        default:
                            GDOS_ERROR("Can't read ACK from rtser%d, "
                                       "ERROR = %d \n", conf->serDev, ret);
                            break;
                    }
                    return ret;
                }
            }

            totalByteCount++;
            if (totalByteCount > 1000)
            {
                GDOS_WARNING("Received 1000 Byte without ACK-byte\n");
                break;
            }

            // ACK found
            if (buffer == 0x06)
            {
                break;
            }

        }

        // set modechange receive timeout
        ret = serialPort.setRxTimeout(conf->wait_for_modechange);
        if (ret)
        {
            GDOS_ERROR("Can't set read timeout for mode change \n");
            return ret;
        }

        byteCount      = 0;
        totalByteCount = 0;

        // wait for MODE CHANGED package
        while (1)
        {
            ret = serialPort.recv(&buffer, 1);
            if (ret)
            {
                if (ret == -ETIMEDOUT)
                {
                    GDOS_WARNING("Can't read MODE CHANGED from rtser%d - "
                                    "ETIMEDOUT)\n", conf->serDev);
                    break;
                }
                else
                {
                    GDOS_ERROR("Can't read MODE CHANGED from rtser%d - "
                                "ERROR)\n", conf->serDev);
                    return ret;
                }
            }

            totalByteCount++;
            if (totalByteCount > 1000)
            {
                GDOS_WARNING("Received 1000 Byte without "
                             "'mode changed package'\n");
                break;
            }

            byteCount++;
            switch (byteCount)
            {
                case 1:
                    if (buffer != 0x02)
                        byteCount = 0;
                    break;
                case 2:
                    if (buffer != 0x80)
                        byteCount = 0;
                    break;
                case 3:
                    if (buffer != 0x03)
                        byteCount = 0;
                    break;
                case 4:
                    if (buffer != 0x00)
                        byteCount = 0;
                    break;
                case 5:
                    if (buffer != 0xa0)
                        byteCount = 0;
                    break;
                case 6:
                    if (buffer != 0x00)
                        byteCount = 0;
                    break;
                case 7:
                    if(buffer != 0x10)
                        byteCount = 0;
                    break;
                case 8:
                    if(buffer != 0x16)
                        byteCount = 0;
                    break;
                case 9:
                    if (buffer == 0x0a)
                    {
                        RackTask::sleep(conf->wait_after_modechange);
                        return 0;
                    }
                    break;
            }
        } // while-loop
    } // try 1-3

    GDOS_WARNING("No reply on command 0x%x 0x%x to rtser%d\n",
                 command[4], command[5], conf->serDev);

    return -ETIMEDOUT;
}

int  LadarSickLms200::norm_loopCheckHead(unsigned char* serialBuffer)
{
    if ((serialBuffer[0] != 0x02) ||
        (serialBuffer[4] != 0xb0))
    {
        GDOS_ERROR("loop: ERROR: command head wrong\n");
        return -EBADMSG;
    }
    return 0;
}

// returns datalen (0=ERROR)
unsigned int  LadarSickLms200::norm_createLadarData(
                                              unsigned char* serialBuffer,
                                              ladar_data *p_data,
                                              int distanceUnit, RACK_TIME time)
{
    int n         = 0;
    int reflector = 0;
    int ret       = 0;
    short a       = 0;
    short b       = 0;

    // The turn around time of the scanner is 13.3ms 75Hz
    // a scan with 0.5 resolution takes two turns 26.6ms
    // --__--__--__--  (-- = scan) (__ = pause)
    // 1.  2.  3.  4.
    //       ^ receive data from 1. and 2.
    p_data->recordingTime = time - 13 * 3/4;

    // get data
    ret = serialPort.recv(serialBuffer + 7, conf->serial_buffer_size - 7,
                          NULL, rack_time_to_ns(getDataBufferPeriodTime(0)));
    if (ret)
    {
        GDOS_ERROR("Can't read data body from rtser%d (%d bytes), code = %d\n",
                   conf->serDev, conf->serial_buffer_size - 7, ret );
        return 0;
    }

    // data checksum
    a = crc_check(serialBuffer, conf->serial_buffer_size - 2);
    b = MKSHORT(serialBuffer[conf->serial_buffer_size - 2],
                serialBuffer[conf->serial_buffer_size - 1]);
    if (a != b)
    {
        GDOS_ERROR("CRC check wrong: 0x%02x != 0x%02x\n", a, b);
        return 0 ;
    }

    p_data->distanceNum = conf->messageDistanceNum;

    for (n = 0; n < conf->messageDistanceNum; n++) // Datenbytes des Scans lesen
    {
        // Datenbytes in interner Struktur speichern
        p_data->distance[n] = *(unsigned short*)&serialBuffer[2*n + 7];
        if (distanceUnit == 10) // cm mode 80m, 13bit distance 3Bit reflector
        {
            reflector = p_data->distance[n] & 0xe000; // 1110 0000 0000 0000
            p_data->distance[n] = p_data->distance[n] & 0x1fff;
            p_data->distance[n] = p_data->distance[n] * 10;
            if (reflector != 0)
            {
                p_data->distance[n] = -p_data->distance[n];
            }
        }
        else // mm mode 36m, 15bit distance 1Bit reflector
        {
            reflector = p_data->distance[n] & 0x8000; // 1000 0000 0000 0000
            p_data->distance[n] = p_data->distance[n] & 0x7fff;
            if (reflector != 0)
            {
                p_data->distance[n] = -p_data->distance[n];
            }
        }
    }
    return (sizeof(ladar_data) +  p_data->distanceNum * sizeof(int));
}


//
// functions for the interlaced protocol
//

int  LadarSickLms200::int_exchangeCommand(char* command, int commandLen)
{
    unsigned char buffer = 0;
    int ret              = 0;
    int i                = 0;
    int byteCount        = 0;
    int totalByteCount   = 0;

  for (i=0; i<3; i++)
    {
        GDOS_DBG_INFO("Send command 0x%x 0x%x to rtser%d (try %d)\n",
                      command[4], command[5], conf->serDev, i);

        // send command
        ret = serialPort.send(command, commandLen);
        if (ret)
        {
            GDOS_ERROR("Can't send command 0x%x 0x%x to rtser%d\n",
                       command[4], command[5], conf->serDev );
            return -ECOMM;
        }

        // set ack receive timeout
        ret = serialPort.setRxTimeout(conf->wait_for_ack);
        if (ret)
        {
            GDOS_ERROR("Can't set read timeout for ack, code = %d \n", ret);
            return ret;
        }

        // waiting for ACK
        while (1)
        {
            ret = serialPort.recv(&buffer, 1);
            if (ret)
            {
                if (ret == -ETIMEDOUT)
                {
                    GDOS_WARNING("Can't read ACK from rtser%d - "
                                 "TIMEOUT \n", conf->serDev);
                    break;
                }
                else
                {
                    GDOS_ERROR("Can't read ACK from rtser%d - "
                               "ERROR \n", conf->serDev);
                    return ret;
                }
            }

            totalByteCount++;
            if (totalByteCount > 1000)
            {
                GDOS_WARNING("Received 1000 Byte without ACK-byte\n");
                break;
            }

            // ACK found
            if (buffer == 0x06)
            {
                break;
            }
        }

        // set modechange receive timeout
        ret = serialPort.setRxTimeout(conf->wait_for_modechange);
        if (ret)
        {
            GDOS_ERROR("Can't set read timeout for modechange, code = %d\n",
                       ret);
            return ret;
        }

        byteCount      = 0;
        totalByteCount = 0;

        // wait for MODE CHANGED package
        while (1)
        {
            ret = serialPort.recv(&buffer, 1);
            if (ret)
            {
                if (ret == -ETIMEDOUT)
                {
                    GDOS_WARNING("Can't read MODE CHANGED from rtser%d - "
                                 "ETIMEDOUT)\n", conf->serDev);
                    break;
                }
                else
                {
                    GDOS_ERROR("Can't read MODE CHANGED from rtser%d - "
                               "ERROR)\n", conf->serDev);
                    return ret;
                }
            }

            totalByteCount++;
            if (totalByteCount > 1000)
            {
                GDOS_WARNING("Received 1000 Byte without "
                             "'mode changed package'\n");
                break;
            }

            byteCount++;
            switch (byteCount)
            {
                case 1:
                    if (buffer != 0x02)
                        byteCount = 0;
                    break;
                case 2:
                    if (buffer != 0x80)
                        byteCount = 0;
                    break;
                case 3:
                    if (buffer != 0x03)
                        byteCount = 0;
                    break;
                case 4:
                    if (buffer != 0x00)
                        byteCount = 0;
                    break;
                case 5:
                    if (buffer != 0xa0)
                        byteCount = 0;
                    break;
                case 6:
                    if (buffer != 0x00)
                        byteCount = 0;
                    break;
                case 7:
                    if( (buffer != 0x10) && (buffer != 0x30) )
                        byteCount = 0;
                    break;
                case 8:
                    if( (buffer != 0x16) && (buffer != 0x36) )
                        byteCount = 0;
                    break;
                case 9:
                    if (buffer == 0x0a)
                    {
                        RackTask::sleep(conf->wait_after_modechange);
                        return 0;
                    }
                    break;
            }
        } // while-loop
    } // try 1-3

    GDOS_WARNING("No reply on command 0x%x 0x%x to rtser%d\n",
                 command[4], command[5], conf->serDev);

    return -ETIMEDOUT;
}

int  LadarSickLms200::int_loopCheckHead(unsigned char* serialBuffer)
{
    if ((serialBuffer[0] != 0x02) ||
        (serialBuffer[4] != 0xb0))
    {
        return -EBADMSG;
    }
    return 0;
}

// returns datalen (0=ERROR)
unsigned int LadarSickLms200::int_createLadarData(
                                            unsigned char* serialBuffer,
                                            ladar_data *p_data,
                                            int distanceUnit, RACK_TIME time)
{
    int n         = 0;
    int reflector = 0;
    int ret       = 0;
    short a       = 0;
    short b       = 0;
    int len       = 0;

    // The turn around time of the scanner is 13.3ms 75Hz
    // --__--__--__--  (-- = scan) (__ = pause)
    // 1.  2.  3.  4.
    //   ^ receive data from 1.
    p_data->recordingTime = time - 13 * 1/4;

    len = MKSHORT(serialBuffer[2], serialBuffer[3]);
    switch(len)
    {
        case 364:    // 180 Messwerte von Teilscan X.50

            // get data
            ret = serialPort.recv(serialBuffer + 7, 363, NULL,
                            2 * rack_time_to_ns(getDataBufferPeriodTime(0)));
            if (ret)
            {
                GDOS_ERROR("Can't read data body from rtser%d (%d bytes), "
                           "code = %d\n", conf->serDev, 363, ret );
                return 0;
            }

            // data checksum
            a = crc_check(serialBuffer, 363 + 7 - 2);
            b = MKSHORT(serialBuffer[363 + 7 - 2], serialBuffer[363 + 7 - 1]);
            if (a != b)
            {
                GDOS_ERROR("CRC check wrong: %x != %x\n", a, b);
                return 0 ;
            }

            p_data->startAngle      = 89.5 * M_PI/180.0;
            p_data->angleResolution = -1.0 * M_PI/180.0;
            p_data->distanceNum     = 180;

            for (n=0; n<180; n++) { // Datenbytes des Scans lesen
                // Datenbytes in interner Struktur speichern
                p_data->distance[n] = *(unsigned short*)&serialBuffer[2*n + 7];

                if (distanceUnit == 10)
                {
                    // cm mode 80m, 13bit distance 3Bit reflector
                    reflector = p_data->distance[n] & 0xe000;
                    p_data->distance[n] = p_data->distance[n] & 0x1fff;
                    p_data->distance[n] = p_data->distance[n] * 10;

                    if (reflector != 0)
                    {
                        p_data->distance[n] = -p_data->distance[n];
                    }
                }
                else
                {
                    // mm mode 36m, 15bit distance 1Bit reflector
                    reflector = p_data->distance[n] & 0x8000;
                    p_data->distance[n] = p_data->distance[n] & 0x7fff;
                    if (reflector != 0)
                    {
                        p_data->distance[n] = -p_data->distance[n];
                    }
                }
            }
            return (sizeof(ladar_data) +  p_data->distanceNum * sizeof(int));

        case 366:    // 181 Messwerte von Teilscan X.00

            // get data
            ret = serialPort.recv(serialBuffer + 7, 365, NULL,
                               2 * rack_time_to_ns(getDataBufferPeriodTime(0)));
            if (ret)
            {
                GDOS_ERROR("Can't read data body from rtser%d (%d bytes), "
                           "code = %d\n", conf->serDev, 365, ret);
                return 0;
            }

            // data checksum
            a = crc_check(serialBuffer, 365 + 7 - 2);
            b = MKSHORT(serialBuffer[365 + 7 - 2], serialBuffer[365 + 7 - 1]);
            if (a != b)
            {
                GDOS_ERROR("CRC check wrong: 0x%02x != 0x%02x\n", a, b);
                return 0 ;
            }

            p_data->startAngle      = 90.0 * M_PI/180.0;
            p_data->angleResolution = -1.0 * M_PI/180.0;
            p_data->distanceNum     = 181;

            for (n=0; n<181; n++) // Datenbytes des Scans lesen
            {
                // Datenbytes in interner Struktur speichern
                p_data->distance[n] = *(unsigned short*)&serialBuffer[2*n + 7];

                if (distanceUnit == 10)
                {
                    // cm mode 80m, 13bit distance 3Bit reflector
                    reflector = p_data->distance[n] & 0xe000; // 1110 0000 0000 0000
                    p_data->distance[n] = p_data->distance[n] & 0x1fff;
                    p_data->distance[n] = p_data->distance[n] * 10;
                    if (reflector != 0)
                    {
                        p_data->distance[n] = -p_data->distance[n];
                    }
                }
                else
                {
                    // mm mode 36m, 15bit distance 1Bit reflector
                    // 1000 0000 0000 0000
                    reflector = p_data->distance[n] & 0x8000;
                    p_data->distance[n] = p_data->distance[n] & 0x7fff;
                    if (reflector != 0)
                    {
                        p_data->distance[n] = -p_data->distance[n];
                    }
                }
            }
            return (sizeof(ladar_data) +  p_data->distanceNum * sizeof(int));

        default:
            GDOS_ERROR("loop: ERROR: Data body len wrong (%d bytes)\n", len);
            return 0;
    }
}


//
// functions for the fast protocol
//

int  LadarSickLms200::fast_exchangeCommand(char* command, int commandLen)
{
    unsigned char buffer = 0;
    int ret              = 0;
    int i                = 0;
    int byteCount        = 0;
    int totalByteCount   = 0;

    for (i=0; i<3; i++)
    {
        GDOS_DBG_INFO("Send command 0x%x 0x%x to rtser%d (try %d)\n",
                       command[4], command[5], conf->serDev, i);

        // send command
        ret = serialPort.send(command, commandLen);
        if (ret)
        {
            GDOS_ERROR("Can't send command 0x%x 0x%x to rtser%d\n",
                       command[4], command[5], conf->serDev );
            return -ECOMM;
        }

        // set ack receive timeout
        ret = serialPort.setRxTimeout(conf->wait_for_ack);
        if (ret)
        {
            GDOS_ERROR("Can't set read timeout for ack, code = %d \n", ret);
            return ret;
        }

        // waiting for ACK
        while (1)
        {
            ret = serialPort.recv(&buffer, 1);
            if (ret)
            {
                if (ret == -ETIMEDOUT)
                {
                    GDOS_WARNING("Can't read ACK from rtser%d - "
                                 "TIMEOUT \n", conf->serDev);
                    break;
                }
                else
                {
                    GDOS_ERROR("Can't read ACK from rtser%d - ERROR \n",
                               conf->serDev);
                    return ret;
                }
            }

            totalByteCount++;
            if (totalByteCount > 1000)
            {
                GDOS_WARNING("Received 1000 Byte without ACK-byte\n");
                break;
            }

            // ACK found
            if (buffer == 0x06)
            {
                break;
            }

        }

        // set modechange receive timeout
        ret = serialPort.setRxTimeout(conf->wait_for_modechange);
        if (ret)
        {
            GDOS_ERROR("Can't set read timeout for modechange, code = %d \n",
                       ret);
            return ret;
        }


        byteCount      = 0;
        totalByteCount = 0;

        // wait for MODE CHANGED package
        while (1)
        {
            ret = serialPort.recv(&buffer, 1);
            if (ret)
            {
                if (ret == -ETIMEDOUT)
                {
                    GDOS_WARNING("Can't read MODE CHANGED from rtser%d - "
                                 "ETIMEDOUT)\n", conf->serDev);
                    break;
                }
                else
                {
                    GDOS_ERROR("Can't read MODE CHANGED from rtser%d - "
                               "ERROR)\n", conf->serDev);
                    return ret;
                }
            }

            totalByteCount++;
            if (totalByteCount > 1000)
            {
                GDOS_WARNING("Received 1000 Byte without "
                             "'mode changed package'\n");
                break;
            }

            byteCount++;
            switch (byteCount)
            {
                case 1:
                    if (buffer != 0x02)
                        byteCount = 0;
                    break;
                case 2:
                    if (buffer != 0x80)
                        byteCount = 0;
                    break;
                case 3:
                    if (buffer != 0x03)
                        byteCount = 0;
                    break;
                case 4:
                    if (buffer != 0x00)
                        byteCount = 0;
                    break;
                case 5:
                    if (buffer != 0xa0)
                        byteCount = 0;
                    break;
                case 6:
                    if (buffer != 0x00)
                        byteCount = 0;
                    break;
                case 7:
                    if(buffer != 0x18)  // Ladar Status OK, Sonderger�
                        byteCount = 0;
                    break;
                case 8:
                    if(buffer != 0x1e)
                        byteCount = 0;
                    break;
                case 9:
                    if (buffer == 0x0a)
                    {
                        RackTask::sleep(conf->wait_after_modechange);
                        return 0;
                    }
                    break;
            }
        } // while-loop
    } // try 1-3

    GDOS_WARNING("No reply on command 0x%x 0x%x to rtser%d\n",
                 command[4], command[5], conf->serDev);

    return -ETIMEDOUT;
}

int  LadarSickLms200::fast_loopCheckHead(unsigned char* serialBuffer)
{
    if ((serialBuffer[0] != 0x02) ||
        (MKSHORT(serialBuffer[2], serialBuffer[3]) != 366) ||
        (serialBuffer[4] != 0xb0))
    {
        return -EBADMSG;
    }
    return 0;
}

unsigned int LadarSickLms200::fast_createLadarData(
                                             unsigned char* serialBuffer,
                                             ladar_data *p_data,
                                             int distanceUnit, RACK_TIME time)
{
    int n         = 0;
    int ret       = 0;
    short a       = 0;
    short b       = 0;

    // The turn around time of the scanner is 13.3ms 75Hz
    // --__--__--__--  (-- = scan) (__ = pause)
    // 1.  2.  3.  4.
    p_data->recordingTime = time - 13 * 1/4;

    // get data
    ret = serialPort.recv(serialBuffer + 7, conf->serial_buffer_size - 7,
                            NULL, rack_time_to_ns(getDataBufferPeriodTime(0)));
    if (ret)
    {
        GDOS_ERROR("loop: ERROR: can't read data body from rtser%d (%d bytes), "
                   "code = %d\n",
                    conf->serDev, conf->serial_buffer_size - 7, ret );
        return 0;
    }

    // checksum
    a = crc_check(serialBuffer, conf->serial_buffer_size - 2);
    b = MKSHORT(serialBuffer[conf->serial_buffer_size - 2],
                serialBuffer[conf->serial_buffer_size - 1]);
    if (a != b)
    {
        GDOS_ERROR("loop: ERROR: CRC check wrong: %x != %x\n", a, b);
        return 0 ;
    }

    p_data->distanceNum = conf->messageDistanceNum;
    for (n=0; n < conf->messageDistanceNum; n++) // Datenbytes des Scans lesen
    {
        p_data->distance[n] = *(unsigned short*)&serialBuffer[2*n + 7] *
                              distanceUnit;
    }

    return (sizeof(ladar_data) +  p_data->distanceNum * sizeof(int));
}

//
// wrapper
//
int LadarSickLms200::exchangeCommand(char* command, int commandLen)
{
  switch(conf->protocol) {
    case normal:
      return norm_exchangeCommand(command, commandLen);
    case interlaced:
      return int_exchangeCommand(command, commandLen);
    case fast:
      return fast_exchangeCommand(command, commandLen);
    default:
      return -EINVAL;
  }
}

int LadarSickLms200::loopCheckHead(unsigned char* serialBuffer)
{
  switch(conf->protocol) {
    case normal:
      return norm_loopCheckHead(serialBuffer);
    case interlaced:
      return int_loopCheckHead(serialBuffer);
    case fast:
      return fast_loopCheckHead(serialBuffer);
    default:
      return -EINVAL;
  }
}

unsigned int LadarSickLms200::createLadarData(unsigned char* serialBuffer,
                                        ladar_data *p_data,
                                        int distanceUnit, RACK_TIME time)
{
  switch(conf->protocol) {
    case normal:
      return norm_createLadarData(serialBuffer, p_data, distanceUnit, time);
    case interlaced:
      return int_createLadarData(serialBuffer, p_data, distanceUnit, time);
    case fast:
      return fast_createLadarData(serialBuffer, p_data, distanceUnit, time);
    default:
      return 0;
  }
}

int LadarSickLms200::connect(void)
{
    int ret;

    ret = serialPort.clean();
    if (ret)
    {
        return ret;
    }

    ret = exchangeCommand(ladar_cmd_config, 16);
    if (!ret)
    {
        GDOS_DBG_INFO("Change baudrate to %d (work rate)\n", conf->baudrate);
        ret = exchangeCommand(conf->cmd_baudrate, 8);
        if (ret)
        {
            GDOS_ERROR("Can't change to baudrate %d (work rate)\n",
                       conf->baudrate);
            return ret;
        }

        GDOS_DBG_INFO("Change rtser%d baudrate to %d (work rate)\n",
                      conf->serDev, conf->baudrate);
        ret = serialPort.setBaudrate(conf->baudrate);
        if (ret)
        {
            return ret;
        }

        RackTask::sleep(30000000); // 30 ms

    } else {
        GDOS_DBG_INFO("Try on work baudrate %d\n", conf->baudrate);

        GDOS_DBG_INFO("Change rtser%d baudrate to %d (work rate)\n",
                      conf->serDev, conf->baudrate);

        ret = serialPort.setBaudrate(conf->baudrate);
        if (ret)
        {
            return ret;
        }

        RackTask::sleep(30000000); // 30 ms

        ret = serialPort.clean();
        if (ret)
        {
            return ret;
        }

        ret = exchangeCommand(ladar_cmd_config, 16);
        if (ret)
        {
            GDOS_ERROR("Can't set ladar into configuration mode, code = %d\n",
                       ret);
            return ret;
        }
    }

    GDOS_DBG_INFO("Change into data mode\n");
    ret = exchangeCommand(conf->cmd_sendData, 8);
    if (ret)
    {
        GDOS_ERROR("Can't set ladar int continuous data mode, code = %d\n", ret);
        return ret;
    }
    return 0;
}

int LadarSickLms200::disconnect(void)
{
    // set ladar into configuration mode
    exchangeCommand(ladar_cmd_config, 16);

    // change baudrate to 9600 (config rate)
    exchangeCommand(ladar_cmd_baudrate_9600, 8);

    serialPort.setBaudrate(9600);

    return 0;
}

unsigned short LadarSickLms200::crc_check(unsigned char* data, int len)
{
    unsigned short CRC16_GEN_POL = 0x8005;    // CRC-Polynom: x^16+x^15+x^2+1
                                              // (mit x^16 im CARRY-Flag)
    unsigned short        uCrc16 = 0;         // CRC-Checksumme
    unsigned char      abData[2] = {0,0};     // Datenspeicher
    int i;

    for (i=0; i<len; i++)
    {
        abData[1] = abData[0];
        abData[0] = data[i];
        if (uCrc16 & 0x8000)
        {
            uCrc16 = (uCrc16 & 0x7fff)<<1;
            uCrc16 ^= CRC16_GEN_POL;
        }
        else
        {
            uCrc16 <<=1;
        }
        uCrc16 ^= MKSHORT(abData[0],abData[1]);
    }
    return uCrc16;
}

/*******************************************************************************
 *   !!! NON REALTIME CONTEXT !!!
 *
 *   moduleInit,
 *      moduleCleanup,
 *      Constructor,
 *   Destructor,
 *   main,
 *
 *   own non realtime user functions
 ******************************************************************************/

int  LadarSickLms200::moduleInit(void)
{
    int ret;

    // call DataModule init function (first command in init)
    ret = DataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    GDOS_DBG_DETAIL("LadarSick::moduleInit ... \n");

    // open rtserial port
    ret = serialPort.open(conf->serDev, &ladar_serial_config);
    if (ret)
    {
        GDOS_ERROR("Can't open rtser%d, code = %d\n", conf->serDev, ret);
        goto init_error;
    }
    GDOS_DBG_INFO("rtser%d has been opened \n", conf->serDev);
    initBits.setBit(INIT_BIT_RTSERIAL_OPENED);

    return 0;

init_error:
    // !!! call local cleanup function !!!
    LadarSickLms200::moduleCleanup();
    return ret;
}

void LadarSickLms200::moduleCleanup(void)
{
    int ret;

    GDOS_DBG_DETAIL("LadarSick::moduleCleanup ... \n");

    // close rtserial port
    if (initBits.testAndClearBit(INIT_BIT_RTSERIAL_OPENED))
    {
        ret = serialPort.close();
        if (!ret)
            GDOS_DBG_DETAIL("Serial port closed \n");
        else
            GDOS_ERROR("Can't close serial port %d, code = %d \n",
                       conf->serDev, ret);
    }

    // call DataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        DataModule::moduleCleanup();
    }
}

LadarSickLms200::LadarSickLms200(void)
      : DataModule( MODULE_CLASS_ID,
                    1000000000llu,    // 1s cmdtask error sleep time
                    1000000000llu,    // 1s datatask error sleep time
                     100000000llu,    // 100ms datatask disable sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    3,                // max buffer entries
                    10)               // data buffer listener


{
  size_t dataSize;

  conf = NULL;

  // get values
  int serialDev = getIntArg("serialDev", argTab);
  int protocol  = getIntArg("protocol", argTab);
  int baudrate  = getIntArg("baudrate", argTab);

  // init Ladar stuff

  if (baudrate == 38400 && protocol == interlaced) {
     GDOS_DBG_DETAIL("Useless combination: 38400 baud and interlaced protocol "
                     "-> use 500K baud\n");
     return;
  }

  // load ladar_driver_config_t, depending on protocol
  switch(protocol) {
    case 0:
      // protocol = normal
      conf = &config_sick_norm;
      conf->protocol = normal;
      break;
    case 1:
      // protocol = interlaced (raw)
      conf = &config_sick_interlaced;
      conf->protocol = interlaced;
      break;
    case 2:
      // protocol = fast
      conf = &config_sick_fast;
      conf->protocol = fast;
      break;

    default:
      return;
  }

  switch(baudrate) {
    case 38400:
      conf->cmd_baudrate = ladar_cmd_baudrate_38400;
      conf->baudrate = 38400;
      break;
    case 500000:
      conf->cmd_baudrate = ladar_cmd_baudrate_500K;
      conf->baudrate = 500000;
      break;

    default:
      return;
  }

  //
  // serial driver
  //
  conf->serDev = serialDev;


  // set dataBuffer size
  dataSize = sizeof(ladar_data) +
             sizeof(int32_t) * conf->messageDistanceNum;
  setDataBufferMaxDataSize(dataSize);

  // set databuffer period time
  setDataBufferPeriodTime(conf->periodTime);
  GDOS_PRINT("periodTime: %d\n", conf->periodTime);
};

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = Module::getArgs(argc, argv, argTab, "LadarSickLms200");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new LadarSick
    p_inst = new LadarSickLms200();
    if (!p_inst || !p_inst->conf)
    {
        printf("Can't create new LadarSickLms200 -> EXIT\n");
        return -ENOMEM;
    }

    ret = p_inst->moduleInit();
    if (ret)
        goto exit_error;

    p_inst->run();
    return 0;

exit_error:

    delete (p_inst);
    return ret;
}
