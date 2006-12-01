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
 *      Oliver Wulf        <oliver.wulf@gmx.de>
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 */
 #include "datalog_rec_class.h"

#include <main/argopts.h>

// init_flags
#define INIT_BIT_CONT_DATA_BUFFER   0
#define INIT_BIT_DATA_MODULE        1
#define INIT_BIT_MBX_WORK           2
#define INIT_BIT_MBX_CONT_DATA      3
#define INIT_BIT_MTX_CREATED        4

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
int  DatalogRec::moduleOn(void)
{
    int         i, ret;
    int         init = 0;
    int         logEnable;
    rack_time_t periodTime;
    rack_time_t realPeriodTime;
    rack_time_t datalogPeriodTime = 1000;
    int         moduleMbx;

    GDOS_DBG_INFO("Turn on...\n");

    RackTask::disableRealtimeMode();

    if (datalogInfoMsg.data.logNum == 0)
    {
        GDOS_ERROR("Can't turn on, no modules to log\n");
        return -EINVAL;
    }

    for (i = 0; i < datalogInfoMsg.data.logNum; i++)
    {
        fileptr[i] = NULL;
    }

    contDataMbx.clean();

    for (i = 0; i < datalogInfoMsg.data.logNum; i++)
    {
        logEnable  = datalogInfoMsg.logInfo[i].logEnable;
        periodTime = datalogInfoMsg.logInfo[i].periodTime;
        moduleMbx  = datalogInfoMsg.logInfo[i].moduleMbx;

        datalogInfoMsg.logInfo[i].bytesLogged = 0;
        datalogInfoMsg.logInfo[i].setsLogged  = 0;

        if (logEnable > 0)
        {
            // open log file
            if ((fileptr[i] = fopen((char *)datalogInfoMsg.logInfo[i].filename, "w")) == NULL)
            {
                GDOS_ERROR("Can't open file %n...\n", moduleMbx);
                return -EIO;
            }

            // turn on module
            ret = moduleOn(moduleMbx, &workMbx, 5000000000ll);

            // request continuous data
            ret = getContData(moduleMbx, periodTime, &contDataMbx, &workMbx,
                              &realPeriodTime, 1000000000ll);
            if (ret)
            {
                GDOS_ERROR("Can't get continuous data from module %n\n", moduleMbx);
                return ret;
            }
            else
            {
                GDOS_DBG_INFO("%n: -log data\n", moduleMbx);
            }

            // set shortest period time
            if (init == 0)
            {
                init = 1;
                datalogPeriodTime = realPeriodTime;
            }

            if (realPeriodTime < datalogPeriodTime)
            {
                datalogPeriodTime = realPeriodTime;
            }
        }
    }

    setDataBufferPeriodTime(datalogPeriodTime);

    ret = initLogFile();
    if (ret < 0)
    {
        GDOS_ERROR("Can't init log files, code= %i\n", ret);
        return ret;
    }

    return RackDataModule::moduleOn();  // have to be last command in moduleOn();
}

void DatalogRec::moduleOff(void)
{
    int         i;
    int         logEnable;
    rack_time_t periodTime;
    int         moduleMbx;

    RackDataModule::moduleOff();        // have to be first command in moduleOff();

    GDOS_DBG_INFO("Turn off...\n");

    for (i = 0; i < datalogInfoMsg.data.logNum; i++)
    {
        logEnable   = datalogInfoMsg.logInfo[i].logEnable;
        periodTime  = datalogInfoMsg.logInfo[i].periodTime;
        moduleMbx   = datalogInfoMsg.logInfo[i].moduleMbx;

        if (logEnable > 0)
        {
            stopContData(moduleMbx, &contDataMbx, &workMbx, 1000000000ll);

            if (fileptr[i] != NULL)
            {
                fclose(fileptr[i]);
            }
        }
    }

    RackTask::enableRealtimeMode();
}

