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

    { ARGOPT_REQ, "serialDev", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The number of the local serial device", { -1 } },

    { ARGOPT_OPT, "protocol", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Protocol (normal = 0, interlaced(raw) = 1, fast = 2), default normal(0)",
      { 0 } },

    { ARGOPT_OPT, "baudrate", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Working serial baudrate [ 38400 | 500000 ], default 38400", { 38400 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
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

static ladar_sick_lms200_config config_sick_norm =
{
    serDev:                 1,
    cmd_baudrate:           NULL,
    cmd_sendData:           ladar_cmd_sendData_norm,
    baudrate:               9600,
    protocol:               normal,
    serial_buffer_size:     0,
    periodTime:             213, // sampling_rate = 4.7
    messageDistanceNum:     0,
    startAngle:             0.0,
    angleResolution:        0.0,
    duration:               0,
    maxRange:               0,
    wait_for_ack:             60000000ll,   // 60ms
    wait_for_modechange:    1000000000ll,   //  1 s
    wait_after_modechange:    30000000ll    // 30ms
};

static ladar_sick_lms200_config config_sick_interlaced =
{
    serDev:                 -1,
    cmd_baudrate:           NULL,
    cmd_sendData:           ladar_cmd_sendData_int,
    baudrate:               9600,
    protocol:               interlaced,
    serial_buffer_size:     0,
    periodTime:             13,    // sampling_rate = 75.0,
    messageDistanceNum:     0,
    startAngle:             0.0,
    angleResolution:        0.0,
    duration:               0,
    maxRange:               0,
    wait_for_ack:            60000000ll,
    wait_for_modechange:    3000000000ll,
    wait_after_modechange:    30000000ll
};

static ladar_sick_lms200_config config_sick_fast =
{
    serDev:                 -1,
    cmd_baudrate:           NULL,
    cmd_sendData:           ladar_cmd_sendData_norm,
    baudrate:               9600,
    protocol:               fast,
    serial_buffer_size:     0,
    periodTime:             13,    // sampling_rate = 75.0,
    messageDistanceNum:     0,
    startAngle:             0.0,
    angleResolution:        0.0,
    duration:               0,
    maxRange:               0,
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

    serialPort.setRecvTimeout(2 * rackTime.toNano(getDataBufferPeriodTime(0)));

    return RackDataModule::moduleOn();  // has to be last command in moduleOn();
}

void LadarSickLms200::moduleOff(void)
{
  RackDataModule::moduleOff();          // has to be first command in moduleOff();

  disconnect();
}

int  LadarSickLms200::moduleLoop(void)
{
    short         a             = 0;
    short         b             = 0;
    int           ret           = 0;
    int           headLength    = 4;
    int           dataLength    = 0;
    int           crcLength     = 2;
    rack_time_t   timeStamp;
    ladar_data    *p_data       = NULL;
    unsigned char serialBuffer[820];


    // get datapointer from databuffer
    p_data = (ladar_data *)getDataBufferWorkSpace();

    // read head with timestamp
    ret = serialPort.recv(&serialBuffer[0], headLength, &timeStamp);
    if (ret)
    {
        GDOS_ERROR("loop: ERROR: can't read message head from rtser%d (4 bytes), "
                   "code = %d\n", conf->serDev, ret );
        return ret;
    }

    // check for start byte
    if (serialBuffer[0] != 0x02)
    {
        GDOS_ERROR("loop: ERROR: wrong start byte %x, required %x\n", serialBuffer[0], 0x02);
        return -EBADMSG;
    }


    // read data with checksum
    dataLength = MKSHORT(serialBuffer[2], serialBuffer[3]);
    ret = serialPort.recv(&serialBuffer[4], dataLength + crcLength);
    if (ret)
    {
        GDOS_ERROR("loop: ERROR: can't read message data from rtser%d (%d bytes), "
                   "code = %d\n", conf->serDev, dataLength + crcLength, ret );
        return ret;
    }

    // checksum
    a = crcCheck(serialBuffer, headLength + dataLength);
    b = MKSHORT(serialBuffer[headLength + dataLength],
                serialBuffer[headLength + dataLength + 1]);
    if (a != b)
    {
        GDOS_ERROR("CRC check wrong: %x != %x\n", a, b);
        return -EINVAL ;
    }


    // decode data
    switch (serialBuffer[4])
    {
        // distance measurements
        case 0xb0:
            analyseLadarData(serialBuffer, p_data, timeStamp, conf->protocol);
            putDataBufferWorkSpace(sizeof(ladar_data) + sizeof(int) * p_data->distanceNum);
            GDOS_DBG_DETAIL("Data recordingtime %i distanceNum %i\n",
                            p_data->recordingTime, p_data->distanceNum);
            break;

        default:
            GDOS_ERROR("loop: ERROR: command %x not implemented\n", serialBuffer[4]);
            return -EINVAL;
    }
    return 0;
}

int  LadarSickLms200::moduleCommand(message_info *p_msginfo)
{
  // not for me -> ask RackDataModule
  return RackDataModule::moduleCommand(p_msginfo);
}


// returns datalen (0=ERROR)
void LadarSickLms200::analyseLadarData(unsigned char* serialBuffer,
                                       ladar_data *data, rack_time_t timeStamp,
                                       int protocol)
{
    int           n              = 0;
    int           reflector      = 0;
    int           dataFormat     = 0;
    int           distanceNum    = 0;
    int           subScanNum     = 0;
    int           subScanFlag    = 0;
    int           distanceUnit   = 0;


    // get data format
    dataFormat = MKSHORT(serialBuffer[5], serialBuffer[6]);


    // number of distance measurements  (bit 0...9)
    distanceNum = dataFormat & 0x3ff;

    // scan part number (bit 11...12)
    subScanNum = ((dataFormat & 0x1800) >> 11);

    // sub scan flag    (bit 13)
    subScanFlag = ((dataFormat & 0x2000) >> 13);

    // distance unit    (bit 14...15)
    distanceUnit = ((dataFormat & 0xc000) >> 14);

    // analyse data bytes of the scan
    for (n = 0; n < distanceNum; n++)
    {
        data->distance[n] = *(unsigned short*)&serialBuffer[7 + 2*n];

        // cm mode 80m, 13bit distance 3Bit reflector
        if (distanceUnit == 0)
        {
            reflector = data->distance[n] & 0xe000; // 1110 0000 0000 0000
            data->distance[n] = data->distance[n] & 0x1fff;
            data->distance[n] = data->distance[n] * 10;

            if (reflector != 0)
            {
                data->distance[n] = -data->distance[n];
            }
        }

        // mm mode 36m, 15bit distance 1Bit reflector
        else
        {
            reflector = data->distance[n] & 0x8000; // 1000 0000 0000 0000
            data->distance[n] = data->distance[n] & 0x7fff;

            if (reflector != 0)
            {
                data->distance[n] = -data->distance[n];
            }
        }
    }

    data->distanceNum = distanceNum;

    // set max range
    if (distanceUnit == 0)
    {
        data->maxRange = 80000;     // 80m (cm mode)
    }
    else
    {
        data->maxRange = 36000;     // 36m (mm mode)
    }

    // scan duration calculation
    //
    // The turn around time of the scanner is 13.3ms 75Hz
    // a scan with 0.5 resolution takes two turns 26.6ms
    // --__--__--__--  (-- = scan) (__ = pause)
    // 1.  2.  3.  4.
    //       ^ receive data from 1. and 2.
    //
    switch (distanceNum)
    {
        // interlaced, x.25, x.50, x.75 deg, 100 deg open angle
        case 100:
            data->duration          = 1000 * 100 / (75 * 360);      // 1 round, 100 deg
            data->recordingTime     = timeStamp - data->duration / 2;
            data->startAngle        = (50.0 - 0.25 * subScanNum) * M_PI / 180.0;
            data->angleResolution   = -1.0 * M_PI/180.0;
        break;

        // standard/interlaced, x.00 deg, 100 deg open angle
        case 101:
            data->duration          = 1000 * 100 / (75 * 360);      // 1 round, 100 deg
            data->recordingTime     = timeStamp - data->duration / 2;
            data->startAngle        = 50.0 * M_PI / 180.0;
            data->angleResolution   = -1.0 * M_PI/180.0;
        break;

        // interlaced, x.25, x.50, x.75 deg, 180 deg open angle
        case 180:
            data->duration          = 1000 * 180 / (75 * 360);      // 1 round, 180 deg
            data->recordingTime     = timeStamp - data->duration / 2;
            data->startAngle        = (90.0 - 0.25 * subScanNum) * M_PI / 180.0;
            data->angleResolution   = -1.0 * M_PI/180.0;
        break;

        // standard/interlaced/fast
        case 181:
            // fast, x.50 deg, 90 deg open angle
            if (protocol == fast)
            {
                data->duration          = 1000 * 180 / (75 * 360);      // 1 round, 180 deg
                data->recordingTime     = timeStamp - data->duration / 2;
                data->startAngle        = 45.0 * M_PI / 180.0;
                data->angleResolution   = -0.5 * M_PI/180.0;
            }
            // standard/interlaced x.00 deg, 180 deg open angle
            else
            {
                data->duration          = 1000 * 180 / (75 * 360);      // 1 round, 180 deg
                data->recordingTime     = timeStamp - data->duration / 2;
                data->startAngle        = 90.0 * M_PI / 180.0;
                data->angleResolution   = -1.0 * M_PI/180.0;
            }
        break;

        // standard, 0.5 deg resolution, 100 deg open angle
        case 201:
            data->duration          = 1000 * 460 / (75 * 360);      // 2 round, 360 + 100 deg
            data->recordingTime     = timeStamp - data->duration / 2;
            data->startAngle        = 50.0 * M_PI / 180.0;
            data->angleResolution   = -0.5 * M_PI/180.0;
        break;

        // standard, 0.5 deg resolution, 180 deg open angle
        case 361:
            data->duration          = 1000 * 540 / (75 * 360);      // 2 round, 360 + 180 deg
            data->recordingTime     = timeStamp - data->duration / 2;
            data->startAngle        = 90.0 * M_PI / 180.0;
            data->angleResolution   = -0.5 * M_PI/180.0;
        break;

        // standard, 0.25 deg resolution, 100 deg open angle
        case 401:
            data->duration          = 1000 * 1180 / (75 * 360);     // 4 round, 3 * 360 + 100 deg
            data->recordingTime     = timeStamp - data->duration / 2;
            data->startAngle        = 50.0 * M_PI / 180.0;
            data->angleResolution   = -0.25 * M_PI/180.0;
        break;
    }
}


// functions for the normal protocol
int  LadarSickLms200::normExchangeCommand(char* command, int commandLen)
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
        ret = serialPort.setRecvTimeout(conf->wait_for_ack);
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
        ret = serialPort.setRecvTimeout(conf->wait_for_modechange);
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


//
// functions for the interlaced protocol
//
int  LadarSickLms200::intExchangeCommand(char* command, int commandLen)
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
        ret = serialPort.setRecvTimeout(conf->wait_for_ack);
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
        ret = serialPort.setRecvTimeout(conf->wait_for_modechange);
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


//
// functions for the fast protocol
//
int  LadarSickLms200::fastExchangeCommand(char* command, int commandLen)
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
        ret = serialPort.setRecvTimeout(conf->wait_for_ack);
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
        ret = serialPort.setRecvTimeout(conf->wait_for_modechange);
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


//
// wrapper
//
int LadarSickLms200::exchangeCommand(char* command, int commandLen)
{
  switch(conf->protocol) {
    case normal:
      return normExchangeCommand(command, commandLen);
    case interlaced:
      return intExchangeCommand(command, commandLen);
    case fast:
      return fastExchangeCommand(command, commandLen);
    default:
      return -EINVAL;
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

unsigned short LadarSickLms200::crcCheck(unsigned char* data, int len)
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

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    GDOS_DBG_DETAIL("LadarSick::moduleInit ... \n");

    // open rtserial port
    ret = serialPort.open(conf->serDev, &ladar_serial_config, this);
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
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // close rtserial port
    if (initBits.testAndClearBit(INIT_BIT_RTSERIAL_OPENED))
    {
        serialPort.close();
    }
}

LadarSickLms200::LadarSickLms200(void)
      : RackDataModule( MODULE_CLASS_ID,
                    1000000000llu,    // 1s datatask error sleep time
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
             sizeof(int32_t) * LADAR_DATA_MAX_DISTANCE_NUM;
  setDataBufferMaxDataSize(dataSize);

  // set databuffer period time
  setDataBufferPeriodTime(conf->periodTime);
  GDOS_PRINT("periodTime: %d\n", conf->periodTime);
};

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "LadarSickLms200");
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
