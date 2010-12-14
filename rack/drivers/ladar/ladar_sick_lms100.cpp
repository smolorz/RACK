/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2009-2009 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Axel Acosta <axjacosta@gmail.com>
 * NOTE: LMS 100 must be have "Auto Start Measure" mode enabled.
 *
 */

#include <iostream>

#include "ladar_sick_lms100.h"

//
// data structures
//

LadarSickLms100 *pInst;

arg_table_t argTab[] = {

    { ARGOPT_REQ, "lmsIp", ARGOPT_REQVAL, ARGOPT_VAL_STR,
      "Ip address of the LMS 100", { 0 } },

    { ARGOPT_OPT, "lmsPort", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Port address of the LMS 100, default '2112'", { 2112 } },

    { ARGOPT_OPT, "reflectorRemission", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Minimum remission intensity for a reflector, default '1000'", {1000} },

    { 0, "", 0, 0, "", { 0 } } // last entry
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

int  LadarSickLms100::moduleOn(void)
{
    int ret;

    RackTask::disableRealtimeMode();

    // read dynamic module parameter
    lmsIp              = getStringParam("lmsIp");
    lmsPort            = getInt32Param("lmsPort");
    reflectorRemission = getInt32Param("reflectorRemission");

    //preparing tcp Socket
    inet_pton(AF_INET, lmsIp, &(tcpAddr.sin_addr));
    tcpAddr.sin_port   = htons((unsigned short)lmsPort);
    tcpAddr.sin_family = AF_INET;
    bzero(&(tcpAddr.sin_zero), 8);

    //openning tcp Socket
    GDOS_DBG_INFO("open network socket...\n");
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == -1)
    {
        GDOS_ERROR("Can't create tcp Socket, (%d)\n",errno);
        return -errno;
    }

    //connect to tcp Socket
    GDOS_DBG_INFO("Connect to network socket\n");
    ret = connect(tcpSocket, (struct sockaddr *)&tcpAddr, sizeof(tcpAddr));
    if(ret)
    {
       GDOS_ERROR("Can't connect to tcp Socket, (%d)\n",errno);
       return errno;
    }
    GDOS_DBG_INFO("Turn on ladar\n");
    RackTask::enableRealtimeMode();

    return RackDataModule::moduleOn();   // has to be last command in moduleOn();
}

void LadarSickLms100::moduleOff(void)
{
    RackDataModule::moduleOff();         // has to be first command in moduleOff();

    // closing tcp Socket
    RackTask::disableRealtimeMode();

    if(tcpSocket !=-1)
    {
        close(tcpSocket);
        tcpSocket = -1;
    }
    RackTask::enableRealtimeMode();
}

int  LadarSickLms100::moduleLoop(void)
{
    ladar_data    *pData = NULL;
    uint32_t      datalength;
    int           len;
    int           ret=0;
    int           n=0;
    int           i=0;
    int           j=0;
    char          tmpbuff[10];
    char          buff[1460];
    rack_time_t   scanTime;

    // get datapointer from rackDataBuffer
    pData = (ladar_data*)getDataBufferWorkSpace();

    RackTask::disableRealtimeMode();

    memset(buff, '\0', sizeof(buff));
    memset(&(ladarHeader),0, sizeof(ladarHeader));


    ret = send(tcpSocket,START_MEAS,strlen(START_MEAS),0);

    scanTime = rackTime.get();
    if (ret < 0)
    {
        GDOS_ERROR("Error receiving data, (%d)",errno);
        return -errno;
    }

    //EXTRACTING FIRST PART OF HEADER (ALWAYS CONSTANT AMOUNT OF BYTES)
    ret=recv(tcpSocket,&ladarHeader, 32*sizeof(char),0);
    if (ret < 1)
    {
        ret= recvfail(ret,errno);
        return ret;
    }


    //EXTRACTING THE NON CONSTANT COUNTERS
    ret = extractHeader( ladarHeader.messagecounter, tcpSocket);
    if (ret < 1)
    {
        ret= recvfail(ret,errno);
        return ret;
    }
    ladarHeader.white8 = ' ';

    ret = extractHeader( ladarHeader.scancounter, tcpSocket);
    if (ret < 1)
    {
        ret= recvfail(ret,errno);
        return ret;
    }
    ladarHeader.white9 = ' ';

    ret = extractHeader( ladarHeader.powerupduration, tcpSocket);
    if (ret < 1)
    {
        ret= recvfail(ret,errno);
        return ret;
    }
    ladarHeader.white10 = ' ';

    ret = extractHeader( ladarHeader.transduration, tcpSocket);
    if (ret < 1)
    {
        ret= recvfail(ret,errno);
        return ret;
    }
    ladarHeader.white11 = ' ';

    //EXTRACTING THE REST OF THE CONSTANT PART OF HEADER
    ret = recv(tcpSocket,&(ladarHeader.inputstatus), 10*sizeof(char),0);
    if (ret < 1)
    {
        ret= recvfail(ret,errno);
        return ret;
    }
    ret = extractHeader( ladarHeader.scanfreq, tcpSocket);
    if (ret < 1)
    {
        ret= recvfail(ret,errno);
        return ret;
    }
    ladarHeader.white16 = ' ';

    ret = recv(tcpSocket,&(ladarHeader.measfreq), 41*sizeof(char),0);
    if (ret < 1)
    {
        ret= recvfail(ret,errno);
        return ret;
    }

    ret = extractHeader( ladarHeader.angstep, tcpSocket);
    if (ret < 1)
    {
        ret= recvfail(ret,errno);
        return ret;
    }
    ladarHeader.white23 = ' ';

    ret = recv(tcpSocket,&(ladarHeader.numdata), 4*sizeof(char),0);
    if (ret < 1)
    {
        ret= recvfail(ret,errno);
        return ret;
    }

    memcpy(tmpbuff, &(ladarHeader.numdata),3);
    n=hextodec(tmpbuff,3);
    //EXTRACTING DATA POINTS


    for (i=0; i < n; i++)
    {
        do
        {
            len= recv(tcpSocket,&(tmpbuff[j]),1*sizeof(char), 0);
            if (ret < 1)
            {
                ret= recvfail(ret,errno);
                return ret;
            }
            j++;
        }
        while (tmpbuff[j-1] != ' ');
        j--;
        ladarHeader.dataPoints[i]= hextodec(tmpbuff,j );
        j=0;
    }

    ret = recv(tcpSocket, buff, sizeof(char), 0);

    if (buff[0] == 'R')
    {
        //Extracting Remission header (no important data)
        ret = recv(tcpSocket, buff, 43*sizeof(char), 0);

        //Extracting remission values for the points

        for (i=0; i < n; i++)
        {
            do
            {
                len= recv(tcpSocket,&(tmpbuff[j]),1*sizeof(char), 0);
                if (ret < 1)
                {
                    ret= recvfail(ret,errno);
                    return ret;
                }
                j++;
            }
            while (tmpbuff[j-1] != ' ');
            j--;
            ladarHeader.dataRemission[i]= hextodec(tmpbuff,j );
            j=0;
        }
    }
    //FlUSHING REST OF DATA PACKAGE IN BUFER
    ret = recv(tcpSocket, buff, sizeof(buff), 0);

    if (n == 1082)
    {
        j=3;
    }
    else
    {
        j=4;
    }
    RackTask::enableRealtimeMode();

     // create ladar data message
     pData->recordingTime = scanTime;
     pData->duration      = hextodec(ladarHeader.measfreq,3)*1000;
     pData->maxRange      = LADAR_MAX_RANGE;
     pData->startAngle    = M_PI * (hextodec(ladarHeader.startangle,8)-900000)/ 1800000;
     pData->endAngle      = M_PI * hextodec(ladarHeader.angstep,j) * n / 1800000;
     pData->pointNum      = n;

     for (i = 0; i < n; i++)
     {
        pData->point[i].angle    = pData->startAngle + (i * M_PI * hextodec(ladarHeader.angstep,j)/ 1800000);
        pData->point[i].distance = ladarHeader.dataPoints[n-i];

        pData->point[i].type = 0;

        if (ladarHeader.dataRemission[n-i] < reflectorRemission)
        {
            pData->point[i].type |= LADAR_POINT_TYPE_UNKNOWN;
        }
        else
        {
            pData->point[i].type |= LADAR_POINT_TYPE_REFLECTOR;
        }
        pData->point[i].intensity = ladarHeader.dataRemission[n-i];
     }

     GDOS_DBG_DETAIL("recordingTime %d, pointNum %d\n", pData->recordingTime, pData->pointNum);

    // write data buffer slot (and send it to all listeners)
    datalength = sizeof(ladar_data) + sizeof(ladar_point) * pData->pointNum;
    putDataBufferWorkSpace(datalength);

    return 0;
}


int LadarSickLms100::recvfail (int ret, int error)
{
    if (ret==-1)
    {
        GDOS_ERROR("Error receiving data, (%d)",errno);
    }
    else if (ret==0)
    {
        GDOS_ERROR("Session closed");
    }
    return -1;
}


int  LadarSickLms100::moduleCommand(RackMessage *msgInfo)
{
    switch (msgInfo->getType())
    {
        default:
            // not for me -> ask RackDataModule
            return RackDataModule::moduleCommand(msgInfo);
    }
    return 0;
}

int LadarSickLms100::extractHeader(char *header, int Socket )
{

    int i=0;
    int a;
    char j;

    do
    {
        a=recv(Socket,(void *) &j, sizeof(char),0);
        if (a < 0)
        {
            a = recvfail (a,errno);
            return -1;
        }
        if ( j != ' ')
        {
            header[i]=j;
            i++;
        }
    }
    while (j != ' ');
    return i;
}

int LadarSickLms100::hextodec (char tmpbuff[], int n)
{
    int i;
    uint32_t result=0;

    for(i=0;n>0;i++)
    {
        if((int)tmpbuff[i]<=57)
        {
            result += (((int)tmpbuff[i])-48)*pow(16,--n);
        }
        else
        {
            result += (((int)tmpbuff[i])-55)*pow(16,--n);
        }
    }
    return result;
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

int  LadarSickLms100::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    return 0;

init_error:
    LadarSickLms100::moduleCleanup();
    return ret;
}

// non realtime context
void LadarSickLms100::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // free own stuff
    // ...

    // free proxies
    // ...

    // delete mailboxes
    // ...
}

LadarSickLms100::LadarSickLms100()
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    5,                // max buffer entries
                    10)               // data buffer listener
{
    dataBufferMaxDataSize   = sizeof(ladar_data_msg);
    dataBufferPeriodTime    = 20; // 20 ms (50 per sec)
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "LadarSickLms100");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new LadarSickLms100
    pInst = new LadarSickLms100();
    if (!pInst)
    {
        printf("Can't create new LadarSickLms100 -> EXIT\n");
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