int  DatalogRec::moduleLoop(void)
{
    int             i, ret;
    message_info    msgInfo;
    datalog_data    *pDatalogData = NULL;

    // get continuous data
    ret = contDataMbx.recvDataMsgTimed(4000000000llu, contDataPtr,
                                       DATALOG_MSG_SIZE_MAX, &msgInfo);

    if (msgInfo.type != MSG_DATA)
    {
        GDOS_ERROR("No data package from %i type %i\n", msgInfo.src, msgInfo.type);
        return -EINVAL;
    }

    datalogMtx.lock(RACK_INFINITE);

    // get datapointer from rackdatabuffer
    pDatalogData = (datalog_data *)getDataBufferWorkSpace();

    // log data
    ret = logData(&msgInfo);
    if (ret)
    {
        datalogMtx.unlock();
        GDOS_ERROR("Error while logging data, code= %i\n", ret);
        return ret;
    }

    // write new data package
    pDatalogData->recordingTime = rackTime.get();
    pDatalogData->logNum        = 0;

    for (i = 0; i < datalogInfoMsg.data.logNum; i++)
    {
        memcpy(&pDatalogData->logInfo[i], &datalogInfoMsg.logInfo[i],
               sizeof(datalogInfoMsg.logInfo[i]));
        (pDatalogData->logNum)++;
    }

    putDataBufferWorkSpace(sizeof(datalog_data)+
                           pDatalogData->logNum * sizeof(datalog_logInfo));
    datalogMtx.unlock();

    return 0;
}

int  DatalogRec::moduleCommand(message_info *msgInfo)
{
    datalog_data *setLogData;

    switch (msgInfo->type)
    {
        case MSG_DATALOG_GET_LOG_STATUS:
            if (initLog == 1)
            {
                logInfoAllModules(&datalogInfoMsg.data);
                datalogInfoMsg.data.logNum = logInfoCurrentModules(datalogInfoMsg.logInfo,
                                                 datalogInfoMsg.data.logNum, datalogInfoMsg.logInfo,
                                                 &workMbx, 1000000000ll);
                initLog = 0;
            }

            cmdMbx.sendDataMsgReply(MSG_DATALOG_LOG_STATUS, msgInfo, 1, &datalogInfoMsg,
                                    sizeof(datalogInfoMsg));
            break;

        case MSG_DATALOG_SET_LOG:
            setLogData = DatalogData::parse(msgInfo);

            if (datalogInfoMsg.data.logNum == setLogData->logNum)
            {
                memcpy(&datalogInfoMsg.data, setLogData, sizeof(datalogInfoMsg));
                cmdMbx.sendMsgReply(MSG_OK, msgInfo);
            }
            else
            {
                cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
            }
            break;

        default:
            // not for me -> ask RackDataModule
            return RackDataModule::moduleCommand(msgInfo);
    }
    return 0;
}

