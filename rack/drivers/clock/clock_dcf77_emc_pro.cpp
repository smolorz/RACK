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
#include <iostream>

#include "clock_dcf77_emc_pro.h"

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE                0
#define INIT_BIT_SERIALPORT_OPEN            1
#define INIT_BIT_MBX_WORK                   2
#define INIT_BIT_PROXY_POSITION             3

//
// data structures
//

ClockDcf77EmcPro *p_inst;

argTable_t argTab[] = {

    { ARGOPT_REQ, "serialDev", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Serial device number", { -1 } },

    { ARGOPT_OPT, "periodTime", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Period time of the timing signal (in ms), default 1000", { 1000 } },

    { ARGOPT_OPT, "realtimeClockUpdate", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Enable update of the realtime clock, 0=off, 1=on, default 1", { 1 } },

    { ARGOPT_OPT, "realtimeClockUpdateTime", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Time interval for updating the realtime clock in ms, default 60000 ", { 60000 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};

struct rtser_config clock_serial_config = {
    config_mask       : 0xFFFF,
    baud_rate         : 9600,
    parity            : RTSER_NO_PARITY,
    data_bits         : RTSER_8_BITS,
    stop_bits         : RTSER_1_STOPB,
    handshake         : RTSER_DEF_HAND,
    fifo_depth        : RTSER_DEF_FIFO_DEPTH,
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
 *   moduleOff,
 *   moduleLoop,
 *   moduleCommand,
 *
 *   own realtime user functions
 ******************************************************************************/
int ClockDcf77EmcPro::moduleOn(void)
{
    // get dynamic module parameter
    dataBufferPeriodTime    = getInt32Param("periodTime");
    realtimeClockUpdate     = getInt32Param("realtimeClockUpdate");
    realtimeClockUpdateTime = getInt32Param("realtimeClockUpdateTime");

    serialPort.clean();

    // set rx timeout 2 * periodTime
    serialPort.setRecvTimeout(rackTime.toNano(2 * periodTime));

    // init variables
    lastUpdateTime = rackTime.get() - realtimeClockUpdateTime;

    return RackDataModule::moduleOn(); // has to be last command in moduleOn();
}

// realtime context
void ClockDcf77EmcPro::moduleOff(void)
{
    RackDataModule::moduleOff();       // has to be first command in moduleOff();
}

// realtime context
int ClockDcf77EmcPro::moduleLoop(void)
{
    int                 ret;
    clock_data*         p_data;

    // get datapointer from rackdatabuffer
    p_data = (clock_data *)getDataBufferWorkSpace();

    // read next serial message
    ret = readSerialMessage(&serialData);
    if (ret)
    {
        GDOS_ERROR("Can't read serial message from serial device %i, code %i\n", serialDev, ret);
        return ret;
    }

    // decode message
    ret = analyseSerialMessage(&serialData, p_data);
    if (ret)
    {
        GDOS_ERROR("Can't decode serial message, code %d\n", ret);
    }

    /*RackTask::disableRealtimeMode();
    printf("clock data: %s\n", serialData.data);
    RackTask::enableRealtimeMode();*/

    p_data->recordingTime = serialData.recordingTime - 22;

    // update realtime clock
    if ((realtimeClockUpdate == 1) && (p_data->syncMode == CLOCK_SYNC_MODE_REMOTE))
    {
        // each realtimeClockUpdateTime
        if (((int)p_data->recordingTime - (int)lastUpdateTime) > realtimeClockUpdateTime)
        {
            rackTime.set(p_data->utcTime, p_data->recordingTime);

            GDOS_DBG_INFO("update realtime clock at recordingtime %dms to utc time %ds\n",
                          p_data->recordingTime, p_data->utcTime);

            lastUpdateTime = serialData.recordingTime;
        }
    }

    GDOS_DBG_DETAIL("recordingtime %i, utcTime %d\n", p_data->recordingTime, p_data->utcTime);

    putDataBufferWorkSpace(sizeof(clock_data));

    return 0;
}

int ClockDcf77EmcPro::moduleCommand(message_info *msgInfo)
{
    // not for me -> ask RackDataModule
    return RackDataModule::moduleCommand(msgInfo);
}

int ClockDcf77EmcPro::readSerialMessage(clock_serial_data *serialData)
{
    int             i, ret;
    int             msgSize;
    unsigned char   currChar;
    rack_time_t     recordingTime;

    // init local variables
    recordingTime   = 0;
    currChar        = 0;
    i               = 0;
    msgSize         = sizeof(serialData->data);

    // synchronize to message head, timeout after 200 attempts
    while ((i < 200) && (currChar != '<'))
    {
        // read next character
        ret = serialPort.recv(&currChar, 1, &recordingTime);
        if (ret)
        {
            GDOS_ERROR("Can't read message head from serial device %i, code = %d\n",
                       serialDev, ret);
            return ret;
        }
        i++;
    }


    // check for timeout
    if (i >= 200)
    {
        GDOS_ERROR("Can't synchronize on message head\n");
        return -ETIME;
    }
    // store first character
    else
    {
        serialData->data[0] = currChar;
        i = 1;
    }


    // read serial message until currChar == "Carriage Return",
    // timout if msgSize is reached
    while ((i < msgSize) && (currChar != 0x0D))
    {
        // read next character
        ret = serialPort.recv(&currChar, 1, &recordingTime);
        if (ret)
        {
            GDOS_ERROR("Can't read data from serial device %i, code = %d\n",
                       serialDev, ret);
            return ret;
        }

        // store character
        serialData->data[i] = currChar;
        i++;
    }

    // if last read character != "Carriage Return" an error occured
    if (currChar != 0x0D)
    {
        GDOS_ERROR("Can't read end of serial message\n");
        return -EINVAL;
    }
    // store recordingtime of last character
    else
    {
        serialData->recordingTime = recordingTime;
    }

    return 0;
}

int ClockDcf77EmcPro::analyseSerialMessage(clock_serial_data *serialData, clock_data *data)
{
    char    subStr[8];
    char    *endPtr;
    int     month, year;

    // hour
    strncpy (subStr, &serialData->data[1], 2);
    subStr[2]='\0';
    data->hour = strtol(subStr, &endPtr, 10);
    if (*endPtr != 0)
    {
        GDOS_ERROR("Cannot read hours from serial data");
        return -EIO;
    }

    // minute
    strncpy (subStr, &serialData->data[3], 2);
    subStr[2]='\0';
    data->minute = strtol(subStr, &endPtr, 10);
    if (*endPtr != 0)
    {
        GDOS_ERROR("Cannot read minutes from serial data");
        return -EIO;
    }

    // second
    strncpy (subStr, &serialData->data[21], 2);
    subStr[2]='\0';
    data->second = strtol(subStr, &endPtr, 10);
    if (*endPtr != 0)
    {
        GDOS_ERROR("Cannot read seconds from serial data");
        return -EIO;
    }

    // day
    strncpy (subStr, &serialData->data[5], 2);
    subStr[2]='\0';
    data->day = strtol(subStr, &endPtr, 10);
    if (*endPtr != 0)
    {
        GDOS_ERROR("Cannot read days from serial data");
        return -EIO;
    }

    // month
    strncpy (subStr, &serialData->data[7], 2);
    subStr[2]='\0';
    data->month = strtol(subStr, &endPtr, 10);
    if (*endPtr != 0)
    {
        GDOS_ERROR("Cannot read months from serial data");
        return -EIO;
    }

    // year
    strncpy (subStr, &serialData->data[9], 2);
    subStr[2]='\0';
    data->year = strtol(subStr, &endPtr, 10);
    if (*endPtr != 0)
    {
        GDOS_ERROR("Cannot read years from serial data");
        return -EIO;
    }

    // day of week
    strncpy (subStr, &serialData->data[11], 1);
    subStr[1]='\0';
    data->dayOfWeek = strtol(subStr, &endPtr, 10);
    if (*endPtr != 0)
    {
        GDOS_ERROR("Cannot read day of week from serial data");
        return -EIO;
    }

    // utc time
    month = data->month;
    year  = data->year + 2000;

    if (0 >= (int)(month -= 2))
    {
        month  += 12;              // puts feb last since it has leap day
        year   -= 1;
    }
    data->utcTime = ((((long)(year/4 - year/100 + year/400 + 367*month/12 + data->day) +
                              year*365 - 719499
                                      )*24 + data->hour
                                      )*60 + data->minute
                                      )*60 + data->second;

    // sync mode
    if (serialData->data[12] == 'F')
    {
        data->syncMode = CLOCK_SYNC_MODE_REMOTE;
    }
    else
    {
        data->syncMode = CLOCK_SYNC_MODE_NONE;
    }

    return 0;
}

/*******************************************************************************
 *   !!! NON REALTIME CONTEXT !!!
 *
 *   moduleInit,
 *   moduleCleanup,
 *   Constructor,
 *   Destructor,
 *   main,
 *
 *   own non realtime user functions
 ******************************************************************************/
int ClockDcf77EmcPro::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    ret = serialPort.open(serialDev, &clock_serial_config, this);
    if (ret)
    {
        GDOS_ERROR("Can't open serialDev %i, code=%d\n", serialDev, ret);
        goto init_error;
    }
    GDOS_DBG_INFO("serialDev %d has been opened \n", serialDev);
    initBits.setBit(INIT_BIT_SERIALPORT_OPEN);

    //
    // create mailboxes
    //

    // work mailbox
    ret = createMbx(&workMbx, 1, 128,
                    MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    return 0;

init_error:
    moduleCleanup();
    return ret;
}

void ClockDcf77EmcPro::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // delete work mailbox
    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }

    if (initBits.testAndClearBit(INIT_BIT_SERIALPORT_OPEN))
    {
        serialPort.close();
    }
}

ClockDcf77EmcPro::ClockDcf77EmcPro()
        : RackDataModule( MODULE_CLASS_ID,
                      5000000000llu,    // 5s datatask error sleep time
                      16,               // command mailbox slots
                      48,               // command mailbox data size per slot
                      MBX_IN_KERNELSPACE | MBX_SLOT, // command mailbox flags
                      5,                // max buffer entries
                      10)               // data buffer listener
{
    // get static module parameter
    serialDev             = getIntArg("serialDev", argTab);
    dataBufferMaxDataSize = sizeof(clock_data);
}

int main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "ClockDcf77EmcPro");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new ClockDcf77EmcPro
    p_inst = new ClockDcf77EmcPro();
    if (!p_inst)
    {
        printf("Can't create new ClockDcf77EmcPro -> EXIT\n");
        return -ENOMEM;
    }

    // init
    ret = p_inst->moduleInit();
    if (ret)
        goto exit_error;

    p_inst->run();

    return 0;

exit_error:

    delete (p_inst);

    return ret;
}
