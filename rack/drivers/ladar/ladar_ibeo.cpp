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


// Driver for IBEO Laserscanner LD A OEM (User Protocol Service Version 1.0 from 2001)
// Communication CAN-Bus (1MBit, 11Bit addr)
// Configuration via win tool LD config

// Timestamp synchronisation is not jet implemented

#include <iostream>

// include own header file
#include "ladar_ibeo.h"

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE                0
#define INIT_BIT_CAN_OPEN                   1


#define PROFILEFORMAT_PROFILESENT  (1 << 0)
#define PROFILEFORMAT_PROFILECOUNT (1 << 1)
#define PROFILEFORMAT_LAYERNUM     (1 << 2)
#define PROFILEFORMAT_SECTORNUM    (1 << 3)
#define PROFILEFORMAT_DIRSTEP      (1 << 4)
#define PROFILEFORMAT_POINTNUM     (1 << 5)
#define PROFILEFORMAT_TSTART       (1 << 6)
#define PROFILEFORMAT_STARTDIR     (1 << 7)
#define PROFILEFORMAT_DISTANCE     (1 << 8)
#define PROFILEFORMAT_TEND         (1 << 11)
#define PROFILEFORMAT_ENDDIR       (1 << 12)
#define PROFILEFORMAT_SENSTAT      (1 << 13)

#define IDLE_MODE    0x1
#define ROTATE_MODE  0x2
#define MEASURE_MODE 0x3

LadarIbeo *p_inst;

//
// data structures
//
typedef struct {
    unsigned int senstat;
} status_response;

typedef struct {
    unsigned short rev;
} trans_rot_request;

struct _trans_measure_response{
    unsigned int   senstat;
    unsigned short errorcode;
} __attribute__ ((packed));
typedef struct _trans_measure_response trans_measure_response;

typedef struct {
    unsigned short syncabs;
} set_time_abs;

typedef struct {
    unsigned short synctime;
} set_time_abs_responce;

typedef struct {
    unsigned short profilenum;
    unsigned short profileformat;
} get_profile_request;

typedef struct {
    uint16_t word[4];
} uint16_package;

typedef struct {
    ladar_data    data;
    int32_t       distance[LADAR_DATA_MAX_DISTANCE_NUM];
} __attribute__((packed)) ladar_data_msg;

struct ladar_sockaddr_can {
    struct sockaddr_can  scan;
    rtcan_filter_t       filter[1];
};