int DatalogRec::initLogFile()
{
    int i, ret = 0;

    for (i = 0; i < datalogInfoMsg.data.logNum; i++)
    {
        if (datalogInfoMsg.logInfo[i].logEnable)
        {
            switch (RackName::classId(datalogInfoMsg.logInfo[i].moduleMbx))
            {
                case CAMERA:
                    ret = fprintf(fileptr[i], "%% Camera(%i)\n"
                                  "%% recordingTime width height depth mode"
                                  " colorFilterId cameraFileNum\n",
                                  RackName::instanceId(datalogInfoMsg.logInfo[i].moduleMbx));
                    break;

                case CHASSIS:
                    ret = fprintf(fileptr[i], "%% Chassis(%i)\n"
                                  "%% recordingTime deltaX deltaY deltaRho"
                                  " vx vy omega battery activePilot\n",
                                  RackName::instanceId(datalogInfoMsg.logInfo[i].moduleMbx));
                    break;

                case GPS:
                    ret = fprintf(fileptr[i], "%% Gps(%i)\n"
                                  "%% recordingTime mode latitude longitude"
                                  "altitude heading speed satelliteNum utcTime pdop"
                                  "posGK.x posGK.y posGK.z posGK.phi posGK.psi posGK.rho\n",
                                  RackName::instanceId(datalogInfoMsg.logInfo[i].moduleMbx));
                    break;

                case LADAR:
                    ret = fprintf(fileptr[i], "%% Ladar(%i)\n"
                                  "%% recordingTime duration maxRange"
                                  " startAngle angleResolution distanceNum distance[0]\n",
                                  RackName::instanceId(datalogInfoMsg.logInfo[i].moduleMbx));
                    break;

                case ODOMETRY:
                    ret = fprintf(fileptr[i], "%% Odometry(%i)\n"
                                  "%% recordingTime pos.x pos.y pos.z"
                                  " pos.phi pos.psi pos.rho\n",
                                  RackName::instanceId(datalogInfoMsg.logInfo[i].moduleMbx));
                    break;

                case PILOT:
                    ret = fprintf(fileptr[i], "%% Pilot(%i)\n"
                                  "%% recordingTime pos.x pos.y pos.z"
                                  " pos.phi pos.psi pos.rho speed curve splineNum"
                                  " spline[0].startPos.x spline[0].startPos.y"
                                  " spline[0].startPos.rho spline[0].endPos.x"
                                  " spline[0].endPos.y spline[0].endPos.rho"
                                  " spline[0].centerPos.x spline[0].centerPos.y"
                                  " spline[0].centerPos.rho spline[0].length"
                                  " spline[0].radius spline[0].vMax spline[0].vStart"
                                  " spline[0].vEnd spline[0].aMax spline[0].lbo\n",
                                  RackName::instanceId(datalogInfoMsg.logInfo[i].moduleMbx));
                    break;

                case POSITION:
                    ret = fprintf(fileptr[i], "%% Position(%i)\n"
                                  "%% recordingTime pos.x pos.y pos.z"
                                  " pos.phi pos.psi pos.rho\n",
                                  RackName::instanceId(datalogInfoMsg.logInfo[i].moduleMbx));
                    break;

                case SCAN2D:
                    ret = fprintf(fileptr[i], "%% Scan2d(%i)\n"
                                  "%% recordingTime duration maxRange"
                                  " pointNum scan2dFileNum\n",
                                  RackName::instanceId(datalogInfoMsg.logInfo[i].moduleMbx));
                    break;
            }
        }
    }

    return ret;
}

