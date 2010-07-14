/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
  * Authors
 *      Oliver Wulf        <wulf@rts.uni-hannover.de>
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 */

#ifndef __DATALOG_PROXY_H__
#define __DATALOG_PROXY_H__

/*!
 * @ingroup tools
 * @defgroup datalog Datalog
 *
 * Data strcture for datalog
 *
 * @{
 */

#include <main/rack_proxy.h>
#include <main/rack_name.h>

#define DATALOG_LOGNUM_MAX 100              /**< maximum number of data logs */

//######################################################################
//# Datalog Message Types
//######################################################################

#define MSG_DATALOG_INIT_LOG            (RACK_PROXY_MSG_POS_OFFSET + 1)
#define MSG_DATALOG_SET_LOG             (RACK_PROXY_MSG_POS_OFFSET + 2)
#define MSG_DATALOG_GET_LOG_STATUS      (RACK_PROXY_MSG_POS_OFFSET + 3)
#define MSG_DATALOG_LOG_STATUS          (RACK_PROXY_MSG_NEG_OFFSET - 3)



//######################################################################
//# Datalog Log Info (static size - MESSAGE)
//######################################################################

/**
 * datalog log info data structure
 */
typedef struct {
    int32_t         logEnable;              /**< log enable flag, 1 = enable, else = disable */
    uint32_t        moduleMbx;              /**< mailbox adress of the module */
    rack_time_t     periodTime;             /**< [ms] periodtime of the module */
    int32_t         maxDataLen;             /**< maxium number of bytes of the data output
                                                 of the module */
    uint8_t         filename[40];           /**< filename for datalogging */
    uint32_t        bytesLogged;            /**< counter for logged bytes */
    uint32_t        setsLogged;             /**< counter for logged full datasets */
} __attribute__((packed)) datalog_log_info;

class DatalogLogInfo
{
    public:
        static void le_to_cpu(datalog_log_info *data)
        {
            data->logEnable   = __le32_to_cpu(data->logEnable);
            data->moduleMbx   = __le32_to_cpu(data->moduleMbx);
            data->periodTime  = __le32_to_cpu(data->periodTime);
            data->maxDataLen  = __le32_to_cpu(data->maxDataLen);
            data->bytesLogged = __le32_to_cpu(data->bytesLogged);
            data->setsLogged  = __le32_to_cpu(data->setsLogged);
        }

        static void be_to_cpu(datalog_log_info *data)
        {
            data->logEnable   = __be32_to_cpu(data->logEnable);
            data->moduleMbx   = __be32_to_cpu(data->moduleMbx);
            data->periodTime  = __be32_to_cpu(data->periodTime);
            data->maxDataLen  = __be32_to_cpu(data->maxDataLen);
            data->bytesLogged = __be32_to_cpu(data->bytesLogged);
            data->setsLogged  = __be32_to_cpu(data->setsLogged);
        }
};


//######################################################################
//# DatalogData (!!! VARIABLE SIZE !!! MESSAGE !!!)
//######################################################################

/* CREATING A MESSAGE :

typedef struct {
    datalog_info_data   data;
    datalog_logInfo     logInfo[ ... ];
} __attribute__((packed)) datalog_data_msg;

datalog_data_msg msg;

ACCESS: msg.data.logInfo[...] OR msg.logInfo[...];
*/

/**
 * datalog data structure
 */
typedef struct {
    rack_time_t      recordingTime;         /**< [ms]  global timestamp (has to be first element)*/
    uint8_t          logPathName[40];       /**< path for logging */
    int32_t          logNum;                /**< number of following log infos */
    datalog_log_info logInfo[0];            /**< list of log infos */
} __attribute__((packed)) datalog_data;

class DatalogData
{
    public:
        static void le_to_cpu(datalog_data *data)
        {
            int i;
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->logNum        = __le32_to_cpu(data->logNum);
            for (i = 0; i < data->logNum; i++)
            {
                DatalogLogInfo::le_to_cpu(&data->logInfo[i]);
            }
        }

        static void be_to_cpu(datalog_data *data)
        {
            int i;
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->logNum        = __be32_to_cpu(data->logNum);
            for (i = 0; i < data->logNum; i++)
            {
                DatalogLogInfo::be_to_cpu(&data->logInfo[i]);
            }
        }

        static datalog_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            datalog_data *p_data = (datalog_data *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }
};

//######################################################################
//# Datalog Proxy Functions
//######################################################################

class DatalogProxy : public RackDataProxy {

    public:

//
// constructor / destructor
// WARNING -> look at module class id in constuctor
//
    DatalogProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
            : RackDataProxy(workMbx, sys_id, DATALOG, instance)
    {
    };

    ~DatalogProxy()
    {
    };


//
// overwriting getData proxy function
// (includes parsing and type conversion)
//
    int getData(datalog_data *recv_data, ssize_t recv_datalen,
                rack_time_t timeStamp)
    {
        return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(datalog_data *recv_data, ssize_t recv_datalen,
                rack_time_t timeStamp, uint64_t reply_timeout_ns);


// initLog
    int initLog()
    {
        return initLog(dataTimeout);
    }

    int initLog(uint64_t reply_timeout_ns);

// setLog
    int setLog(datalog_data *recv_data, ssize_t recv_datalen)
    {
        return setLog(recv_data, recv_datalen, dataTimeout);
    }

    int setLog(datalog_data *recv_data, ssize_t recv_datalen,
               uint64_t reply_timeout_ns);

// getLogStatus
    int getLogStatus(datalog_data *recv_data, ssize_t recv_datalen)
    {
        return getLogStatus(recv_data, recv_datalen, dataTimeout);
    }

    int getLogStatus(datalog_data *recv_data, ssize_t recv_datalen,
                     uint64_t reply_timeout_ns);
};

/*@}*/

#endif // __DATALOG_PROXY_H__
