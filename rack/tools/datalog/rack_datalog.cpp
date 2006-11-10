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
 #include "rack_datalog.h"

#include <main/argopts.h>

// init_flags
#define INIT_BIT_DATA_MODULE        0
#define INIT_BIT_MBX_WORK           1
#define INIT_BIT_MBX_CONT_DATA      2
#define INIT_BIT_MTX_CREATED        3

//
// data structures
//
RackDatalog *p_inst;

argTable_t argTab[] = {

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
int  RackDatalog::moduleOn(void)
{
    int         i, ret;
    int         init = 0;
    int         logEnable;
    rack_time_t periodTime;
    rack_time_t realPeriodTime;
    rack_time_t datalogPeriodTime = 1000;
    int         moduleMbx;

    GDOS_DBG_INFO("Turn on...\n");

    for (i = 0; i < datalogInfoMsg.data.logNum; i++)
    {
        fileptr[i] = NULL;
    }

    for (i = 0; i < datalogInfoMsg.data.logNum; i++)
    {
        logEnable  = datalogInfoMsg.logInfo[i].logEnable;
        periodTime = datalogInfoMsg.logInfo[i].periodTime;
        moduleMbx  = datalogInfoMsg.logInfo[i].moduleMbx;

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

    return RackDataModule::moduleOn();  // have to be last command in moduleOn();
}

void RackDatalog::moduleOff(void)
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
}

int  RackDatalog::moduleLoop(void)
{
    int             i,j, ret;
    int             bytesMax;
    message_info    msgInfo;

    char            imageFilenameBuf[40] = "camera_";
    char            imageTimestampBuf[20];
    FILE*           imagefileptr;

    camera_data     *cameraData;
    chassis_data    *chassisData;
    gps_data        *gpsData;
    ladar_data      *ladarData;
    odometry_data   *odometryData;
    position_data   *positionData;
    scan2d_data     *scan2dData;
    datalog_data    *pDatalogData = NULL;


    // get continuous data
    ret = contDataMbx.recvDataMsgTimed(4000000000llu, contDataPtr,
                                       DATALOG_MSG_SIZE_MAX, &msgInfo);

    // position data received
    if (msgInfo.type != MSG_DATA)
    {
        GDOS_ERROR("No data package from %n type %i\n", msgInfo.src, msgInfo.type);
        return -EINVAL;
    }

    datalogMtx.lock(RACK_INFINITE);

    // get datapointer from rackdatabuffer
    pDatalogData = (datalog_data *)getDataBufferWorkSpace();

    for (i = 0; i < datalogInfoMsg.data.logNum; i++)
    {
        if (datalogInfoMsg.logInfo[i].moduleMbx == msgInfo.src)
        {
            switch (RackName::classId(msgInfo.src))
            {
                case CAMERA:
                    cameraData = CameraData::parse(&msgInfo);
                    ret = fprintf(fileptr[i], "%i\n", (int)cameraData->recordingTime);

                    sprintf(imageTimestampBuf, "%i", cameraData->recordingTime);
                    strncat(imageFilenameBuf, imageTimestampBuf, strlen(imageTimestampBuf));

                    if (cameraData->mode == CAMERA_MODE_JPEG)
                    {
                        strcat(imageFilenameBuf, ".jpg");
                    }
                    else
                    {
                        strcat(imageFilenameBuf, ".raw");
                    }

                    if ((imagefileptr = fopen(imageFilenameBuf , "w")) == NULL)
                    {
                        datalogMtx.unlock();
                        GDOS_ERROR("Can't open file for Mbx %n...\n",
                                   datalogInfoMsg.logInfo[i].moduleMbx);
                        return -EIO;
                    }

                    bytesMax = cameraData->width * cameraData->height * cameraData->depth / 8;
                    for (j = 0; j < bytesMax; j++)
                    {
                        ret = fputc(cameraData->byteStream[j], imagefileptr);

                        if (ret < 1)
                        {
                            datalogMtx.unlock();
                            GDOS_ERROR("Can't write data package from %n to file, code = %i\n",
                                        msgInfo.src, ret);
                        }
                    }
                    fclose(imagefileptr);
                    break;

                case GPS:
                    gpsData = GpsData::parse(&msgInfo);
                    ret = fprintf(fileptr[i], "%i  %i %.16f %.16f %i %f %i %i %li %f %i %i %i %f %f %f\n",
                        (int)gpsData->recordingTime,
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
                    break;

                case CHASSIS:
                    chassisData = ChassisData::parse(&msgInfo);
                    ret = fprintf(fileptr[i], "%i  %f %f %f %f %f %f %f %i\n",
                        (int)chassisData->recordingTime,
                        chassisData->deltaX,
                        chassisData->deltaY,
                        chassisData->deltaRho,
                        chassisData->vx,
                        chassisData->vy,
                        chassisData->omega,
                        chassisData->battery,
                        chassisData->activePilot);
                    break;

                case LADAR:
                    ladarData = LadarData::parse(&msgInfo);
                    ret = fprintf(fileptr[i], "%i  %i %i %f %f %i ",
                        (int)ladarData->recordingTime,
                        ladarData->duration,
                        ladarData->maxRange,
                        ladarData->startAngle,
                        ladarData->angleResolution,
                        ladarData->distanceNum);

                    for (j = 0; j < ladarData->distanceNum; j++)
                    {
                        ret = fprintf(fileptr[i], " %i", ladarData->distance[j]);

                        if (ret < 0)
                        {
                            datalogMtx.unlock();
                            GDOS_ERROR("Can't write data package from %n to file, code = %i\n",
                                        msgInfo.src, ret);
                            return ret;
                        }
                    }
                    break;

                case ODOMETRY:
                    odometryData = OdometryData::parse(&msgInfo);
                    ret = fprintf(fileptr[i], "%i  %i %i %i %f %f %f\n",
                        (int)odometryData->recordingTime,
                        odometryData->pos.x,
                        odometryData->pos.y,
                        odometryData->pos.z,
                        odometryData->pos.phi,
                        odometryData->pos.psi,
                        odometryData->pos.rho);
                    break;

                case POSITION:
                    positionData = PositionData::parse(&msgInfo);
                    ret = fprintf(fileptr[i], "%i  %i %i %i %f %f %f\n",
                        (int)positionData->recordingTime,
                        positionData->pos.x,
                        positionData->pos.y,
                        positionData->pos.z,
                        positionData->pos.phi,
                        positionData->pos.psi,
                        positionData->pos.rho);
                    break;

                case SCAN2D:
                    scan2dData = Scan2DData::parse(&msgInfo);
                    ret = fprintf(fileptr[i], "%i  %i %i %i ",
                        (int)scan2dData->recordingTime,
                        (int)scan2dData->duration,
                        scan2dData->maxRange,
                        scan2dData->pointNum);

                    for (j = 0; j < scan2dData->pointNum; j++)
                    {
                        ret = fprintf(fileptr[i], " %i %i %i %i %i %i",
                            scan2dData->point[j].x,
                            scan2dData->point[j].y,
                            scan2dData->point[j].z,
                            scan2dData->point[j].type,
                            scan2dData->point[j].segment,
                            scan2dData->point[j].intensity);

                       if (ret < 0)
                        {
                            datalogMtx.unlock();
                            GDOS_ERROR("Can't write data package from %n to file, code = %i\n",
                                       msgInfo.src, ret);
                            return ret;
                         }
                    }
                    break;

                default:
                    GDOS_PRINT("No write function for module class %n\n", msgInfo.src);
                    datalogMtx.unlock();
                    return -EINVAL;
            }

            if (ret < 0)
            {
                GDOS_ERROR("Can't write data package from %n to file, code = %i\n",
                           msgInfo.src, ret);
                datalogMtx.unlock();
                return ret;
            }
        }
    }

    // write new data package
    pDatalogData->recordingTime = rackTime.get();
    pDatalogData->dataLogged    = 1;
    putDataBufferWorkSpace(sizeof(datalog_data));

    datalogMtx.unlock();

    return 0;
}

int  RackDatalog::moduleCommand(message_info *msgInfo)
{
    datalog_info_data *setLogData;

    switch (msgInfo->type)
    {
        case MSG_DATALOG_GET_LOG_STATUS:
            if (initLog == 1)
            {
                datalogInfoMsg.data.logNum = 0;
                datalogInfoMsg.data.logNum = logInfoAllModules(datalogInfoMsg.logInfo);
                datalogInfoMsg.data.logNum = logInfoCurrentModules(datalogInfoMsg.logInfo,
                                                 datalogInfoMsg.data.logNum, datalogInfoMsg.logInfo,
                                                 &workMbx, 1000000000ll);
                initLog = 0;
            }

            logInfoPrintModules(datalogInfoMsg.logInfo, datalogInfoMsg.data.logNum);
            cmdMbx.sendDataMsgReply(MSG_DATALOG_LOG_STATUS, msgInfo, 1, &datalogInfoMsg,
                                    sizeof(datalogInfoMsg));
            break;

        case MSG_DATALOG_SET_LOG:
            setLogData = DatalogInfoData::parse(msgInfo);

            if (datalogInfoMsg.data.logNum == setLogData->logNum)
            {
                memcpy(&datalogInfoMsg.data, &setLogData, sizeof(datalogInfoMsg));
                logInfoPrintModules(datalogInfoMsg.logInfo, datalogInfoMsg.data.logNum);
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

int RackDatalog::getStatus(uint32_t destMbxAdr, RackMailbox *replyMbx, uint64_t reply_timeout_ns)
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
        GDOS_WARNING("Proxy cmd to %n: Can't send command %d, code = %d\n",
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
            GDOS_WARNING("Proxy cmd to %n: Can't receive reply of "
                         "command %d, code = %d\n",
                         destMbxAdr, MSG_GET_STATUS, ret);
            return ret;
        }

        if (msgInfo.src == destMbxAdr)
        {
            switch(msgInfo.type)
            {
                case MSG_TIMEOUT:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - timeout -\n",
                                 MSG_GET_STATUS, destMbxAdr);
                    return -ETIMEDOUT;

                case MSG_NOT_AVAILABLE:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - not available \n",
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

int RackDatalog::moduleOn(uint32_t destMbxAdr, RackMailbox *replyMbx, uint64_t reply_timeout_ns)
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


int RackDatalog::getContData(uint32_t destMbxAdr, rack_time_t requestPeriodTime,
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

int RackDatalog::stopContData(uint32_t destMbxAdr, RackMailbox *dataMbx, RackMailbox *replyMbx,
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

int RackDatalog::logInfoAllModules(datalog_info *logInfo)
{
    int num;

    for (num = 0; num < DATALOG_LOGNUM_MAX; num++)
    {
        logInfo[num].logEnable = 0;
        logInfo[num].periodTime = 0;
        bzero(logInfo[num].filename, 40);
    }

    num = 0;
    logInfo[num].moduleMbx = RackName::create(CAMERA, 0);
    snprintf((char *)logInfo[num].filename, 40, "camera_0_data.sav");
    num++;

    logInfo[num].moduleMbx = RackName::create(CAMERA, 1);
    snprintf((char *)logInfo[num].filename, 40, "camera_1_data.sav");
    num++;

    logInfo[num].moduleMbx = RackName::create(GPS, 0);
    snprintf((char *)logInfo[num].filename, 40, "gps_0_data.sav");
    num++;

    logInfo[num].moduleMbx = RackName::create(GPS, 1);
    snprintf((char *)logInfo[num].filename, 40, "gps_1_data.sav");
    num++;

    logInfo[num].moduleMbx = RackName::create(CHASSIS, 0);
    snprintf((char *)logInfo[num].filename, 40, "chassis_0_data.sav");
    num++;

    logInfo[num].moduleMbx = RackName::create(LADAR, 0);
    snprintf((char *)logInfo[num].filename, 40, "ladar_0_data.sav");
    num++;

    logInfo[num].moduleMbx = RackName::create(LADAR, 1);
    snprintf((char *)logInfo[num].filename, 40, "ladar_1_data.sav");
    num++;

    logInfo[num].moduleMbx = RackName::create(ODOMETRY, 0);
    snprintf((char *)logInfo[num].filename, 40, "odometry_0_data.sav");
    num++;

    logInfo[num].moduleMbx = RackName::create(POSITION, 0);
    snprintf((char *)logInfo[num].filename, 40, "position_0_data.sav");
    num++;

    logInfo[num].moduleMbx = RackName::create(SCAN2D, 0);
    snprintf((char *)logInfo[num].filename, 40, "scan2d_0_data.sav");
    num++;

    return num;
}

int RackDatalog::logInfoCurrentModules(datalog_info *logInfoAll, int num,
                                       datalog_info *logInfoCurrent, RackMailbox *replyMbx,
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
                   sizeof(datalog_info));
            currNum++;
        }
    }

    return currNum;
}

void RackDatalog::logInfoPrintModules(datalog_info *logInfo, int logNum)
{
    int i;

    printf("LogInfo logNum %i\n", logNum);

    for (i = 0; i < logNum; i++)
    {
        printf("logEnable %i moduleMbx %x periodTime %i filename %s\n",
               logInfo[i].logEnable, logInfo[i].moduleMbx, logInfo[i].periodTime,
               logInfo[i].filename);
    }
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
int RackDatalog::moduleInit(void)
{
    int ret;

    // allocate memory
    contDataPtr = malloc(DATALOG_MSG_SIZE_MAX);

    // set log variable
    initLog = 1;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    GDOS_DBG_DETAIL("RackDatalog::moduleInit ... \n");

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
    RackDatalog::moduleCleanup();
    return ret;
}

void RackDatalog::moduleCleanup(void)
{
    GDOS_DBG_DETAIL("RackDatalog::moduleCleanup ... \n");

    // free memory
    free(contDataPtr);

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

    // call RackDataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }
}

RackDatalog::RackDatalog(void)
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s cmdtask error sleep time
                    5000000000llu,    // 5s datatask error sleep time
                     100000000llu,    // 100ms datatask disable sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    10,               // max buffer entries
                    10)               // data buffer listener
{
    //
    // get values
    //

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(datalog_data));
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "RackDatalog");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new RackDatalog
    p_inst = new RackDatalog();
    if (!p_inst)
    {
        printf("Can't create new RackDatalog -> EXIT\n");
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