int DatalogRec::logData(message_info *msgInfo)
{
    int             i, j;
    int             bytes;
    int             bytesMax;
    char            extFilenameBuf[40];
    char            fileNumBuf[20];
    char            instanceBuf[5];
    FILE*           extFileptr;

    camera_data     *cameraData;
    chassis_data    *chassisData;
    gps_data        *gpsData;
    ladar_data      *ladarData;
    odometry_data   *odometryData;
    pilot_data      *pilotData;
    position_data   *positionData;
    scan2d_data     *scan2dData;


    for (i = 0; i < datalogInfoMsg.data.logNum; i++)
    {
        if (datalogInfoMsg.logInfo[i].moduleMbx == msgInfo->src)
        {
            switch (RackName::classId(msgInfo->src))
            {
                case CAMERA:
                    cameraData = CameraData::parse(msgInfo);

                    sprintf(extFilenameBuf, "camera_");
                    sprintf(fileNumBuf, "%i", datalogInfoMsg.logInfo[i].setsLogged + 1);
                    sprintf(instanceBuf, "%i_", RackName::instanceId(msgInfo->src));

                    strncat(extFilenameBuf, instanceBuf, strlen(instanceBuf));
                    strncat(extFilenameBuf, fileNumBuf, strlen(fileNumBuf));

                    if (cameraData->mode == CAMERA_MODE_JPEG)
                    {
                        strcat(extFilenameBuf, ".jpg");
                    }
                    else
                    {
                        strcat(extFilenameBuf, ".raw");
                    }

                    bytes = fprintf(fileptr[i], "%u %i %i %i %i %i %i\n",
                        (unsigned int)cameraData->recordingTime,
                        cameraData->width,
                        cameraData->height,
                        cameraData->depth,
                        cameraData->mode,
                        cameraData->colorFilterId,
                        datalogInfoMsg.logInfo[i].setsLogged + 1);

                    if ((extFileptr = fopen(extFilenameBuf , "w")) == NULL)
                    {
                        GDOS_ERROR("Can't open file for Mbx %n...\n",
                                   datalogInfoMsg.logInfo[i].moduleMbx);
                        return -EIO;
                    }

                    bytesMax = cameraData->width * cameraData->height * cameraData->depth / 8;
                    bytes += fwrite(&cameraData->byteStream[0],
                                    sizeof(cameraData->byteStream[0]), bytesMax, extFileptr);

                    fclose(extFileptr);

                    datalogInfoMsg.logInfo[i].bytesLogged += bytes;
                    datalogInfoMsg.logInfo[i].setsLogged  += 1;
                    break;

                case CHASSIS:
                    chassisData = ChassisData::parse(msgInfo);
                    bytes = fprintf(fileptr[i], "%u %f %f %f %f %f %f %f %u\n",
                        (unsigned int)chassisData->recordingTime,
                        chassisData->deltaX,
                        chassisData->deltaY,
                        chassisData->deltaRho,
                        chassisData->vx,
                        chassisData->vy,
                        chassisData->omega,
                        chassisData->battery,
                        chassisData->activePilot);

                    datalogInfoMsg.logInfo[i].bytesLogged += bytes;
                    datalogInfoMsg.logInfo[i].setsLogged  += 1;
                    break;

                case GPS:
                    gpsData = GpsData::parse(msgInfo);
                    bytes = fprintf(fileptr[i], "%u %i %.16f %.16f %i %f %i %i %li %f %i %i %i %f %f %f\n",
                        (unsigned int)gpsData->recordingTime,
                        gpsData->mode,
                        gpsData->latitude,
                        gpsData->longitude,
                        gpsData->altitude,
                        gpsData->heading,
                        gpsData->speed,
                        gpsData->satelliteNum,
                        (long int)gpsData->utcTime,
                        gpsData->pdop,
                        gpsData->posGK.x,
                        gpsData->posGK.y,
                        gpsData->posGK.z,
                        gpsData->posGK.phi,
                        gpsData->posGK.psi,
                        gpsData->posGK.rho);

                    datalogInfoMsg.logInfo[i].bytesLogged += bytes;
                    datalogInfoMsg.logInfo[i].setsLogged  += 1;
                    break;

                case LADAR:
                    ladarData = LadarData::parse(msgInfo);
                    bytes = fprintf(fileptr[i], "%u %i %i %f %f %i",
                        (unsigned int)ladarData->recordingTime,
                        ladarData->duration,
                        ladarData->maxRange,
                        ladarData->startAngle,
                        ladarData->angleResolution,
                        ladarData->distanceNum);

                    for (j = 0; j < ladarData->distanceNum; j++)
                    {
                        bytes += fprintf(fileptr[i], " %i", ladarData->distance[j]);
                    }

                    bytes += fprintf(fileptr[i], "\n");

                    datalogInfoMsg.logInfo[i].bytesLogged += bytes;
                    datalogInfoMsg.logInfo[i].setsLogged  += 1;
                    break;

                case ODOMETRY:
                    odometryData = OdometryData::parse(msgInfo);
                    bytes = fprintf(fileptr[i], "%u %i %i %i %f %f %f\n",
                        (unsigned int)odometryData->recordingTime,
                        odometryData->pos.x,
                        odometryData->pos.y,
                        odometryData->pos.z,
                        odometryData->pos.phi,
                        odometryData->pos.psi,
                        odometryData->pos.rho);

                    datalogInfoMsg.logInfo[i].bytesLogged += bytes;
                    datalogInfoMsg.logInfo[i].setsLogged  += 1;
                    break;

                case PILOT:
                    pilotData = PilotData::parse(msgInfo);
                    bytes = fprintf(fileptr[i], "%u %i %i %i %f %f %f %i %f %i",
                        (unsigned int)pilotData->recordingTime,
                        pilotData->pos.x,
                        pilotData->pos.y,
                        pilotData->pos.z,
                        pilotData->pos.phi,
                        pilotData->pos.psi,
                        pilotData->pos.rho,
                        pilotData->speed,
                        pilotData->curve,
                        pilotData->splineNum);

                    for (j = 0; j < pilotData->splineNum; j++)
                    {
                        bytes += fprintf(fileptr[i], " %i %i %f %i %i %f %i %i %f %i %i %i %i %i %i %i",
                            pilotData->spline[j].startPos.x,
                            pilotData->spline[j].startPos.y,
                            pilotData->spline[j].startPos.rho,
                            pilotData->spline[j].endPos.x,
                            pilotData->spline[j].endPos.y,
                            pilotData->spline[j].endPos.rho,
                            pilotData->spline[j].centerPos.x,
                            pilotData->spline[j].centerPos.y,
                            pilotData->spline[j].centerPos.rho,
                            pilotData->spline[j].length,
                            pilotData->spline[j].radius,
                            pilotData->spline[j].vMax,
                            pilotData->spline[j].vStart,
                            pilotData->spline[j].vEnd,
                            pilotData->spline[j].aMax,
                            pilotData->spline[j].lbo);
                    }

                    bytes += fprintf(fileptr[i], "\n");

                    datalogInfoMsg.logInfo[i].bytesLogged += bytes;
                    datalogInfoMsg.logInfo[i].setsLogged  += 1;
                    break;

                case POSITION:
                    positionData = PositionData::parse(msgInfo);
                    bytes = fprintf(fileptr[i], "%u %i %i %i %f %f %f\n",
                        (unsigned int)positionData->recordingTime,
                        positionData->pos.x,
                        positionData->pos.y,
                        positionData->pos.z,
                        positionData->pos.phi,
                        positionData->pos.psi,
                        positionData->pos.rho);

                    datalogInfoMsg.logInfo[i].bytesLogged += bytes;
                    datalogInfoMsg.logInfo[i].setsLogged  += 1;
                    break;

                case SCAN2D:
                    scan2dData = Scan2DData::parse(msgInfo);

                    sprintf(extFilenameBuf, "scan2d_");
                    sprintf(fileNumBuf, "%i", datalogInfoMsg.logInfo[i].setsLogged + 1);
                    sprintf(instanceBuf, "%i_", RackName::instanceId(msgInfo->src));

                    strncat(extFilenameBuf, instanceBuf, strlen(instanceBuf));
                    strncat(extFilenameBuf, fileNumBuf, strlen(fileNumBuf));
                    strcat(extFilenameBuf, ".2d");

                    bytes = fprintf(fileptr[i], "%u %u %i %i %i\n",
                        (unsigned int)scan2dData->recordingTime,
                        (unsigned int)scan2dData->duration,
                        scan2dData->maxRange,
                        scan2dData->pointNum,
                        datalogInfoMsg.logInfo[i].setsLogged + 1);

                    if ((extFileptr = fopen(extFilenameBuf , "w")) == NULL)
                    {
                        GDOS_ERROR("Can't open file for Mbx %n...\n",
                                   datalogInfoMsg.logInfo[i].moduleMbx);
                        return -EIO;
                    }

                    for (j = 0; j < scan2dData->pointNum; j++)
                    {
                        bytes += fprintf(extFileptr, "%i %i %i %i %i %i\n",
                            scan2dData->point[j].x,
                            scan2dData->point[j].y,
                            scan2dData->point[j].z,
                            scan2dData->point[j].type,
                            scan2dData->point[j].segment,
                            scan2dData->point[j].intensity);
                    }

                    fclose(extFileptr);

                    datalogInfoMsg.logInfo[i].bytesLogged += bytes;
                    datalogInfoMsg.logInfo[i].setsLogged  += 1;
                    break;

                default:
                    GDOS_PRINT("No write function for module class %n\n", msgInfo->src);
                    return -EINVAL;
            }

            if (bytes < 0)
            {
                GDOS_ERROR("Can't write data package from %n to file, code = %i\n",
                           msgInfo->src, bytes);
                return bytes;
            }
        }
    }

    return 0;
}

