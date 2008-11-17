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
#define INIT_BIT_MBX_WORK                   1
#define INIT_BIT_PROXY_OBJ_RECOG_BOUND      2
#define INIT_BIT_PROXY_OBJ_RECOG_CONTOUR    3


//
// data structures
//

LadarIbeoLux *p_inst;

argTable_t argTab[] = {

    { ARGOPT_OPT, "objRecogBoundInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the object recognition relay for bounding-boxes, default -1", { -1 } },

    { ARGOPT_OPT, "objRecogContourInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the object recognition relay for contours, default -1", { -1 } },

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

    // object recognition bounding-box
    if (objRecogBoundInst >= 0)
    {
        // init objRecogBound values and send first dataMsg
        objRecogBoundData.data.recordingTime = rackTime.get();
        objRecogBoundData.data.objectNum      = 0;

        ret = workMbx.sendDataMsg(MSG_DATA, objRecogBoundMbxAdr + 1, 1, 1,
                                 &objRecogBoundData, sizeof(obj_recog_data));
        if (ret)
        {
            GDOS_WARNING("Error while sending first objRecogBound data from %x to %x (bytes %d)\n",
                         workMbx.getAdr(), objRecogBoundMbxAdr, sizeof(obj_recog_data));
        }

        GDOS_DBG_DETAIL("Turn on ObjRecog(%d)\n", objRecogBoundInst);
        ret = objRecogBound->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on ObjRecogBound(%i), code = %d\n", objRecogBoundInst, ret);
            return ret;
        }
    }

    // object recognition contour
    if (objRecogContourInst >= 0)
    {
        // init objRecogContour values and send first dataMsg
        objRecogContourData.data.recordingTime = rackTime.get();
        objRecogContourData.data.objectNum      = 0;

        ret = workMbx.sendDataMsg(MSG_DATA, objRecogContourMbxAdr + 1, 1, 1,
                                 &objRecogContourData, sizeof(obj_recog_data));
        if (ret)
        {
            GDOS_WARNING("Error while sending first objRecogContour data from %x to %x (bytes %d)\n",
                         workMbx.getAdr(), objRecogContourMbxAdr, sizeof(obj_recog_data));
        }

        GDOS_DBG_DETAIL("Turn on ObjRecog(%d)\n", objRecogContourInst);
        ret = objRecogContour->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on ObjRecogBound(%i), code = %d\n", objRecogContourInst, ret);
            return ret;
        }
    }

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
    fracFactor        = pow(2, -32);
    dataRate          = 0;
    dataRateCounter   = 0;
    dataRateStartTime = rackTime.get();

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
    int                         ret, i, j, l;
    int                         dx, dy, length;
    int                         dT;
    uint32_t                    datalength         = 0;
    float                       angle;
    double                      scanStartTime;
    double                      scanEndTime;
    double                      ladarSendTime;
    rack_time_t                 recordingTime;
    ladar_data                  *p_data            = NULL;
    ladar_ibeo_lux_scan_data    *ladarScanData     = NULL;
    ladar_ibeo_lux_obj_data     *ladarObjData      = NULL;
    ladar_ibeo_lux_obj          *ladarObj          = NULL;
    ladar_ibeo_lux_point2d      *ladarContourPoint = NULL;
    ladar_ibeo_lux_point2d      ladarContourPointOld;
    point_2d                    startPoint, endPoint;

    // get datapointer from rackdatabuffer
    p_data = (ladar_data *)getDataBufferWorkSpace();

    RackTask::disableRealtimeMode();

    // read ladar header
    ret = recvLadarHeader(&ladarHeader, &recordingTime, 1000);
    if (ret)
    {
        return ret;
    }

    // calc current data rate
    dataRateCounter += ladarHeader.messageSize + 24;
    dT               = (int)recordingTime - (int)dataRateStartTime;
    if (dT > 1000)
    {
        dataRate          = dataRateCounter * 1000 / dT;
        dataRateStartTime = recordingTime;
        dataRateCounter   = 0;
    }

    GDOS_DBG_DETAIL("time %d, ladar dataType %x, messageSize %d, dataRate %d bytes/s\n",
                    recordingTime, ladarHeader.dataType, ladarHeader.messageSize, dataRate);

    // read ladar data body
    ret = recvLadarData(&ladarData, ladarHeader.messageSize);

   RackTask::enableRealtimeMode();

    // parse ladar data body
    switch (ladarHeader.dataType)
    {
        // scan data
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
            p_data->recordingTime   = recordingTime -
                                        (int)rint((ladarSendTime - scanEndTime +
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
                p_data->point[i].distance = ladarScanData->point[i].distance * 10;

                // ladar type
                switch (ladarScanData->point[i].flags)
                {
                    case 0:
                        p_data->point[i].type = LADAR_POINT_TYPE_UNKNOWN;           // unknown
                        break;
                    case 1:
                        p_data->point[i].type = LADAR_POINT_TYPE_TRANSPARENT;       // transparent
                        break;
                    case 2:
                        p_data->point[i].type = LADAR_POINT_TYPE_RAIN;              // rain
                        break;
                    case 4:
                        p_data->point[i].type = LADAR_POINT_TYPE_INVALID;           // ground
                        break;
                    case 8:
                        p_data->point[i].type = LADAR_POINT_TYPE_DIRT;              // dirt
                        break;
                    default:
                        p_data->point[i].type = LADAR_POINT_TYPE_INVALID;
                        break;
                }
            }
            GDOS_DBG_DETAIL("Data recordingtime %i pointNum %i\n",
                            p_data->recordingTime, p_data->pointNum);
            datalength = sizeof(ladar_data) + sizeof(ladar_point) * p_data->pointNum;
            putDataBufferWorkSpace(datalength);
            break;

        // object data
        case LADAR_IBEO_LUX_OBJ_DATA:
            ladarObjData = (ladar_ibeo_lux_obj_data *)&ladarData;
            l            = sizeof(ladar_ibeo_lux_obj_data);

            // calculate ladar timestamps (unit seconds)
            scanStartTime = (double)ladarObjData->timestamp.seconds +
                                    fracFactor * (double)ladarObjData->timestamp.secondsFrac;
            ladarSendTime = (double)ladarHeader.ntpTime.seconds +
                                    fracFactor * (double)ladarHeader.ntpTime.secondsFrac;


            // bounding-box data
            // recalculate recordingtime to the beginning of the scan
            objRecogBoundData.data.recordingTime = recordingTime -
                                                   (int)rint((ladarSendTime - scanStartTime) * 1000);
            objRecogBoundData.data.refPos.x      = 0;
            objRecogBoundData.data.refPos.y      = 0;
            objRecogBoundData.data.refPos.z      = 0;
            objRecogBoundData.data.refPos.phi    = 0.0f;
            objRecogBoundData.data.refPos.psi    = 0.0f;
            objRecogBoundData.data.refPos.rho    = 0.0f;
            objRecogBoundData.data.objectNum     = 0;

            // contour data
            // recalculate recordingtime to the beginning of the scan
            objRecogContourData.data.recordingTime = recordingTime -
                                                     (int)rint((ladarSendTime - scanStartTime) * 1000);
            objRecogContourData.data.refPos.x      = 0;
            objRecogContourData.data.refPos.y      = 0;
            objRecogContourData.data.refPos.z      = 0;
            objRecogContourData.data.refPos.phi    = 0.0f;
            objRecogContourData.data.refPos.psi    = 0.0f;
            objRecogContourData.data.refPos.rho    = 0.0f;
            objRecogContourData.data.objectNum     = 0;


            // loop over all objects
            for (i = 0; i < ladarObjData->objNum; i++)
            {
                ladarObj = (ladar_ibeo_lux_obj *)&ladarData[l];
                l       += sizeof(ladar_ibeo_lux_obj);

                // bounding-box data
                if (objRecogBoundData.data.objectNum < OBJ_RECOG_OBJECT_MAX)
                {
                    objRecogBoundData.data.objectNum++;
                    objRecogBoundData.object[i].objectId =  ladarObj->id;
                    objRecogBoundData.object[i].pos.x    =  ladarObj->objBoxCenter.x * 10;
                    objRecogBoundData.object[i].pos.y    = -ladarObj->objBoxCenter.y * 10;
                    objRecogBoundData.object[i].pos.z    =  0;
                    objRecogBoundData.object[i].pos.phi  =  0.0f;
                    objRecogBoundData.object[i].pos.psi  =  0.0f;
                    objRecogBoundData.object[i].pos.rho  = -(float)ladarObj->objBoxOrientation *
                                                                   M_PI / (180.0 * 32.0);
                    objRecogBoundData.object[i].vel.x    =  ladarObj->velRel.x * 10;
                    objRecogBoundData.object[i].vel.y    = -ladarObj->velRel.y * 10;
                    objRecogBoundData.object[i].vel.z    =  0;
                    objRecogBoundData.object[i].vel.phi  =  0.0f;
                    objRecogBoundData.object[i].vel.psi  =  0.0f;
                    objRecogBoundData.object[i].vel.rho  =  0.0f;
                    objRecogBoundData.object[i].dim.x    =  ladarObj->objBoxSize.x * 10;
                    objRecogBoundData.object[i].dim.y    =  ladarObj->objBoxSize.y * 10;
                    objRecogBoundData.object[i].dim.z    =  0;
                    objRecogBoundData.object[i].prob     =  0.0f;
                    objRecogBoundData.object[i].imageArea.x      = 0;
                    objRecogBoundData.object[i].imageArea.y      = 0;
                    objRecogBoundData.object[i].imageArea.width  = 0;
                    objRecogBoundData.object[i].imageArea.height = 0;
                }


                // contour data
                for (j = 0; j < ladarObj->contourPointNum; j++)
                {
                    ladarContourPoint = (ladar_ibeo_lux_point2d *)&ladarData[l];
                    l                += sizeof(ladar_ibeo_lux_point2d);

                    if (j >= 1)
                    {
                        if (objRecogContourData.data.objectNum < OBJ_RECOG_OBJECT_MAX)
                        {
                            startPoint.x =  ladarContourPointOld.x * 10;
                            startPoint.y = -ladarContourPointOld.y * 10;
                            endPoint.x   =  ladarContourPoint->x * 10;
                            endPoint.y   = -ladarContourPoint->y * 10;

                            dx     = endPoint.x - startPoint.x;
                            dy     = endPoint.y - startPoint.y;
                            angle  = atan2((double)dy, (double)dx);
                            length = (int)sqrt((double)dx * (double)dx + (double)dy * (double)dy);

                            objRecogContourData.data.objectNum++;
                            objRecogContourData.object[j].objectId =  ladarObj->id;
                            objRecogContourData.object[j].pos.x    =  (startPoint.x + endPoint.x) / 2;
                            objRecogContourData.object[j].pos.y    =  (startPoint.y + endPoint.y) / 2;
                            objRecogContourData.object[j].pos.z    =  0;
                            objRecogContourData.object[j].pos.phi  =  0.0f;
                            objRecogContourData.object[j].pos.psi  =  0.0f;
                            objRecogContourData.object[j].pos.rho  =  angle;
                            objRecogContourData.object[j].vel.x    =  ladarObj->velRel.x * 10;
                            objRecogContourData.object[j].vel.y    = -ladarObj->velRel.y * 10;
                            objRecogContourData.object[j].vel.z    =  0;
                            objRecogContourData.object[j].vel.phi  =  0.0f;
                            objRecogContourData.object[j].vel.psi  =  0.0f;
                            objRecogContourData.object[j].vel.rho  =  0.0f;
                            objRecogContourData.object[j].dim.x    =  length;
                            objRecogContourData.object[j].dim.y    =  0;
                            objRecogContourData.object[j].dim.z    =  0;
                            objRecogContourData.object[j].prob     =  0.0f;
                            objRecogContourData.object[j].imageArea.x      = 0;
                            objRecogContourData.object[j].imageArea.y      = 0;
                            objRecogContourData.object[j].imageArea.width  = 0;
                            objRecogContourData.object[j].imageArea.height = 0;
                        }
                    }

                    memcpy(&ladarContourPointOld, ladarContourPoint, sizeof(ladar_ibeo_lux_point2d));
                }
            }

            // send bounding-box data
            if (objRecogBoundInst >= 0)
            {
                GDOS_DBG_DETAIL("ObjRecogBoundData recordingtime %i objectNum %i\n",
                                objRecogBoundData.data.recordingTime,
                                objRecogBoundData.data.objectNum);
                datalength =  sizeof(obj_recog_data) +
                              objRecogBoundData.data.objectNum * sizeof(obj_recog_object);

                ret = workMbx.sendDataMsg(MSG_DATA, objRecogBoundMbxAdr + 1, 1, 1,
                                         &objRecogBoundData, datalength);
                if (ret)
                {
                    GDOS_ERROR("Error while sending objRecogBound data from %x to %x (bytes %d)\n",
                               workMbx.getAdr(), objRecogBoundMbxAdr, datalength);
                    return ret;
                }
            }

            // send contour data
            if (objRecogContourInst >= 0)
            {
                GDOS_DBG_DETAIL("ObjRecogContourData recordingtime %i objectNum %i\n",
                                objRecogContourData.data.recordingTime,
                                objRecogContourData.data.objectNum);
                datalength =  sizeof(obj_recog_data) +
                              objRecogContourData.data.objectNum * sizeof(obj_recog_object);

                ret = workMbx.sendDataMsg(MSG_DATA, objRecogContourMbxAdr + 1, 1, 1,
                                         &objRecogContourData, datalength);
                if (ret)
                {
                    GDOS_ERROR("Error while sending objRecogContour data from %x to %x (bytes %d)\n",
                               workMbx.getAdr(), objRecogContourMbxAdr, datalength);
                    return ret;
                }
            }
            break;
    }

    return 0;
}

