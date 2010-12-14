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

#include "compass_cmps03.h"

#define CALIBRATE_STATE_IDLE        0
#define CALIBRATE_STATE_1_PREALIGN  1
#define CALIBRATE_STATE_1_ALIGN     2
#define CALIBRATE_STATE_1           3
#define CALIBRATE_STATE_1_OK        4

//
// data structures
//

arg_table_t argTab[] = {

   { ARGOPT_REQ, "serialDev", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Serial device number", { -1 } },

    { ARGOPT_OPT, "baudrate", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Baudrate of serial device, default 57600", { 57600 } },

    { ARGOPT_OPT, "periodTime", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "PeriodTime of the Compass module (in ms), default 200", { 200 } },

    { ARGOPT_OPT, "calibration", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Starts the calibration of the Compass module, default 0", { 0 } },

    { ARGOPT_OPT, "compassAligned", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Compass module is aligned to one ordinal direction, default 0", { 0 } },

    { ARGOPT_OPT, "compassOffset", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Sets the compass offset in degree, default 0.0", { 0 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};

struct rtser_config serial_config = {
    config_mask       : 0xFFFF,
    baud_rate         : 0,
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
int CompassCmps03::moduleOn(void)
{
    // get dynamic module parameter
    dataBufferPeriodTime    = getInt32Param("periodTime");
    compassOffset           = getInt32Param("compassOffset");

    serialPort.clean();

    // set rx timeout 2 * periodTime
    serialPort.setRecvTimeout(rackTime.toNano(2 * dataBufferPeriodTime));

    // init
    calibrationFlag    = 0;
    compassAlignedFlag = 0;
    calibrationOK      = 0;
    state              = 0;
    ordinalCount       = 1;

    return RackDataModule::moduleOn(); // has to be last command in moduleOn();
}

// realtime context
void CompassCmps03::moduleOff(void)
{
    RackDataModule::moduleOff();       // has to be first command in moduleOff();
}

// realtime context
int CompassCmps03::moduleLoop(void)
{
    int                 ret;
    compass_data*       p_data;

    // get datapointer from rackdatabuffer
    p_data = (compass_data *)getDataBufferWorkSpace();

    calibrateCompass();

    // read next serial message
    ret = readSerialMessage(&serialData);
    if (ret)
    {
        GDOS_ERROR("Can't read serial message from serial device %i, code %i\n", serialDev, ret);
        //return ret;
    }

    // decode message
    ret = analyseSerialMessage(&serialData, p_data);
    if (ret)
    {
        GDOS_ERROR("Can't decode serial message, code %d\n", ret);
    }

    p_data->recordingTime = serialData.recordingTime;

    if (state != 0)
    {
        p_data->orientation    = 0.0f;
        p_data->varOrientation = 0.0f;
    }

    GDOS_DBG_DETAIL("recordingtime %i, orientation %a, varOrientation %a\n", p_data->recordingTime,
    p_data->orientation, p_data->varOrientation);
    putDataBufferWorkSpace(sizeof(compass_data));
    return 0;
}

int CompassCmps03::moduleCommand(RackMessage *msgInfo)
{
    int ret;

    switch (msgInfo->getType())
    {
        case MSG_SET_PARAM:
            ret = RackDataModule::moduleCommand(msgInfo);

            GDOS_DBG_INFO("Module parameter changed\n");
            calibrationFlag    = getInt32Param("calibration");
            compassAlignedFlag = getInt32Param("compassAligned");
            return ret;
            break;

        // not for me -> ask RackDataModule
        default:
            return RackDataModule::moduleCommand(msgInfo);
    }
}

int CompassCmps03::readSerialMessage(compass_serial_data *serialData)
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
    while ((i < 200) && (currChar != 36))
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


    // read serial message until currChar == "0x03",
    // timout if msgSize is reached
    while ((i < msgSize) && (currChar != 10))
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

    // if last read character != "0x0a" an error occured
    if (currChar != 10)
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

int CompassCmps03::analyseSerialMessage(compass_serial_data *serialData, compass_data *data)
{
    char    subStr[8];
    char    *endPtr;
    float   degree;

    calibrationOK = 0;

    // degree
    strncpy (subStr, &serialData->data[1], 4);
    subStr[4]='\0';

    //calibrationOK
    if(subStr[0] == 79 && subStr[1] == 75)
    {
        calibrationOK = 1;
    }
    else
    {
        if(calibrationFlag != 1)
        {
            degree = (float)strtol(subStr, &endPtr, 10)/10.0;
            data->orientation = normaliseAngle(degree * M_PI / 180.0f +
                                               (float)compassOffset * M_PI / 180.0f);
            if (*endPtr != 0)
            {
                GDOS_ERROR("Cannot read orientation from serial data");
                return -EIO;
            }
        }
    }
    return 0;
}

int CompassCmps03::calibrateCompass()
{
    int ret;
    unsigned char startByte = 0x7E; // 0x7E = '~'
    unsigned char ackByte   = 0x23; // 0x23 = '#'

    switch (state)
    {
        case CALIBRATE_STATE_IDLE:
            if (calibrationFlag == 1)
            {
                GDOS_PRINT("Calibration requested\n");
                ordinalCount = 1;
                state = CALIBRATE_STATE_1_PREALIGN;
            }
            break;

        case CALIBRATE_STATE_1_PREALIGN:
            GDOS_PRINT("Orientate the compass towards the %i. ordinal direction and "
                       "set the AlignedFlag\n",ordinalCount);
            state = CALIBRATE_STATE_1_ALIGN;
            break;

        case CALIBRATE_STATE_1_ALIGN:
            if (compassAlignedFlag == 1)
            {
                state = CALIBRATE_STATE_1;
                // send Calibration StartByte
                ret = serialPort.send(&startByte, 1);
                if (ret)
                {
                    GDOS_ERROR("Can't send CalibrateStartByte to rtser%d\n", serialDev );
                    return ret;
                }
            }
            break;

        case CALIBRATE_STATE_1:
            if(calibrationOK == 1)
            {
                state = CALIBRATE_STATE_1_OK;
            }
            break;

         case CALIBRATE_STATE_1_OK:
            ret = serialPort.send(&ackByte, 1);
            if (ret)
            {
                GDOS_ERROR("Can't send AckByte to rtser%d\n", serialDev );
                return ret;
            }
            compassAlignedFlag = 0;
            setInt32Param("compassAligned", compassAlignedFlag);
            calibrationOK = 0;

            if(ordinalCount >= 4)
            {   calibrationFlag = 0;
                state = CALIBRATE_STATE_IDLE;
                setInt32Param("calibration", 0);
                GDOS_PRINT("Calibration fully completed\n");
            }
            else
            {
                GDOS_PRINT("Calibration of the %i. ordinal direction successful\n",ordinalCount);
                ordinalCount++;
                state = CALIBRATE_STATE_1_PREALIGN;
            }
            break;

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

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE                0
#define INIT_BIT_SERIALPORT_OPEN            1
#define INIT_BIT_MBX_WORK                   2

int CompassCmps03::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    ret = serialPort.open(serialDev, &serial_config, this);
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

void CompassCmps03::moduleCleanup(void)
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

CompassCmps03::CompassCmps03()
        : RackDataModule( MODULE_CLASS_ID,
                      5000000000llu,    // 5s datatask error sleep time
                      16,               // command mailbox slots
                      48,               // command mailbox data size per slot
                      MBX_IN_KERNELSPACE | MBX_SLOT, // command mailbox flags
                      5,                // max buffer entries
                      10)               // data buffer listener
{
    // get static module parameter
    serialDev                = getIntArg("serialDev", argTab);
    serial_config.baud_rate  = getIntArg("baudrate", argTab);
    dataBufferMaxDataSize    = sizeof(compass_data);
}

int main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "CompassCmps03");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new CompassCmps03
    CompassCmps03 *pInst;

    pInst = new CompassCmps03();
    if (!pInst)
    {
        printf("Can't create new CompassCmps03 -> EXIT\n");
        return -ENOMEM;
    }

    // init
    ret = pInst->moduleInit();
    if (ret)
        goto exit_error;

    pInst->run();

    return 0;

exit_error:

    delete (pInst);

    return ret;
}
