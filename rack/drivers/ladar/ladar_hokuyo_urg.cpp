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
 *      Daniel Lecking   <lecking@rts.uni-hannover.de>
 *
 */
#include <iostream>

// include own header file
#include "ladar_hokuyo_urg.h"

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE                0
#define INIT_BIT_SERIALPORT_OPEN            1

LadarHokuyoUrg *p_inst;

argTable_t argTab[] = {
  {ARGOPT_REQ, "serialDev", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "The number of the local serial device", (int)-1 },
  {ARGOPT_OPT, "start", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "Point of the area from where the data reading starts, default 44",
   (int)44 },
  {ARGOPT_OPT, "end", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "Point of the area from where the data reading stops, default 725",
   (int)725 },
  {ARGOPT_OPT, "cluster", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "Number of neighboring points that are grouped as a cluster, default 3",
   (int)3 },
  {ARGOPT_OPT, "startAngle", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "startAngle, default 120", (int)120 },
  {0,"",0,0,""}
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

int  LadarHokuyoUrg::moduleOn(void)
{
    int ret;

    GDOS_DBG_INFO("Connect\n");

    ret = serialPort.send(sCommand115200, 15);
    if (ret)
    {
        GDOS_ERROR("Can't send S-Command to serial dev,code=%d\n", ret);
        return ret;
    }

    GDOS_DBG_INFO("Set baudrate to 115200\n");

    ret = serialPort.recv(serialBuffer, 1);
    if (ret)
    {
        GDOS_WARNING("Can't read set baudrate ack, code=%d\n", ret);
        GDOS_WARNING("Try on work baudrate 115200\n");
    }

    RackTask::sleep(200000000); // 200 ms

    ret = serialPort.setBaudrate(115200);
    if (ret)
    {
        GDOS_ERROR("Can't set baudrate to 115200, code=%d\n", ret);
        return ret;
    }

    RackTask::sleep(200000000); // 200 ms

    ret = serialPort.clean();    
    if (ret)
    {
        GDOS_ERROR("Can't clean serial port\n", ret);
        return ret;
    }

    return DataModule::moduleOn();   // have to be last command in moduleOn();
}

void LadarHokuyoUrg::moduleOff(void)
{
    DataModule::moduleOff();         // have to be first command in moduleOff();

    GDOS_DBG_INFO("Disconnect\n");

    serialPort.send(sCommand19200, 15);

    RackTask::sleep(200000000); // 200 ms

    serialPort.setBaudrate(19200);
}

int  LadarHokuyoUrg::moduleLoop(void)
{
    ladar_data  *p_data       = NULL;
    uint32_t      datalength = 0;
    int serialDataLen;
    int i, j, ret;

    p_data = (ladar_data *)getDataBufferWorkSpace();

    p_data->duration        = 1000;
    p_data->maxRange        = 4000;

    // send G-Command (Distance Data Acquisition)
    gCommand[1] = (int)(start/100)      + 0x30;
    gCommand[2] = ((int)(start/10))%10  + 0x30;
    gCommand[3] = (start%10)            + 0x30;

    gCommand[4] = (int)(end/100)        + 0x30;
    gCommand[5] = ((int)(end/10))%10    + 0x30;
    gCommand[6] = (end%10) + 0x30;

    gCommand[7] = (int)(cluster/10)     + 0x30;
    gCommand[8] = (cluster%10)          + 0x30;

    ret = serialPort.send(gCommand, 10);
    if (ret)
    {
        GDOS_ERROR("Can't send G-Command to serial dev, code=%d\n", ret);
        return ret;
    }

    // receive G-Command (Distance Data Acquisition)

    // synchronize on message head, timeout after 200 attempts *****
    i = 0;
    serialBuffer[0] = 0;

    while((i < 2000) && (serialBuffer[0] != 'G'))
    {
        // Read next character
        ret = serialPort.recv(serialBuffer, 1, &(p_data->recordingTime), 2000000000ll);
        if (ret)
        {
            GDOS_ERROR("Can't read data from serial dev, code=%d\n", ret);
            return ret;
        }
        i++;
    }

    if(i == 2000)
    {
        GDOS_ERROR("Can't read data 2 from serial dev\n");
        return -1;
    }
    p_data->distanceNum     = (int32_t)((end - start)/cluster);     //max 681
    p_data->startAngle      = (float32_t)startAngle * M_PI/180.0;
    p_data->angleResolution = (float32_t)cluster * -0.3515625 * M_PI/180.0;

    serialDataLen = 15 + p_data->distanceNum * 2 + p_data->distanceNum / 32;

    ret = serialPort.recv(serialBuffer, serialDataLen, NULL, 5000000000ll);
    if (ret)
    {
        GDOS_ERROR("Can't read data 3 from serial dev, code=%d\n",ret);
        return ret;
    }

    j = 10;
    for(i = 0; i < p_data->distanceNum; i++)
    {
        if((i % 32) == 0)
        {
            j += 1;  // first j = 11
        }
        p_data->distance[i] = ((int32_t)(serialBuffer[j] - 0x30) << 6) | (int32_t)(serialBuffer[j + 1] - 0x30);

        if(p_data->distance[i] < 20)
        {
            p_data->distance[i] = 0;
        }

        j += 2;
    }

    p_data->recordingTime = get_rack_time();

    datalength = sizeof(ladar_data) + sizeof(int32_t) * p_data->distanceNum; // points

    // write data buffer slot (and send it to all listener)
    if (datalength > 0 && datalength <= getDataBufferMaxDataSize() )
    {
        putDataBufferWorkSpace( datalength );
        return 0;
    }
    return -ENOSPC;
}

int  LadarHokuyoUrg::moduleCommand(MessageInfo *msgInfo)
{
    // not for me -> ask DataModule
    return DataModule::moduleCommand(msgInfo);
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

int  LadarHokuyoUrg::moduleInit(void)
{
    int ret;

    ret = DataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    ret = serialPort.open(serialDev, &urg_serial_config);
    if (ret)
    {
        GDOS_ERROR("Can't open serialDev %i, code=%d\n", serialDev, ret);
        goto init_error;
    }

    GDOS_DBG_INFO("serialDev %d has been opened \n", serialDev);
    initBits.setBit(INIT_BIT_SERIALPORT_OPEN);
    return 0;

init_error:

    // !!! call local cleanup function !!!
    LadarHokuyoUrg::moduleCleanup();
    return ret;
}

// non realtime context
void LadarHokuyoUrg::moduleCleanup(void)
{
    int ret;

    if (initBits.testAndClearBit(INIT_BIT_SERIALPORT_OPEN))
    {
        ret = serialPort.close();
        if (!ret)
            GDOS_DBG_DETAIL("Serial port closed \n");
        else
            GDOS_ERROR("Can't close serial port, code = %d \n", ret);
    }
    // call DataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        DataModule::moduleCleanup();
    }
}

LadarHokuyoUrg::LadarHokuyoUrg()
      : DataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s cmdtask error sleep time
                    5000000000llu,    // 5s datatask error sleep time
                     100000000llu,    // 100ms datatask disable sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    5,                // max buffer entries
                    10)               // data buffer listener
{

    // get value(s) out of argument table
    serialDev   = getIntArg("serialDev", argTab);
    start       = getIntArg("start", argTab);
    end         = getIntArg("end", argTab);
    cluster     = getIntArg("cluster", argTab);
    startAngle  = getIntArg("startAngle", argTab);

    if (start > 44)
    {
        startAngle  = startAngle + 681 / (start - 44) * 240;
    }

    if (cluster < 0 || cluster > 3)
    {
        printf("Invalid argument -> cluster [use 1..3] \n");
    }

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(ladar_data_msg));

    // set databuffer period time
    setDataBufferPeriodTime(10);
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = Module::getArgs(argc, argv, argTab, "LadarHokuyoUrg");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new LadarHokuyoUrg

    p_inst = new LadarHokuyoUrg();
    if (!p_inst)
    {
        printf("Can't create new LadarHokuyoUrg -> EXIT\n");
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