argTable_t argTab[] = {

    { ARGOPT_REQ, "canDev", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "CAN device number", { -1 } },

    { ARGOPT_REQ, "sensorId", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "CAN Sensor ID", { -1 } },

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

int LadarIbeo::moduleOn(void)
{
    int ret;

    if(getSensorStatus() == -1)
        return -1;

    timeOffsetSector[0] = 0;
    timeOffsetSector[1] = 0;
    timeOffsetSector[2] = 0;
    timeOffsetSector[3] = 0;
    timeOffsetSector[4] = 0;
    timeOffsetSector[5] = 0;
    timeOffsetSector[6] = 0;
    timeOffsetSector[7] = 0;

    GDOS_DBG_DETAIL("getSensorStatus ... \n");

    if(setTimeAbs() == -1)
        return -1;

    GDOS_DBG_DETAIL("setTimeAbs ... \n");

    // setting receive timeout
    ret = canPort.setRxTimeout(5000000000ll);
    if (ret)
    {
        GDOS_ERROR("Can't set receive timeout to 5 s, code = %d\n", ret);
        return ret;
    }

    ret = transRot();
    if (ret)
    {
        GDOS_ERROR("Can't set transRot, code = %d\n", ret);
        return ret;
    }

    ret = transMeasure();
    if (ret)
    {
        GDOS_ERROR("Can't set transMeasure, code = %d\n", ret);
        return ret;
    }

    ret = getProfile();
    if (ret)
    {
        GDOS_ERROR("Can't set getProfile, code = %d\n", ret);
        return ret;
    }

    // setting receive timeout
    ret = canPort.setRxTimeout(1000000000ll);
    if (ret)
    {
        GDOS_ERROR("Can't set receive timeout to 1 s, code = %d\n", ret);
        return ret;
    }

    return DataModule::moduleOn();   // have to be last command in moduleOn();
}

void LadarIbeo::moduleOff(void)
{
    int ret;
    DataModule::moduleOff();         // have to be first command in moduleOff();

    ret = cancelProfile();
    if (ret)
    {
        GDOS_ERROR("Can't set cancelProfile, code = %d\n", ret);
    }

    ret = transRot();
    if (ret)
    {
        GDOS_ERROR("Can't set transRot, code = %d\n", ret);
    }

    ret = transIdle();
    if (ret)
    {
        GDOS_ERROR("Can't set transIdle, code = %d\n", ret);
    }
}

int  LadarIbeo::moduleLoop(void)
{
    ladar_data      *p_data     = NULL;
    uint32_t        datalength = 0;
    int profileLen;
    int profileIdx;

    uint16_t format;
    uint8_t  layerNum;
    uint8_t  sectorNum;
    int sector;
    uint16_t dirstep;

    uint16_t tstart;
    uint16_t startdir;
    int distance;
    uint16_t tend;
    int distanceIndex;

    rack_time_t recordingtime;

    // get datapointer from rackdatabuffer
    // you don't need check this pointer
    p_data = (ladar_data *)getDataBufferWorkSpace();

    // read data profile
    if((profileLen = receiveResponsePackage(0x8301, MAX_PROFILE_SIZE, profile, &recordingtime)) < 4)
    {
        GDOS_ERROR("Can't receive data profile\n");
        return -1;
    }

    // decode profile head
    format         = __be16_to_cpu(*((uint16_t*)(profile + 0)));
    layerNum       = profile[2];
    sectorNum      = profile[3];
    profileIdx     = 4;

    GDOS_DBG_DETAIL("Data profile length %i format 0x%x layers %i sectors %i CAN-recordingtime %i\n",
                    profileLen, format, layerNum, sectorNum, recordingtime);
    // loop over all sectors of the profile
    for(sector = 0; sector < sectorNum; sector++)
    {
        dirstep = __be16_to_cpu(*((uint16_t*)(profile + profileIdx)));

        profileIdx += 2;

        p_data->angleResolution = (dirstep * M_PI/180.0f) / 16.0f;
        p_data->distanceNum     = __be16_to_cpu(*((uint16_t*)(profile + profileIdx)));

        profileIdx += 2;

        p_data->duration = 100;  // 100ms
        p_data->maxRange = 100000;  // 100m

        if(p_data->distanceNum > LADAR_DATA_MAX_DISTANCE_NUM)
        {
            GDOS_ERROR("Number of distanceNum is bigger than MAX_DISTANCE_NUM\n");
            return -1;
        }
        tstart = __be16_to_cpu(*((uint16_t*)(profile + profileIdx)));
        profileIdx += 2;

        startdir = __be16_to_cpu(*((uint16_t*)(profile + profileIdx)));
        profileIdx += 2;
        p_data->startAngle = (startdir * M_PI/180.0f) / 16.0f - M_PI;

        for(distanceIndex = 0; distanceIndex < p_data->distanceNum; distanceIndex++)
        {
            distance = __be16_to_cpu(*((uint16_t*)(profile + profileIdx)));  // measurement unit is m * 256
            p_data->distance[distanceIndex] = (int)((float)distance * 1000.0f / 256.0f) ;  // measurement unit is mm
            profileIdx += 2;
        }

        tend = __be16_to_cpu(*((uint16_t*)(profile + profileIdx)));
        profileIdx += 2;

        if(timeOffsetSector[sector] != 0)
        {
            p_data->recordingTime = recordingtime + timeOffsetSector[sector];
        }
        else
        {
            p_data->recordingTime = ((((rack_time_t)tstart) + ((rack_time_t)tend))/2) + timeOffset;
            timeOffsetSector[sector] = p_data->recordingTime - recordingtime;
        }

        if(p_data->distanceNum > 0)
        {
            datalength = sizeof(ladar_data) +
                    sizeof(int32_t) * p_data->distanceNum; // points
            putDataBufferWorkSpace(datalength);
            GDOS_DBG_DETAIL("Data sector %i distanceNum %i startAngle %a angleResolution %a tstart %i tend %i recordingtime  %i\n", sector,
                            p_data->distanceNum, p_data->startAngle, p_data->angleResolution, tstart, tend, p_data->recordingTime);
        }
    }
    if(decodeSensorStatus(*((unsigned int*)(profile + profileIdx))) == -1)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int  LadarIbeo::moduleCommand(MessageInfo *p_msginfo)
{
    // not for me -> ask DataModule
    return DataModule::moduleCommand(p_msginfo);
}

int LadarIbeo::sendRequestPackage(int requestCommand, int parameterLen, void* parameter)
{
    rtcan_frame_t canSend;
    uint16_package canPackage;
    int packageId;
    int parameterCount;
    int i, ret;

    GDOS_DBG_DETAIL("send sensor request %x parameterLen %i\n", requestCommand, parameterLen);

    canSend.can_dlc     = 8;
    canSend.can_id      = canHostIdBase | hostId;

    if(parameterLen <= 2)  // request frame fits into one CAN package
    {
        canSend.data[0] = 0;  // packageId = 0x0000
        canSend.data[1] = 0;
        canSend.data[2] = 0;
        canSend.data[3] = sensorId;

        canPackage.word[2]  =   __cpu_to_be16(requestCommand);

        canSend.data[4]     =   canPackage.word[2];
        canSend.data[5]     =   (canPackage.word[2] >> 8) & 0xff;

        for(i = 0; i < parameterLen; i++)
        {
            canSend.data[6 + i] = *((char*)parameter + i);
        }

        GDOS_DBG_DETAIL( "send CAN package 0=%x 1=%x 2=%x 3=%x 4=%x 5=%x 6=%x 7=%x\n", canSend.data[0], canSend.data[1], canSend.data[2], canSend.data[3], canSend.data[4], canSend.data[5], canSend.data[6], canSend.data[7]);

        ret = canPort.send(&canSend);
        if (ret)
        {
            GDOS_ERROR("Can't send can command, code = %d\n", ret);
            return ret;
        }
    }
    else  // request frame will be split into several CAN packages
    {
        packageId = (parameterLen + 9) / 6;

        canPackage.word[0]  = __cpu_to_be16(0xffff);
        canPackage.word[1]  = __cpu_to_be16(packageId);
        canPackage.word[2]  = __cpu_to_be16(sensorId);
        canPackage.word[3]  = __cpu_to_be16(requestCommand);

        canSend.data[0]     =   canPackage.word[0];
        canSend.data[1]     =   (canPackage.word[0] >> 8) & 0xff;
        canSend.data[2]     =   canPackage.word[1];
        canSend.data[3]     =   (canPackage.word[1] >> 8) & 0xff;
        canSend.data[4]     =   canPackage.word[2]; // __cpu_to_be16(sensorId);
        canSend.data[5]     =   (canPackage.word[2] >> 8) & 0xff;
        canSend.data[6]     =   canPackage.word[3];
        canSend.data[7]     =   (canPackage.word[3] >> 8) & 0xff;

        GDOS_DBG_DETAIL( "send first CAN package 0=%x 1=%x 2=%x 3=%x 4=%x 5=%x 6=%x 7=%x\n", canSend.data[0], canSend.data[1], canSend.data[2], canSend.data[3], canSend.data[4], canSend.data[5], canSend.data[6], canSend.data[7]);

        ret = canPort.send(&canSend);
        if (ret)
        {
            GDOS_ERROR("Can't send can command, code = %d\n", ret);
            return ret;
        }
        packageId--;
        parameterCount = 0;

        while(packageId > 0)
        {
            canPackage.word[0]  = __cpu_to_be16(packageId);

            canSend.data[0]     =   canPackage.word[0];
            canSend.data[1]     =   (canPackage.word[0] >> 8) & 0xff;

            for(i = 0; i < 6; i++)
            {
                canSend.data[2 + i] = *((char*)parameter + parameterCount);
                parameterCount++;
            }

            RackTask::sleep(5000000llu); // 1ms

            GDOS_DBG_DETAIL( "send CAN package 0=%x 1=%x 2=%x 3=%x 4=%x 5=%x 6=%x 7=%x\n", canSend.data[0], canSend.data[1], canSend.data[2], canSend.data[3], canSend.data[4], canSend.data[5], canSend.data[6], canSend.data[7]);

            ret = canPort.send(&canSend);
            if (ret)
            {
                GDOS_ERROR("Can't send can command, code = %d\n", ret);
                return ret;
            }
            packageId--;
        }
        return 0;
    }
    return 0;
}

int LadarIbeo::receiveResponsePackage(int responseCode, int maxParameterLen, void* parameter, rack_time_t* recordingtime)
{
    rtcan_frame_t canReceive;
    rack_time_t timestamp = 0;
    int parameterLen;
    int sequenceFlag;
    int packageId;
    int did;
    int code;
    int pid;
    int i, ret;
    int packageCounter = 0;

    do  // synchronize on first frame package with correct response code
    {
        packageCounter++;
        if (packageCounter > 100)
        {
            GDOS_WARNING("Can't synchronize on first frame package\n");
            return -1;
        }

        ret = canPort.recv(&canReceive, &timestamp);
        if (ret)
        {
            GDOS_ERROR("Can't read CAN data, code = %d\n", ret);
            return ret;
        }
        packageId = byteorder_read_be_u16(canReceive.data);
        if (packageId == 0xffff)  // receive 1st package of multi package frame
        {
            GDOS_DBG_DETAIL("Multi package frame ... \n");
            sequenceFlag = 0xffff;
            packageId    = byteorder_read_be_u16(&canReceive.data[2]);
            code         = byteorder_read_be_u16(&canReceive.data[6]);
            did          = canReceive.data[5];
            parameterLen = 0;
            if(recordingtime != NULL)
                *recordingtime = timestamp;
        }
        else  // receive single package frame
        {
            sequenceFlag = 0;
            code = byteorder_read_be_u16(&canReceive.data[4]);
            did  = canReceive.data[3];
            for(parameterLen = 0; parameterLen < 2; parameterLen++)
            {
                if(parameterLen >= maxParameterLen)
                    break;

                *((char*)parameter + parameterLen) = canReceive.data[6 + parameterLen];
            }
            if(recordingtime != NULL)
                *recordingtime = timestamp;
        }
    }
    while(((packageId != 0) & (sequenceFlag != 0xffff)) | (code != responseCode) | (did != hostId));

    if (sequenceFlag == 0xffff)  // receive multi package frame
    {
         for(pid = (packageId - 1); pid > 0; pid--)  // receive all expected following packages
        {
            ret = canPort.recv(&canReceive, &timestamp);
            if (ret)
            {
                GDOS_ERROR("Can't receive following CAN package, code = %d\n", ret);
                return ret;
            }
            packageId = __be16_to_cpu(*((uint16_t*)(&canReceive.data[0])));

            if(packageId != pid)  // check if this package is expected
            {
                GDOS_WARNING("CAN package lost\n");
                return -1;
            }

            for(i = 0; i < 6; i++)
            {
                *((char*)parameter + parameterLen) = canReceive.data[2 + i];

                parameterLen++;
                if(parameterLen >= maxParameterLen)
                {
                    GDOS_DBG_DETAIL("received sensor response %x parameterLen %i\n", code, parameterLen);
                    return(parameterLen);
                }
            }
        }
        return(parameterLen);
    }
    else
    {
        return(parameterLen);
    }
}

int LadarIbeo::decodeSensorStatus(unsigned int senstat)
{
    uint32_t workingMode       = (senstat & 0x0f000000) >> 24;
    uint32_t motorMode         = (senstat & 0xf0000000) >> 28;
    uint32_t measureMode       = (senstat & 0x00ff0000) >> 16;
    uint32_t systemMode        = (senstat & 0x0000ff00) >> 8;
    uint32_t communicationMode = (senstat & 0x000000ff);

    switch(motorMode)
    {
    case 0x9:
        GDOS_WARNING("Motor spin to high\n");
        break;
    case 0xa:
        GDOS_WARNING("Motor spin to low\n");
        break;
    case 0xb:
        GDOS_WARNING("Motor stops on coder error\n");
        break;
    }

    if(measureMode & 0x1)
    {
        GDOS_WARNING("External reference target not found\n");
    }

    if(systemMode & 0x0)
    {
        GDOS_WARNING("Signatur Failure Program Code (Flash)\n");
    }
    if(systemMode & 0x1)
    {
        GDOS_WARNING("Signatur Failure Configuration Table (Flash)\n");
    }
    if(systemMode & 0x2)
    {
        GDOS_WARNING("Signatur Failure User (Flash)\n");
    }
    if(systemMode & 0x4)
    {
        GDOS_WARNING("Failure in SRAM\n");
    }
    if(systemMode & 0x8)
    {
        GDOS_WARNING("Communication failure with Application DSP\n");
    }
    if(systemMode & 0xa)
    {
        GDOS_WARNING("Internal temperature is higher than 65�C\n");
    }

    if(communicationMode & 0x0)
    {
        GDOS_WARNING("Sensor was busoff (CAN)\n");
    }
    if(communicationMode & 0x1)
    {
        GDOS_WARNING("Package lost (CAN)\n");
    }
    if(communicationMode & 0x2)
    {
        GDOS_WARNING("Incorrect header (RS232/RS422)\n");
    }
    if(communicationMode & 0x4)
    {
        GDOS_WARNING("Incorrect data (RS232/RS422)\n");
    }
    if(workingMode == 0x4)
    {
        GDOS_ERROR("Sensor in ERROR_MODE");
        return -1;
    }
    else
    {
        return 0;
    }
}

int LadarIbeo::getSensorStatus(void)
{
    status_response response;

    if(sendRequestPackage(0x0102, 0, NULL) != 0)
    {
        GDOS_ERROR("Can't send getStatus request to sensor\n");
        return -1;
    }
    GDOS_DBG_DETAIL("receiveResponsePackage ... \n");

    if(receiveResponsePackage(0x8102, sizeof(status_response), &response, NULL) != sizeof(status_response))
    {
        GDOS_ERROR("Can't receive response on getStatus request\n");
        return -1;
    }

    return(decodeSensorStatus(response.senstat));
}

int LadarIbeo::transIdle(void)
{
    status_response response;

    GDOS_DBG_INFO("transIdle\n");

    if(sendRequestPackage(0x0402, 0, NULL) != 0)
    {
        GDOS_ERROR("Can't send transIdle request to sensor\n");
        return -1;
    }

    if(receiveResponsePackage(0x8402, sizeof(status_response), &response, NULL) != sizeof(status_response))
    {
        GDOS_ERROR("Can't receive response on transIdle request\n");
        return -1;
    }

    return(decodeSensorStatus(response.senstat));
}

int LadarIbeo::transRot(void)
{
    trans_rot_request request;
    status_response response;

    GDOS_DBG_INFO("transRot\n");

    request.rev = 0;  // Scanning frequency corresponds to the configuration parameter
    if(sendRequestPackage(0x0403, sizeof(trans_rot_request), &request) != 0)
    {
        GDOS_ERROR("Can't send transRot request to sensor\n");
        return -1;
    }

    if(receiveResponsePackage(0x8403, sizeof(status_response), &response, NULL) != sizeof(status_response))
    {
        GDOS_ERROR("Can't receive response on transRot request\n");
        return -1;
    }

    return(decodeSensorStatus(response.senstat));
}

int LadarIbeo::transMeasure(void)
{
    trans_measure_response response;

    GDOS_DBG_INFO("transMeasure\n");

    if(sendRequestPackage(0x0404, 0, NULL) != 0)
    {
        GDOS_ERROR("Can't send transMeasure request to sensor\n");
        return -1;
    }

    if(receiveResponsePackage(0x8404, sizeof(trans_measure_response), &response, NULL) != sizeof(trans_measure_response))
    {
        GDOS_ERROR("Can't receive response on transMeasure request\n");
        return -1;
    }

    switch(response.errorcode >> 8)
    {
    case 1:
        GDOS_ERROR("Maximum laser pulse frequency too high\n");
        return -1;
    case 2:
        GDOS_ERROR("Mean laser pulse frequency too high\n");
        return -1;
    case 3:
        GDOS_ERROR("The sector borders are not configured correctly\n");
        return -1;
    case 4:
        GDOS_ERROR("A sector border is not a whole multiple of the angle step\n");
        return -1;
    }

// ******************** should be replaced by a better solution *****************************************
   RackTask::sleep(5000000000llu); // 5s
// ******************************************************************************************************

    return(decodeSensorStatus(response.senstat));
}

int LadarIbeo::getProfile(void)
{
    get_profile_request request;
    int profilenum    = 0;  // request continuous data
    int profileformat = PROFILEFORMAT_DIRSTEP | PROFILEFORMAT_POINTNUM | PROFILEFORMAT_TSTART | PROFILEFORMAT_STARTDIR |
                        PROFILEFORMAT_DISTANCE |
                        PROFILEFORMAT_TEND | PROFILEFORMAT_SENSTAT;

    GDOS_DBG_INFO("getProfile\n");

    request.profilenum      = __cpu_to_be16(profilenum);
    request.profileformat   = __cpu_to_be16(profileformat);

    if(sendRequestPackage(0x0301, sizeof(get_profile_request), &request) != 0)
    {
        GDOS_ERROR("Can't send getProfile request to sensor\n");
        return -1;
    }
    return 0;
}

int LadarIbeo::cancelProfile(void)
{
    status_response response;

    if(sendRequestPackage(0x0302, 0, NULL) != 0)
    {
        GDOS_ERROR("Can't send cancleProfile request to sensor\n");
        return -1;
    }

    if(receiveResponsePackage(0x8302, sizeof(status_response), &response, NULL) != sizeof(status_response))
    {
        GDOS_ERROR("Can't receive response on cancelProfile request\n");
        return -1;
    }

    return(decodeSensorStatus(response.senstat));
}

int LadarIbeo::setTimeAbs(void)
{
    set_time_abs send;
    set_time_abs_responce response;
    rack_time_t recordingtime;

    send.syncabs = 0;

    if(sendRequestPackage(0x0203, sizeof(set_time_abs), &send) != 0)
    {
        GDOS_ERROR("Can't send setTimeAbs request to sensor\n");
        return -1;
    }

    if(receiveResponsePackage(0x8203, sizeof(set_time_abs_responce), &response, &recordingtime) != sizeof(set_time_abs_responce))
    {
        GDOS_ERROR("Can't receive response on setTimeAbs request\n");
        return -1;
    }

    timeOffset = recordingtime - (rack_time_t)response.synctime;

    GDOS_DBG_DETAIL("getSyncTime synctime %i recordingtime %i timeOffset %i\n", (int)response.synctime, recordingtime, timeOffset);

    return 0;
}

int LadarIbeo::byteorder_read_be_u16(void* u16)
{
    return((int)((*((unsigned char*)u16) << 8) | *((unsigned char*)u16 + 1)));
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

int LadarIbeo::moduleInit(void)
{
    int ret;

    GDOS_PRINT("Ladar Ibeo version '01 canDev=%i sensorId=%i\n", canDev, sensorId);

    // call DataModule init function (first command in init)
    ret = DataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    ladar_sockaddr_can cscan =
     {
        scan   :
        {
            can_family   : AF_CAN, can_ifindex  : 0,
            can_flistlen : 1
        },
        filter :
        {
            {(canSensorIdBase | sensorId), 0xffff}
        },
    };

    // Open can port
    ret = canPort.open(canDev, (sockaddr_can *)&cscan, sizeof(cscan), this);
    if (ret)
    {
        GDOS_ERROR("Can't open can port, code = %d\n", ret);
        goto init_error;
    }
    initBits.setBit(INIT_BIT_CAN_OPEN);
    GDOS_DBG_INFO("Can port has been bound\n");

    // setting receive timeout
    ret = canPort.setRxTimeout(1000000000ll);
    if (ret)
    {
        GDOS_WARNING("Can't set receive timeout to 1 s, code = %d\n", ret);
        goto init_error;
    }

    // setting send timeout
    ret = canPort.setTxTimeout(100000000ll);
    if (ret)
    {
        GDOS_WARNING("Can't set send timeout to 100 ms, code = %d\n", ret);
        goto init_error;
    }

    // take timestamps
    ret = canPort.getTimestamps();
    if (ret)
    {
        GDOS_ERROR("Can't enable timestamp mode, code = %d\n", ret);
        goto init_error;
    }
    GDOS_DBG_INFO("Timestamp mode has been enabled\n");

    return 0;

init_error:
    // !!! call local cleanup function !!!
    LadarIbeo::moduleCleanup();
    return ret;
}

// non realtime context
void LadarIbeo::moduleCleanup(void)
{
    int ret;

    GDOS_DBG_DETAIL("LadarIbeo::moduleCleanup ... \n");

    // close rtcan port
    if (initBits.testAndClearBit(INIT_BIT_CAN_OPEN))
    {
        ret = canPort.close();
        if (!ret)
            GDOS_DBG_DETAIL("CAN port closed \n");
        else
            GDOS_ERROR("Can't close CAN port \n");
    }
    // call DataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        DataModule::moduleCleanup();
    }
}

LadarIbeo::LadarIbeo()
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
    // get value(s) out of your argument table
    canDev  = getIntArg("canDev", argTab);
    sensorId  = getIntArg("sensorId", argTab);

    canHostIdBase   = 0x2a8;
    canSensorIdBase = 0x700;
    hostId = 1;

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(ladar_data_msg));

    // set databuffer period time
    setDataBufferPeriodTime(100); // 100 ms (10 per sec)
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = Module::getArgs(argc, argv, argTab, "LadarIbeo");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new NewDataModule
    p_inst = new LadarIbeo();
    if (!p_inst)
    {
        printf("Can't create new LadarIbeo -> EXIT\n");
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