int DatalogRec::getStatus(uint32_t destMbxAdr, RackMailbox *replyMbx, uint64_t reply_timeout_ns)
{
    message_info msgInfo;
    int ret;

    if (!replyMbx)
    {
        return -EINVAL;
    }

    ret = replyMbx->sendMsg(MSG_GET_STATUS, destMbxAdr, 0);
    if (ret)
    {
        GDOS_DBG_DETAIL("Proxy cmd to %n: Can't send command %d, code = %d\n",
                        destMbxAdr, MSG_GET_STATUS, ret);
        return ret;
    }
    GDOS_DBG_DETAIL("Proxy cmd to %n: command %d has been sent\n",
                        destMbxAdr, MSG_GET_STATUS);

    while (1)
    {   // waiting for reply (without data)
        ret = replyMbx->recvMsgTimed(reply_timeout_ns, &msgInfo);
        if (ret)
        {
            GDOS_DBG_DETAIL("Proxy cmd to %n: Can't receive reply of "
                         "command %d, code = %d\n",
                         destMbxAdr, MSG_GET_STATUS, ret);
            return ret;
        }

        if (msgInfo.src == destMbxAdr)
        {
            switch(msgInfo.type)
            {
                case MSG_TIMEOUT:
                    GDOS_DBG_DETAIL("Proxy cmd %d to %n: Replied - timeout -\n",
                                    MSG_GET_STATUS, destMbxAdr);
                    return -ETIMEDOUT;

                case MSG_NOT_AVAILABLE:
                    GDOS_DBG_DETAIL("Proxy cmd %d to %n: Replied - not available \n",
                                    MSG_GET_STATUS, destMbxAdr);
                    return -ENODATA;

                case MSG_ENABLED:
                case MSG_DISABLED:
                case MSG_ERROR:
                    return msgInfo.type;
            }
        }
    } // while-loop
    return -EINVAL;
}

