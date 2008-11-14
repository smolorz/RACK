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
 *      Matthias Hentschel      <hentschel@rts.uni-hannover.de>
 *
 */

#include <iostream>

#include "ladar_ibeo_lux.h"

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE                0

//
// data structures
//

LadarIbeoLux *p_inst;

argTable_t argTab[] = {

    { ARGOPT_REQ, "ladarIp", ARGOPT_REQVAL, ARGOPT_VAL_STR,
      "IP address of the ibeo lux ladar, default '10.152.36.200'", { 0 } },

    { ARGOPT_OPT, "ladarPort", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Port of the ibeo lux ladar, default 12002", { 12002 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};


/*****************************************************************************
 *   !!! REALTIME CONTEXT !!!
 *
 *   moduleOn,
 *   moduleOff,
 *   moduleLoop,
 *   moduleCommand,
 *
 *   own realtime user functions
 ****************************************************************************/
int LadarIbeoLux::moduleOn(void)
{
    int             ret;
    struct timeval  recvTimeout;

    // read dynamic module parameter
    ladarIp   = getStringParam("ladarIp");
    ladarPort = getInt32Param("ladarPort");

    RackTask::disableRealtimeMode();


    // prepare tcp-socket
    GDOS_DBG_INFO("open network socket...\n");
    if ((tcpAddr.sin_addr.s_addr = inet_addr(ladarIp)) == INADDR_NONE)
    {
        GDOS_ERROR("Ip adress is not valid\n");
        return -1;
    }

    tcpAddr.sin_port   = htons((unsigned short)ladarPort);
    tcpAddr.sin_family = AF_INET;
    bzero(&(tcpAddr.sin_zero), 8);

    // open tcp-socket
    tcpSocket = -1;
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == -1)
    {
        GDOS_ERROR("Can't create tcp socket, (%d)\n", errno);
        return -errno;
    }

    // set tcp-socket timeout
    recvTimeout.tv_sec  = 0;
    recvTimeout.tv_usec = 500000;   // 500ms
    ret = setsockopt(tcpSocket, SOL_SOCKET, SO_RCVTIMEO, &recvTimeout, sizeof(struct timeval));
    if (ret == -1)
    {
        GDOS_ERROR("Can't set timeout of tcp socket, (%d)\n", errno);
        return -errno;
    }

    // connect
    GDOS_DBG_INFO("Connect to network socket\n");
    ret = connect(tcpSocket, (struct sockaddr *)&tcpAddr, sizeof(tcpAddr));
    if (ret)
    {
        GDOS_ERROR("Can't connect to tcp socket, (%d)\n", errno);
        return -errno;
    }

    GDOS_DBG_INFO("Turn on ladar\n");

    RackTask::enableRealtimeMode();

    // init variables
    fracFactor = pow(2, -32);

    // has to be last command in moduleOn()
    return RackDataModule::moduleOn();
}

void LadarIbeoLux::moduleOff(void)
{
    RackDataModule::moduleOff(); // has to be first command in moduleOff()

    // close tcp socket
    RackTask::disableRealtimeMode();
    if (tcpSocket != -1)
    {
        close(tcpSocket);
        tcpSocket = -1;
    }

    GDOS_DBG_INFO("Closed tcp socket\n");
    RackTask::enableRealtimeMode();
}

int  LadarIbeoLux::moduleLoop(void)
{
    int                         ret, i;
    double                      scanStartTime;
    double                      scanEndTime;
    double                      ladarSendTime;
    uint32_t                    datalength      = 0;
    ladar_data                  *p_data         = NULL;
    ladar_ibeo_lux_scan_data    *ladarScanData  = NULL;

    // get datapointer from rackdatabuffer
    p_data = (ladar_data *)getDataBufferWorkSpace();

    RackTask::disableRealtimeMode();

    // read ladar header
    p_data->recordingTime = rackTime.get();
    ret = recvLadarHeader(&ladarHeader, 1000);
    if (ret)
    {
        return ret;
    }

    GDOS_DBG_DETAIL("dataType %x, messageSize %d\n", ladarHeader.dataType, ladarHeader.messageSize);

    // read ladar data body
    ret = recvLadarData(&ladarData, ladarHeader.messageSize);

    // parse ladar data body
    switch (ladarHeader.dataType)
    {
        case LADAR_IBEO_LUX_SCAN_DATA:
            ladarScanData = (ladar_ibeo_lux_scan_data *)&ladarData;

            // calculate ladar timestamps (unit seconds)
            scanStartTime = (double)ladarScanData->scanStartTime.secondsFrac +
                                    fracFactor * (double)ladarScanData->scanStartTime.seconds;
            scanEndTime   = (double)ladarScanData->scanEndTime.secondsFrac +
                                    fracFactor * (double)ladarScanData->scanEndTime.seconds;
            ladarSendTime = (double)ladarHeader.ntpTime.seconds +
                                    fracFactor * (double)ladarHeader.ntpTime.secondsFrac;

            // recalculate recordingtime to the center of the scan
            p_data->recordingTime  -= (int)rint((ladarSendTime - scanEndTime +
                                                ((scanEndTime - scanStartTime) / 2.0)) * 1000.0);

            // create ladar data message
            p_data->duration        = (int)rint((scanEndTime - scanStartTime) * 1000.0);
            p_data->maxRange        = LADAR_IBEO_LUX_MAX_RANGE;
            p_data->startAngle      = -(2.0 * M_PI * ladarScanData->startAngle) /
                                        ladarScanData->angleTicksPerRot;
            p_data->endAngle        = -(2.0 * M_PI * ladarScanData->endAngle) /
                                        ladarScanData->angleTicksPerRot;
            p_data->pointNum        = ladarScanData->scanPoints;


            for (i = 0; i < ladarScanData->scanPoints; i++)
            {
                p_data->point[i].angle    = -(2.0 * M_PI * ladarScanData->point[i].angle) /
                                              ladarScanData->angleTicksPerRot;
                p_data->point[i].distance = ladarScanData->point[i].distance * 100;
                p_data->point[i].type     = ladarScanData->point[i].flags;

            }

            datalength = sizeof(ladar_data) + sizeof(ladar_point) * p_data->pointNum; // points
            putDataBufferWorkSpace(datalength);
            break;

        case LADAR_IBEO_LUX_OBJ_DATA:
            //ladarObjectData = (ladar_ibeo_lux_obj_data *)&ladarData;
            break;

    }
    RackTask::enableRealtimeMode();

    return 0;
}

int  LadarIbeoLux::moduleCommand(message_info *p_msginfo)
{
    // not for me -> ask RackDataModule
    return RackDataModule::moduleCommand(p_msginfo);
}


int LadarIbeoLux::recvLadarHeader(ladar_ibeo_lux_header *data, unsigned int retryNumMax)
{
    int          ret;
    unsigned int len, i;
    uint32_t     magicWord = 0;

    // synchronize to header by reading the magic word (4 bytes)
    for (i = 0; i < retryNumMax; i++)
    {
        len = 0;
        while (len < sizeof(magicWord))
        {
            ret = recv(tcpSocket, (char*)&magicWord + len, sizeof(magicWord) - len, 0);
            if (ret == -1)          // error
            {
                if (errno == EAGAIN)
                {
                    GDOS_ERROR("Timeout on receiving ladar header, (%d)\n", errno);
                }
                else
                {
                    GDOS_ERROR("Can't receive header magic word, (%d)\n", errno);
                }
                return -errno;
            }
            if (ret == 0)           // socket closed
            {
                GDOS_ERROR("Tcp socket closed\n");
                return -1;
            }
            len += ret;
        }

        // adjust byteorder and search for magic word
        magicWord = __be32_to_cpu(magicWord);
        if (magicWord == LADAR_IBEO_LUX_MAGIC_WORD)
        {
            break;
        }
    }

    // synchronization timeout
    if (i >= retryNumMax)
    {
        GDOS_ERROR("Can't synchronize to ladar header, timeout after %d attempts\n", i);
        return -ETIMEDOUT;
    }

    // receive header data
    len = 0;
    while (len < sizeof(ladar_ibeo_lux_header))
    {
        ret = recv(tcpSocket, (char*)data + len, sizeof(ladar_ibeo_lux_header) - len, 0);
        if (ret == -1)              // error
        {
            if (errno == EAGAIN)
            {
                GDOS_ERROR("Timeout on receiving ladar header, (%d)\n", errno);
            }
            else
            {
                GDOS_ERROR("Can't receive ladar header, (%d)\n", errno);
            }
            return -errno;
        }
        if (ret == 0)               // socket closed
        {
            GDOS_ERROR("Tcp socket closed\n");
            return -1;
        }
        len += ret;
    }

    // adjust byte order
    parseLadarIbeoLuxHeader(data);

    return 0;
}

int LadarIbeoLux::recvLadarData(void *data, unsigned int messageSize)
{
    int          ret;
    unsigned int len;

    // receive ladar data
    len = 0;
    while (len < messageSize)
    {
        ret = recv(tcpSocket, (char*)data + len, messageSize - len, 0);
        if (ret == -1)              // error
        {
            if (errno == EAGAIN)
            {
                GDOS_ERROR("Timeout on receiving ladar data, (%d)\n", errno);
            }
            else
            {
                GDOS_ERROR("Can't receive ladar data, (%d)\n", errno);
            }
            return -errno;
        }
        if (ret == 0)               // socket closed
        {
            GDOS_ERROR("Tcp socket closed\n");
            return -1;
        }
        len += ret;
    }

    return 0;
}

/*****************************************************************************
 *   !!! NON REALTIME CONTEXT !!!
 *
 *   moduleInit,
 *   moduleCleanup,
 *   Constructor,
 *   Destructor,
 *   main,
 *
 *   own non realtime user functions
 ****************************************************************************/
int LadarIbeoLux::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    return 0;
}

// non realtime context
void LadarIbeoLux::moduleCleanup(void)
{
    // call RackDataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }
}

LadarIbeoLux::LadarIbeoLux()
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    5,                // max buffer entries
                    10)               // data buffer listener
{
    dataBufferMaxDataSize   = sizeof(ladar_data_msg);
    dataBufferPeriodTime    = 100; // 100 ms (10 per sec)
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "LadarIbeoLux");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new NewRackDataModule
    p_inst = new LadarIbeoLux();
    if (!p_inst)
    {
        printf("Can't create new LadarIbeoLux -> EXIT\n");
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
