/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2009-2009 Leibniz University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Axel Acosta <axjacosta@hotmail.com>
 *
 */
#ifndef __LADAR_SICK_LMS100_H__
#define __LADAR_SICK_LMS100_H__

#include <main/rack_data_module.h>
#include <drivers/ladar_proxy.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// define module class
#define MODULE_CLASS_ID     LADAR

#define START_MEAS                          "\02sRN LMDscandata\03"
#define LADAR_MAX_RANGE                      20000

typedef struct
{
    ladar_data          data;
    ladar_point         point[LADAR_DATA_MAX_POINT_NUM];
} __attribute__((packed)) ladar_data_msg;

typedef struct
{
    char                 answer[4];
    char                 white1;
    char                 command[11];
    char                 white2;
    char                 versioninfo;
    char                 white3;
    char                 deviceid;
    char                 white4;
    char                 factoryserialnum[6];
    char                 white5;
    char                 status;
    char                 white6;
    char                 status2;
    char                 white7;
    char                 messagecounter[4];
    char                 white8;
    char                 scancounter[4];
    char                 white9;
    char                 powerupduration[8];
    char                 white10;
    char                 transduration[8];
    char                 white11;
    char                 inputstatus;
    char                 white12;
    char                 noidea;
    char                 white13;
    char                 outputstatus;
    char                 white14;
    char                 noidea2[3];
    char                 white15;
    char                 scanfreq[4];
    char                 white16;
    char                 measfreq[3];
    char                 white17;
    char                 noidea3[3];
    char                 white18;
    char                 measdatacontent[5];
    char                 white19;
    char                 scalingfactor[8];
    char                 white20;
    char                 scalingoffset[8];
    char                 white21;
    char                 startangle[8];
    char                 white22;
    char                 angstep[4];
    char                 white23;
    char                 numdata[3];
    char                 white24;
    int                  dataPoints[1082];
    int                  dataRemission[1082];
}__attribute__((packed)) ladar_lms100_header;

//######################################################################
//# class NewRackDataModule
//######################################################################

class LadarSickLms100 : public RackDataModule {
    private:

        int                  tcpSocket;
        struct               sockaddr_in tcpAddr;
        ladar_lms100_header  ladarHeader;
        char                 *lmsIp;
        int                  lmsPort;
        int                  reflectorRemission;

    protected:

        // -> realtime context
        int  moduleOn(void);
        void moduleOff(void);
        int  moduleLoop(void);
        int  moduleCommand(RackMessage *msgInfo);

        // -> non realtime context
        void moduleCleanup(void);

        int extractHeader(char *, int );
        int hextodec(char tmpbuff[], int n);
        int recvfail(int , int );

    public:

        // constructor und destructor
        LadarSickLms100();
        ~LadarSickLms100() {};

        // > non realtime context
        int  moduleInit(void);

};

#endif // __LADAR_SICK_LMS100_H__