int DatalogRec::moduleOn(uint32_t destMbxAdr, RackMailbox *replyMbx, uint64_t reply_timeout_ns)
{
    message_info msgInfo;
    int          ret;

    if (!replyMbx)
    {
        return -EINVAL;
    }

    ret = replyMbx->sendMsg(MSG_ON, destMbxAdr, 0);
    if (ret)
    {
        GDOS_WARNING("Proxy cmd to %n: Can't send command %d, code = %d\n",
                        destMbxAdr, MSG_ON, ret);
        return ret;
    }

    while (1)
    {
        // waiting for reply (without data)
        ret = replyMbx->recvMsgTimed(reply_timeout_ns, &msgInfo);
        if (ret)
        {
            GDOS_WARNING("Proxy cmd to %n: Can't receive reply of "
                         "command %d, code = %d\n",
                         destMbxAdr, MSG_ON, ret);
            return ret;
        }

        if (msgInfo.src == destMbxAdr)
        {
            switch(msgInfo.type)
            {
                case MSG_ERROR:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - error -\n",
                                 MSG_ON, destMbxAdr);
                    return -ECOMM;

                case MSG_TIMEOUT:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - timeout -\n",
                                 MSG_ON, destMbxAdr);
                    return -ETIMEDOUT;

                case MSG_NOT_AVAILABLE:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - not available \n",
                                 MSG_ON, destMbxAdr);
                    return -ENODATA;

                case MSG_OK:
                    return 0;
            }
        }
    } // while-loop
    return -EINVAL;
}


int DatalogRec::getContData(uint32_t destMbxAdr, rack_time_t requestPeriodTime,
                             RackMailbox *dataMbx, RackMailbox *replyMbx,
                             rack_time_t *realPeriodTime, uint64_t reply_timeout_ns)
{
    int ret;
    message_info        msgInfo;
    rack_get_cont_data  send_data;
    rack_cont_data      recv_data;

    send_data.periodTime = requestPeriodTime;
    send_data.dataMbxAdr = dataMbx->getAdr();


    if (!replyMbx)
    {
        return -EINVAL;
    }

    ret = replyMbx->sendDataMsg(MSG_GET_CONT_DATA, destMbxAdr, 0, 1, &send_data,
                                sizeof(rack_get_cont_data));
    if (ret)
    {
        GDOS_WARNING("Proxy cmd to %n: Can't send command %d, code = %d\n",
                     destMbxAdr, MSG_GET_CONT_DATA, ret);
        return ret;
    }

    while (1)
    {
        ret = replyMbx->recvDataMsgTimed(reply_timeout_ns, &recv_data,
                                         sizeof(rack_cont_data), &msgInfo);
        if (ret)
        {
            GDOS_WARNING("Proxy cmd to %n: Can't receive reply of "
                         "command %d, code = %d\n",
                         destMbxAdr, MSG_GET_CONT_DATA, ret);
            return ret;
        }

        if (msgInfo.src == destMbxAdr)
        {
            switch(msgInfo.type)
            {
                case MSG_ERROR:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - error -\n",
                                 MSG_GET_CONT_DATA, destMbxAdr);
                    return -ECOMM;

                case MSG_TIMEOUT:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - timeout -\n",
                                 MSG_GET_CONT_DATA, destMbxAdr);
                    return -ETIMEDOUT;

                case MSG_NOT_AVAILABLE:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - not available \n",
                                 MSG_GET_CONT_DATA, destMbxAdr);
                    return -ENODATA;
            }

            if (msgInfo.type == MSG_CONT_DATA)
            {

                RackContData::parse(&msgInfo);

                if (realPeriodTime)
                {
                    *realPeriodTime = recv_data.periodTime;
                }

                return 0;
            }
        }
    } // while-loop
    return -EINVAL;
}

