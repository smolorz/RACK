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
 *      Daniel Lecking   <lecking@rts.uni-hannover.de>
 *      Oliver Wulf      <wulf@rts.uni-hannover.de>
 *
 */
#include "ladar_sick_cms3000.h"

// init_flags
#define INIT_BIT_DATA_MODULE            0
#define INIT_BIT_RTSERIAL_OPENED        1

#define MKSHORT(a,b) ((unsigned short)(a)|((unsigned short)(b)<<8))

//
// init data structures
//
typedef struct {
    ladar_data    data;
    int32_t       distance[LADAR_DATA_MAX_DISTANCE_NUM];
} __attribute__((packed)) ladar_data_msg;

static unsigned char serialBuffer[4 * 1024];

static unsigned short crcTable[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

LadarSickCms3000 *p_inst;

argTable_t argTab[] = {

    { ARGOPT_REQ, "serialDev", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The number of the local serial device", { -1 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};

/* serial configuration ( written in non realtime context -> Module::Init() )*/
const struct rtser_config ladar_serial_config =
{
    config_mask       : 0xFFFF,
    baud_rate         : 500000,
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

int  LadarSickCms3000::moduleOn(void)
{
    int ret;
 
    ret = serialPort.clean();
    if (ret)
    {
        return ret;
    }
    GDOS_DBG_INFO("Serial buffer cleaned, serial dev %i\n", serialDev);

    return DataModule::moduleOn();  // have to be last command in moduleOn();
}

void LadarSickCms3000::moduleOff(void)
{
  DataModule::moduleOff();          // have to be first command in moduleOff();
}

int  LadarSickCms3000::moduleLoop(void)
{
    ladar_data  *p_data = NULL;
    RACK_TIME time = 0;
    int ret = 0;
    int i;
    int totalCount = 0;
    int size = 0;
    int distance = 0;
    uint32_t datalength = 0;

    // get datapointer from databuffer
    p_data = (ladar_data *)getDataBufferWorkSpace();

    for(i = 0; i < 10; i++)
    {
        ret = serialPort.recv(&serialBuffer[i], 1, &time,
                          2 * rackTime.toNano(getDataBufferPeriodTime(0)));
        if (ret)
        {
            GDOS_ERROR("Can't read from serial dev %i\n", serialDev);
            return ret;
        }
        if(totalCount > 4 * 1024)
        {
            GDOS_ERROR("Can´t synchronise on package head\n");
            return EAGAIN;
        }

        switch(i)
        {
        case 0:
            p_data->recordingTime = time;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            if(serialBuffer[i] != 0x00)
                i = -1;
            break;
        case 6:
        case 7:
            break;
        case 8:
            if(serialBuffer[i] != 0xff)
                i = -1;
            break;
        case 9:
            if(serialBuffer[i] != 0x07)
                i = -1;
            break;
        }
        totalCount++;
    }

    size = MKSHORT(serialBuffer[7], serialBuffer[6]);

    ret = serialPort.recv(&serialBuffer[i], (size-3)*2, &time,
                          2 * rackTime.toNano(getDataBufferPeriodTime(0)));
    if (ret)
    {
        GDOS_ERROR("Can't read data body from serial dev %i\n", serialDev);
        return ret;
    }

    if(crc_check((unsigned char*)&serialBuffer[4], (size-1)*2) != MKSHORT(serialBuffer[size*2+2], serialBuffer[size*2+3]))
    {
        GDOS_ERROR("CRC check wrong\n");
        return -1;
    }

    // parse package body, read laser raw data
    if((size != 392) |
       (MKSHORT(serialBuffer[20], serialBuffer[21]) != 0xbbbb) |
       (MKSHORT(serialBuffer[22], serialBuffer[23]) != 0x1111))
    {
        GDOS_ERROR("Continuous output configuration is not supported (size %i)\n", size);
        return -1;
    }

    p_data->startAngle      = 95.0 * M_PI/180.0;
    p_data->angleResolution = -0.5 * M_PI/180.0;
    p_data->distanceNum     = 2 * 190 + 1;
    p_data->duration        = 300;
    p_data->maxRange        = 20000;
    p_data->recordingTime   = p_data->recordingTime - 300;

    for(i = 0; i < p_data->distanceNum; i++)
    {
        distance = MKSHORT(serialBuffer[i*2+24], serialBuffer[i*2+25]);

        if(distance & (1 << 13))  // is reflector
        {
            distance = distance & 0x1fff;  // use bits 0-12
            distance = -10 * distance;     // convert cm to mm
        }
        else
        {
            distance = distance & 0x1fff;  // use bits 0-12
            distance = 10 * distance;      // convert cm to mm
        }

        p_data->distance[i] = distance;
    }

    datalength = sizeof(ladar_data) + sizeof(int32_t) * p_data->distanceNum; // points

    // write data buffer slot (and send it to all listener)
    if (datalength > 0 && datalength <= getDataBufferMaxDataSize() )
    {
        putDataBufferWorkSpace( datalength );
        return 0;
    }
    return -ENOSPC;
}

int  LadarSickCms3000::moduleCommand(MessageInfo *p_msginfo)
{
  // not for me -> ask DataModule
  return DataModule::moduleCommand(p_msginfo);
}

unsigned short LadarSickCms3000::crc_check(unsigned char* data, int len)
{
    unsigned short crc16 = 0xFFFF;
    int i;
    for(i = 0; i < len; i++)
    {
        crc16 = (crc16 << 8) ^ (crcTable[(crc16 >> 8) ^ (data[i])]);
    }
    return crc16;
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

int  LadarSickCms3000::moduleInit(void)
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
    ret = serialPort.open(serialDev, &ladar_serial_config, this);
    if (ret)
    {
        GDOS_ERROR("Can't open rtser%d, code = %d\n", serialDev, ret);
        goto init_error;
    }
    GDOS_DBG_INFO("rtser%d has been opened \n", serialDev);
    initBits.setBit(INIT_BIT_RTSERIAL_OPENED);    
    
    ret = serialPort.setBaudrate(500000);
    if (ret)
    {
        GDOS_ERROR("Can't set baudrate to 500kBaud, code = %d\n", ret);
        goto init_error;
    }
    return 0;

init_error:
    // !!! call local cleanup function !!!
    LadarSickCms3000::moduleCleanup();
    return ret;
}

void LadarSickCms3000::moduleCleanup(void)
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
                      serialDev, ret);
    }

    // call DataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        DataModule::moduleCleanup();
    }
}

LadarSickCms3000::LadarSickCms3000(void)
      : DataModule( MODULE_CLASS_ID,
                    1000000000llu,    // 1s cmdtask error sleep time
                    1000000000llu,    // 1s datatask error sleep time
                     100000000llu,    // 100ms datatask disable sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    5,                // max buffer entries
                    10)               // data buffer listener


{
    // get values
    serialDev = getIntArg("serialDev", argTab);

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(ladar_data_msg));

    // set databuffer period time
    setDataBufferPeriodTime(30);
};

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = Module::getArgs(argc, argv, argTab, "LadarSickCms3000");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new LadarSick
    p_inst = new LadarSickCms3000();
    if (!p_inst)
    {
        printf("Can't create new LadarSickCms3000 -> EXIT\n");
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