int  LadarIbeoLux::moduleCommand(message_info *p_msginfo)
{
    // not for me -> ask RackDataModule
    return RackDataModule::moduleCommand(p_msginfo);
}


int LadarIbeoLux::recvLadarHeader(ladar_ibeo_lux_header *data, rack_time_t *recordingTime,
                                  unsigned int retryNumMax)
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
            *recordingTime = rackTime.get();
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

    // create mailbox
    ret = createMbx(&workMbx, 10, sizeof(obj_recog_data_msg),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // create objRecogBound proxy
    if (objRecogBoundInst >= 0)
    {
        objRecogBound       = new ObjRecogProxy(&workMbx, 0, objRecogBoundInst);
        if (!objRecogBound)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_OBJ_RECOG_BOUND);
    }

    // create objRecogContour proxy
    if (objRecogContourInst >= 0)
    {
        objRecogContourMbxAdr = RackName::create(OBJ_RECOG, objRecogContourInst);
        objRecogContour       = new ObjRecogProxy(&workMbx, 0, objRecogContourInst);
        if (!objRecogContour)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_OBJ_RECOG_CONTOUR);
    }

    return 0;

init_error:
    LadarIbeoLux::moduleCleanup();
    return ret;
}

// non realtime context
void LadarIbeoLux::moduleCleanup(void)
{
    // call RackDataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // destroy objRecogContour proxy
    if (objRecogContourInst >= 0)
    {
        if (initBits.testAndClearBit(INIT_BIT_PROXY_OBJ_RECOG_CONTOUR))
        {
            delete objRecogContour;
        }
    }

    // destroy objRecogBound proxy
    if (objRecogBoundInst >= 0)
    {
        if (initBits.testAndClearBit(INIT_BIT_PROXY_OBJ_RECOG_BOUND))
        {
            delete objRecogBound;
        }
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
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
    // get static module parameter
    objRecogBoundInst   = getIntArg("objRecogBoundInst", argTab);
    objRecogContourInst = getIntArg("objRecogContourInst", argTab);

    objRecogBoundMbxAdr = RackName::create(OBJ_RECOG, objRecogBoundInst);

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