int DatalogRec::stopContData(uint32_t destMbxAdr, RackMailbox *dataMbx, RackMailbox *replyMbx,
                              uint64_t reply_timeout_ns)
{
    rack_stop_cont_data     send_data;
    message_info            msgInfo;
    int                     ret;

    send_data.dataMbxAdr = dataMbx->getAdr();

    if (!replyMbx)
    {
        return -EINVAL;
    }

    ret = replyMbx->sendDataMsg(MSG_STOP_CONT_DATA, destMbxAdr, 0, 1, &send_data,
                                sizeof(rack_stop_cont_data));
    if (ret)
    {
        GDOS_WARNING("Proxy cmd to %n: Can't send command %d, code = %d\n",
                     destMbxAdr, MSG_STOP_CONT_DATA, ret);
        return ret;
    }

    while (1)
    {
        // waiting for reply (without data)
        ret = replyMbx->recvMsgTimed(reply_timeout_ns, &msgInfo);
        if (ret)
        {
            GDOS_WARNING("Proxy cmd to %n: Can't receive reply of "
                         "command %d, code = %d\n",
                         destMbxAdr, MSG_STOP_CONT_DATA, ret);
            return ret;
        }

        if (msgInfo.src == destMbxAdr)
        {
            switch(msgInfo.type)
            {
                case MSG_ERROR:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - error -\n",
                                 MSG_STOP_CONT_DATA, destMbxAdr);
                    return -ECOMM;

                case MSG_TIMEOUT:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - timeout -\n",
                                 MSG_STOP_CONT_DATA, destMbxAdr);
                    return -ETIMEDOUT;

                case MSG_NOT_AVAILABLE:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - not available \n",
                                 MSG_STOP_CONT_DATA, destMbxAdr);
                    return -ENODATA;

                case MSG_OK:
                return 0;
            }
        }
    } // while-loop
    return -EINVAL;
}

