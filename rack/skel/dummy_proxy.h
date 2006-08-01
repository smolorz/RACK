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
 *      YourName <YourMailAddress>
 *
 */
#ifndef __DUMMY_PROXY_H__
#define __DUMMY_PROXY_H__

/*!
 * @ingroup skel
 * @defgroup dummy Dummy
 *
 * Example for a new module class.
 *
 * @{
 */

#include <main/rack_proxy.h>

//######################################################################
//# NewDataModule Message Types
//######################################################################

#define DUMMY_SEND_CMD                (RACK_PROXY_MSG_POS_OFFSET + 1)
#define DUMMY_SEND_PARAM              (RACK_PROXY_MSG_POS_OFFSET + 2)
#define DUMMY_RECV_PARAM              (RACK_PROXY_MSG_POS_OFFSET + 3)
#define DUMMY_SEND_RECV_PARAM         (RACK_PROXY_MSG_POS_OFFSET + 4)

#define DUMMY_PARAM                   (RACK_PROXY_MSG_NEG_OFFSET - 1)

//######################################################################
//# New Data Module Data (!!! VARIABLE SIZE !!! MESSAGE !!!)
//######################################################################

/* CREATING A MESSAGE :

typedef struct {
  dummy_data  data;
  int32_t   value[ ... ];
} __attribute__((packed)) dummy_data_msg;

dummy_data_msg msg;

ACCESS: msg.data.value[...] OR msg.value[...];

*/

// the maximum number of values
#define DUMMY_MAX_VALUE_NUM                 20

// dummy_data will be replied to the sender of:
// -> DUMMY_GET_DATA
// -> DUMMY_GET_CONT_DATA

// !!! USE ONLY TYPES WITH CLEARLY LENGTH !!!
// int          -> int32_t
// long long    -> int64_t
// ...

typedef struct {
    RACK_TIME   recordingTime; // !!! HAS TO BE FIRST ELEMENT !!!
    float32_t   valA;
    uint32_t    valB;
    int32_t     valueNum;
    int32_t     value[0];
} __attribute__((packed)) dummy_data;

// class for e.g. parsing functions
// It will be used in proxy functions receiving data and in moduleCommand()
class DummyData
{
    public:
        static void le_to_cpu(dummy_data *data)
        {
            int i;

            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->valA          = __le32_float_to_cpu(data->valA);
            data->valB          = __le32_to_cpu(data->valB);
            data->valueNum      = __le32_to_cpu(data->valueNum);
            for (i = 0; i < data->valueNum; i++)
            {
                data->value[i] = __le32_to_cpu(data->value[i]);
            }
        }

        static void be_to_cpu(dummy_data *data)
        {
            int i;

            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->valA          = __be32_float_to_cpu(data->valA);
            data->valB          = __be32_to_cpu(data->valB);
            data->valueNum      = __be32_to_cpu(data->valueNum);
            for (i = 0; i < data->valueNum; i++)
            {
                data->value[i] = __be32_to_cpu(data->value[i]);
            }
        }

        static dummy_data *parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            dummy_data *pData = (dummy_data *)msgInfo->p_data;

            if (msgInfo->flags & MSGINFO_DATA_LE) // data in little endian
            {
                le_to_cpu(pData);
            }
            else // data in big endian
            {
                be_to_cpu(pData);
            }
            msgInfo->usedMbx->setDataByteorder(msgInfo);
            return pData;
        }

};

//######################################################################
//# New Data Module Data ( static size - MESSAGE )
//######################################################################

// dummy_param_t will be sent to a recipient (Proxy-Function):
// -> DUMMY_SEND_DATA_CMD
// -> DUMMY_SEND_RECV_DATA_CMD

typedef struct {
    float   valX;
    int32_t valY;
} __attribute__((packed)) dummy_param;

class DummyParam
{
    public:
        static void le_to_cpu(dummy_param *data)
        {
            data->valX = __le32_float_to_cpu(data->valX);
            data->valY = __le32_to_cpu(data->valY);
        }

        static void be_to_cpu(dummy_param *data)
        {
            data->valX = __be32_float_to_cpu(data->valX);
            data->valY = __be32_to_cpu(data->valY);
        }

        static dummy_param *parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            dummy_param *pData = (dummy_param *)msgInfo->p_data;

            if (msgInfo->flags & MSGINFO_DATA_LE) // data in little endian
            {
                le_to_cpu(pData);
            }
            else // data in big endian
            {
                be_to_cpu(pData);
            }
            msgInfo->usedMbx->setDataByteorder(msgInfo);
            return pData;
        }

};

//######################################################################
//# NewDataModule Proxy Functions
//######################################################################

class DummyProxy : public RackDataProxy {

  public:

//
// constructor / destructor
// WARNING -> look at module class id in constuctor
//

    DummyProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
          : RackDataProxy(workMbx, sys_id, TEST, instance)
    {
    };

    ~DummyProxy()
    {
    };

//
// overwriting getData proxy function
// (includes parsing and type conversion)
//

    int getData(dummy_data *recv_data, ssize_t recv_datalen, RACK_TIME timeStamp)
    {
        return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(dummy_data *recv_data, ssize_t recv_datalen, RACK_TIME timeStamp,
                uint64_t reply_timeout_ns);

//
// sendCmd
//

    int sendCmd(void) // use default timeout
    {
        return sendCmd(dataTimeout);
    }

    int sendCmd(uint64_t reply_timeout_ns)
    {
        return proxySendCmd(DUMMY_SEND_CMD, reply_timeout_ns);
    }

//
// sendDataCmd
//

    int sendParam(dummy_param *send_data, size_t send_datalen) // use default timeout
    {
        return sendParam(send_data, send_datalen, dataTimeout);
    }

    int sendParam(dummy_param *send_data, size_t send_datalen, uint64_t reply_timeout_ns);

//
// recvDataCmd
//

    int recvParam(dummy_data *recv_data, ssize_t recv_datalen) // use default timeout
    {
        return recvParam(recv_data, recv_datalen, dataTimeout);
    }

    int recvParam(dummy_data* recv_data, ssize_t recv_datalen, uint64_t reply_timeout_ns);

//
// sendRecvDataCmd
//

    int sendRecvParam(dummy_param *send_data, size_t send_datalen,
                      dummy_param *recv_data, size_t recv_datalen) // use default timeout
    {
        return sendRecvParam(send_data, send_datalen, recv_data, recv_datalen, dataTimeout);
    }

    int sendRecvParam(dummy_param *send_data, size_t send_datalen,
                      dummy_param *recv_data, size_t recv_datalen,
                      uint64_t reply_timeout_ns);

};

/*@}*/

#endif //__DUMMY_PROXY_H__
