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
#include "gyro_xsens.h"

//
// init data structures
//

arg_table_t argTab[] = {

    { ARGOPT_REQ, "serialDev", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Serial device number", { -1 } },

    { ARGOPT_OPT, "baudrate", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Working serial baudrate, default 115200", { 115200 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};

const struct rtser_config serialConfig = {
    config_mask       : 0xFFFF,
    baud_rate         : 115200,
    parity            : RTSER_NO_PARITY,
    data_bits         : RTSER_8_BITS,
    stop_bits         : RTSER_2_STOPB,
    handshake         : RTSER_DEF_HAND,
    fifo_depth        : RTSER_DEF_FIFO_DEPTH,
    rx_timeout        : 50000000ll,
    tx_timeout        : RTSER_DEF_TIMEOUT,
    event_timeout     : 50000000ll,
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
int  GyroXsens::moduleOn(void)
{
    int            ret;

    // cleaning of serialport
    ret = serialPort.clean();
    if (ret)
    {
        GDOS_ERROR("Can't clean serial port, code = %d \n", ret);
    }

    return RackDataModule::moduleOn();  // has to be last command in moduleOn();
}

void GyroXsens::moduleOff(void)
{
  RackDataModule::moduleOff();          // has to be first command in moduleOff();
}

int  GyroXsens::moduleLoop(void)
{
    gyro_data *pData;
    int        ret;


    // get datapointer from databuffer
    pData = (gyro_data *)getDataBufferWorkSpace();

    // read next message from gyro
    ret = readMessage(&gyroMessage, &(pData->recordingTime));
    if (ret)
    {
        GDOS_ERROR("Can't read message from serial dev %i, code %d\n", serialDev, ret);
        return ret;
    }

    // parse message in dependance on message id
    switch (gyroMessage.mid)
    {
        case GYRO_XSENS_MESSAGE_MTDATA:
            ret = parseMTDataMessage(&gyroMessage, pData);
            if (ret)
            {
                GDOS_ERROR("Can't parse MTDataMessage, code %d\n", ret);
            }

            putDataBufferWorkSpace(sizeof(gyro_data));

            GDOS_DBG_DETAIL("data recordingtime %i roll %a pitch %a yaw %a\n",
                            pData->recordingTime, pData->roll, pData->pitch,
                            pData->yaw);
            break;

        default:
            break;
    }

    return 0;
}

int  GyroXsens::moduleCommand(RackMessage *msgInfo)
{
  // not for me -> ask RackDataModule
  return RackDataModule::moduleCommand(msgInfo);
}


int  GyroXsens::readMessage(gyro_xsens_message *message, rack_time_t *recordingTime)
{
    int     i;
    int     ret;
    uint8_t checksum;

    // init variables
    i                 = 0;
    message->preamble = 0;

    // synchronize to preamble, timeout after 200 attempts
    while ((i < 200) && (message->preamble != GYRO_XSENS_MESSAGE_PREAMBLE))
    {
        // wait for next event on serial port
        ret = serialPort.recv(&message->preamble, 1, recordingTime);
        if (ret)
        {
            GDOS_ERROR("Can't read message preamble on serial dev %i, code %d\n",
                       serialDev, ret);
            return ret;
        }
    }
    i++;

    // check for timeout
    if (i >= 200)
    {
        GDOS_ERROR("Can't synchronize on message head\n");
        return -ETIME;
    }

    // read message header
    ret = serialPort.recv(&message->bid, 3);
    if (ret)
    {
        GDOS_ERROR("Can't read message header on serial dev %i, code %d\n",
                   serialDev, ret);
        return ret;
    }

    // check if extended length message is given
    if (message->len == 0xff)
    {
        ret = serialPort.recv(&message->extLen, 2);
        if (ret)
        {
            GDOS_ERROR("Can't read extended length on serial dev %i, code %d\n",
                       serialDev, ret);
            return ret;
        }
    }
    else
    {
        message->extLen = 0;
    }

    // read message data
    ret = serialPort.recv(&message->data, (message->len + message->extLen));
    if (ret)
    {
        GDOS_ERROR("Can't read message data on serial dev %i, code %d\n",
                   serialDev, ret);
        return ret;
    }

    // read message checksum
    ret = serialPort.recv(&message->checksum, 1);
    if (ret)
    {
        GDOS_ERROR("Can't read message checksum on serial dev %i, code %d\n",
                   serialDev, ret);
        return ret;
    }

    // calculate checksum
    checksum  = 0;
    checksum += message->bid;
    checksum += message->mid;
    checksum += message->len;
    checksum += message->extLen;

    for (i = 0; i < (message->len + message->extLen); i++)
    {
        checksum += message->data[i];
    }
    checksum += message->checksum;

    // valudate checksum
    if (checksum != 0)
    {
        GDOS_ERROR("Wrong checksum in message\n");
//        return -EINVAL;
    }

    GDOS_DBG_DETAIL("%d received message, bid %x, mid %x, len %x, extLen %x\n",
                    *recordingTime, message->bid, message->mid, message->len, message->extLen);

    return 0;
}

int  GyroXsens::parseMTDataMessage(gyro_xsens_message *message, gyro_data *data)
{
    uint16_t sampleCount;
    float    magX, magY, magZ;

    // read calibrated data output
    data->aX     = -getDataFloat(message->data, 0) * 1000.0f;
    data->aY     = -getDataFloat(message->data, 4) * 1000.0f;
    data->aZ     = -getDataFloat(message->data, 8) * 1000.0f;
    data->wRoll  =  getDataFloat(message->data, 12) * M_PI / 180.0f;
    data->wPitch =  getDataFloat(message->data, 16) * M_PI / 180.0f;
    data->wYaw   =  getDataFloat(message->data, 20) * M_PI / 180.0f;
    magX         =  getDataFloat(message->data, 24);
    magY         =  getDataFloat(message->data, 28);
    magZ         =  getDataFloat(message->data, 32);

    // read orientation (12 bytes)
    data->roll  =  getDataFloat(message->data, 36) * M_PI / 180.0f;
    data->pitch =  getDataFloat(message->data, 40) * M_PI / 180.0f;
    data->yaw   =  getDataFloat(message->data, 44) * M_PI / 180.0f;

    // read sample count
    sampleCount = getDataShort(message->data, 48);

    GDOS_DBG_DETAIL("parsed MTDataMessage\n");

    return 0;
}

// return the current value of the data as an uint16_t (16 bits)
uint16_t GyroXsens::getDataShort(uint8_t *data, uint16_t offset)
{
	uint16_t ret;
	uint8_t* dest = (uint8_t*) &ret;
	uint8_t* src = &(data[offset]);
	dest[0] = src[1];
	dest[1] = src[0];

	return ret;
}

// return the current value of the data as a float (32 bits)
float GyroXsens::getDataFloat(uint8_t *data, uint16_t offset)
{
	float ret;
	uint8_t* dest = (uint8_t*)&ret;
	uint8_t* src = &(data[offset]);
	dest[0] = src[3];
	dest[1] = src[2];
	dest[2] = src[1];
	dest[3] = src[0];

	return ret;
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

// init_flags
#define INIT_BIT_DATA_MODULE            0
#define INIT_BIT_SERIALPORT_OPEN        1

int  GyroXsens::moduleInit(void)
{
    int            ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    ret = serialPort.open(serialDev, &serialConfig, this);
    if (ret)
    {
        GDOS_ERROR("Can't open serialDev %i\n", serialDev);
        goto init_error;
    }
    GDOS_DBG_INFO("serialDev %d has been opened \n", serialDev);
    initBits.setBit(INIT_BIT_SERIALPORT_OPEN);

    return 0;

init_error:
    // !!! call local cleanup function !!!
    GyroXsens::moduleCleanup();
    return ret;
}

void GyroXsens::moduleCleanup(void)
{
    // call RackDataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // close serial port
    if (initBits.testAndClearBit(INIT_BIT_SERIALPORT_OPEN))
    {
        serialPort.close();
    }
}

GyroXsens::GyroXsens(void)
      : RackDataModule( MODULE_CLASS_ID,
                    1000000000llu,    // 1s datatask error sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    5,                // max buffer entries
                    10)               // data buffer listener


{
    // get static module parameter
    serialDev = getIntArg("serialDev", argTab);
    baudrate  = getIntArg("baudrate", argTab);

    dataBufferMaxDataSize   = sizeof(gyro_data);
    dataBufferPeriodTime    = 10; // 10 ms (100 per sec)}
};

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "GyroXsens");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    GyroXsens *pInst;

    // create new GyroXsens
    pInst = new GyroXsens();
    if (!pInst)
    {
        printf("Can't create new GyroXsens -> EXIT\n");
        return -ENOMEM;
    }

    ret = pInst->moduleInit();
    if (ret)
        goto exit_error;

    pInst->run();
    return 0;

exit_error:

    delete (pInst);
    return ret;
}