void DatalogRec::logInfoAllModules(datalog_data *data)
{
    int num;

    for (num = 0; num < DATALOG_LOGNUM_MAX; num++)
    {
        data->logInfo[num].logEnable = 0;
        data->logInfo[num].periodTime = 0;
        bzero(data->logInfo[num].filename, 40);
    }

    data->logNum = 0;
    num = data->logNum;

    data->logInfo[num].moduleMbx = RackName::create(CAMERA, 0);
    snprintf((char *)data->logInfo[num].filename, 40, "camera_0.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(CAMERA, 1);
    snprintf((char *)data->logInfo[num].filename, 40, "camera_1.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(CAMERA, 2);
    snprintf((char *)data->logInfo[num].filename, 40, "camera_2.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(CAMERA, 3);
    snprintf((char *)data->logInfo[num].filename, 40, "camera_3.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(CHASSIS, 0);
    snprintf((char *)data->logInfo[num].filename, 40, "chassis_0.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(GPS, 0);
    snprintf((char *)data->logInfo[num].filename, 40, "gps_0.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(GPS, 1);
    snprintf((char *)data->logInfo[num].filename, 40, "gps_1.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(GPS, 2);
    snprintf((char *)data->logInfo[num].filename, 40, "gps_2.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(GPS, 3);
    snprintf((char *)data->logInfo[num].filename, 40, "gps_3.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(LADAR, 0);
    snprintf((char *)data->logInfo[num].filename, 40, "ladar_0.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(LADAR, 1);
    snprintf((char *)data->logInfo[num].filename, 40, "ladar_1.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(LADAR, 2);
    snprintf((char *)data->logInfo[num].filename, 40, "ladar_2.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(LADAR, 3);
    snprintf((char *)data->logInfo[num].filename, 40, "ladar_3.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(ODOMETRY, 0);
    snprintf((char *)data->logInfo[num].filename, 40, "odometry_0.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(ODOMETRY, 1);
    snprintf((char *)data->logInfo[num].filename, 40, "odometry_1.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(PILOT, 0);
    snprintf((char *)data->logInfo[num].filename, 40, "pilot_0.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(PILOT, 1);
    snprintf((char *)data->logInfo[num].filename, 40, "pilot_1.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(PILOT, 2);
    snprintf((char *)data->logInfo[num].filename, 40, "pilot_2.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(POSITION, 0);
    snprintf((char *)data->logInfo[num].filename, 40, "position_0.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(POSITION, 1);
    snprintf((char *)data->logInfo[num].filename, 40, "position_1.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(SCAN2D, 0);
    snprintf((char *)data->logInfo[num].filename, 40, "scan2d_0.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(SCAN2D, 1);
    snprintf((char *)data->logInfo[num].filename, 40, "scan2d_1.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(SCAN2D, 2);
    snprintf((char *)data->logInfo[num].filename, 40, "scan2d_2.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(SCAN2D, 3);
    snprintf((char *)data->logInfo[num].filename, 40, "scan2d_3.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(SCAN2D, 4);
    snprintf((char *)data->logInfo[num].filename, 40, "scan2d_4.dat");
    num++;

    data->logInfo[num].moduleMbx = RackName::create(SCAN2D, 5);
    snprintf((char *)data->logInfo[num].filename, 40, "scan2d_5.dat");
    num++;

    data->logNum = num;
}

int DatalogRec::logInfoCurrentModules(datalog_logInfo *logInfoAll, int num,
                                       datalog_logInfo *logInfoCurrent, RackMailbox *replyMbx,
                                       uint64_t reply_timeout_ns)
{
    int i, status;
    int currNum = 0;

    for (i = 0; i < num; i++)
    {
        status = getStatus(logInfoAll[i].moduleMbx, replyMbx, reply_timeout_ns);

        if ((status == MSG_ENABLED) || (status == MSG_DISABLED))
        {
            memcpy(&logInfoCurrent[currNum].moduleMbx, &logInfoAll[i].moduleMbx,
                   sizeof(datalog_logInfo));
            currNum++;
        }
    }

    return currNum;
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
int DatalogRec::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    GDOS_DBG_DETAIL("DatalogRec::moduleInit ... \n");

    // allocate memory
    contDataPtr = malloc(DATALOG_MSG_SIZE_MAX);
    if (contDataPtr == NULL)
    {
        GDOS_ERROR("Can't allocate contData buffer\n");
        return -ENOMEM;
    }
    // set log variable
    initLog = 1;
    initBits.setBit(INIT_BIT_CONT_DATA_BUFFER);

    // work mailbox
    ret = createMbx(&workMbx, 1, 128, MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // continuous-data mailbox
    ret = createMbx(&contDataMbx, 10, DATALOG_MSG_SIZE_MAX,
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_CONT_DATA);

    // create datalog mutex
    ret = datalogMtx.create();
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MTX_CREATED);

    return 0;

init_error:
    // !!! call local cleanup function !!!
    DatalogRec::moduleCleanup();
    return ret;
}

void DatalogRec::moduleCleanup(void)
{
    GDOS_DBG_DETAIL("DatalogRec::moduleCleanup ... \n");

    // destroy mutex
    if (initBits.testAndClearBit(INIT_BIT_MTX_CREATED))
    {
        datalogMtx.destroy();
    }

    // delete mailboxes
    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_CONT_DATA))
    {
        destroyMbx(&contDataMbx);
    }

    if (initBits.testAndClearBit(INIT_BIT_CONT_DATA_BUFFER))
    {
        free(contDataPtr);
    }

    // call RackDataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }
}

DatalogRec::DatalogRec(void)
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s cmdtask error sleep time
                    5000000000llu,    // 5s datatask error sleep time
                     100000000llu,    // 100ms datatask disable sleep time
                    16,               // command mailbox slots
                    sizeof(datalog_data) + // command mailbox data size per slot
                    DATALOG_LOGNUM_MAX * sizeof(datalog_logInfo),
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    10,               // max buffer entries
                    10)               // data buffer listener
{
    //
    // get values
    //

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(datalog_data_msg));
}