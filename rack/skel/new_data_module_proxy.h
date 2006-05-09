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
#ifndef __NEW_DATA_MODULE_API_H__
#define __NEW_DATA_MODULE_API_H__

#include <main/rack_proxy.h>

//######################################################################
//# NewDataModule Message Types
//######################################################################

#define MSG_SEND_CMD                (RACK_PROXY_MSG_POS_OFFSET + 1)
#define MSG_SEND_DATA_CMD           (RACK_PROXY_MSG_POS_OFFSET + 2)
#define MSG_RECV_DATA_CMD           (RACK_PROXY_MSG_POS_OFFSET + 3)
#define MSG_SEND_RECV_DATA_CMD      (RACK_PROXY_MSG_POS_OFFSET + 4)

//######################################################################
//# New Data Module Data (!!! VARIABLE SIZE !!! MESSAGE !!!)
//######################################################################

/* CREATING A MESSAGE :

typedef struct {
  new_data  data;
  int32_t   value[ ... ];
} __attribute__((packed)) new_data_msg;

new_data_msg msg;

ACCESS: msg.data.value[...] OR msg.value[...];

*/

// the maximum number of values
#define NEWDATAMODULE_VALUE_MAX                  20

// new_data will be replied to the sender of:
// -> MSG_GET_DATA
// -> MSG_GET_CONT_DATA

// !!! USE ONLY TYPES WITH CLEARLY LENGTH !!!
// int          -> int32_t
// long long    -> int64_t
// ...

typedef struct new_data {
    RACK_TIME   recordingTime; // !!! HAVE TO BE FIRST ELEMENT !!!
    float32_t   val_A;
    uint32_t    val_B;
    int32_t     num_val;
    int32_t     value[0];
} __attribute__((packed)) new_data;

// class for e.g. parsing functions
// It will be used in proxy functions receiving data and in moduleCommand()
class NewData
{
    public:
        static void le_to_cpu(new_data *data)
        {
            int i;

            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->val_A         = __le32_float_to_cpu(data->val_A);
            data->val_B         = __le32_to_cpu(data->val_B);
            data->num_val       = __le32_to_cpu(data->num_val);
            for (i = 0; i < data->num_val; i++)
            {
                data->value[i] = __le32_to_cpu(data->value[i]);
            }
        }

        static void be_to_cpu(new_data *data)
        {
            int i;

            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->val_A         = __be32_float_to_cpu(data->val_A);
            data->val_B         = __be32_to_cpu(data->val_B);
            data->num_val       = __be32_to_cpu(data->num_val);
            for (i = 0; i < data->num_val; i++)
            {
                data->value[i] = __be32_to_cpu(data->value[i]);
            }
        }

        static new_data *parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            new_data *p_data = (new_data *)msgInfo->p_data;

            if (msgInfo->flags & MSGINFO_DATA_LE) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->usedMbx->setDataByteorder(msgInfo);
            return p_data;
        }

};

//######################################################################
//# New Data Module Data ( static size - MESSAGE )
//######################################################################

// send_data_t will be sent to a recipient (Proxy-Function):
// -> MSG_SEND_DATA_CMD
// -> MSG_SEND_RECV_DATA_CMD

typedef struct {
    float   val_X;
    int32_t val_Y;
} __attribute__((packed)) send_data;

class SendData
{
    public:
        static void le_to_cpu(send_data *data)
        {
            data->val_X = __le32_float_to_cpu(data->val_X);
            data->val_Y = __le32_to_cpu(data->val_Y);
        }

        static void be_to_cpu(send_data *data)
        {
            data->val_X = __be32_float_to_cpu(data->val_X);
            data->val_Y = __be32_to_cpu(data->val_Y);
        }

        static send_data *parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            send_data *p_data = (send_data *)msgInfo->p_data;

            if (msgInfo->flags & MSGINFO_DATA_LE) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->usedMbx->setDataByteorder(msgInfo);
            return p_data;
        }

};

//######################################################################
//# NewDataModule Proxy Functions
//######################################################################

class NewDataModuleProxy : public RackDataProxy {

  public:

//
// constructor / destructor
// WARNING -> look at module class id in constuctor
//

    NewDataModuleProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
          : RackDataProxy(workMbx, sys_id, TEST, instance)
    {
    };

    ~NewDataModuleProxy()
    {
    };

//
// overwriting getData proxy function
// (includes parsing and type conversion)
//

    int getData(new_data *recv_data, ssize_t recv_datalen, RACK_TIME timeStamp,
                MessageInfo *msgInfo)
    {
        return getData(recv_data, recv_datalen, timeStamp, dataTimeout, msgInfo);
    }

    int getData(new_data *recv_data, ssize_t recv_datalen, RACK_TIME timeStamp,
                uint64_t reply_timeout_ns, MessageInfo *msgInfo);

//
// sendCmd
//

    int sendCmd(void) // use default timeout
    {
        return sendCmd(dataTimeout);
    }

    int sendCmd(uint64_t reply_timeout_ns)
    {
        return proxySendCmd(MSG_SEND_CMD, reply_timeout_ns);
    }

//
// sendDataCmd
//

    int sendDataCmd(void) // use default timeout
    {
        return sendDataCmd(dataTimeout);
    }

    int sendDataCmd(uint64_t reply_timeout_ns);

//
// recvDataCmd
//

    int recvDataCmd(new_data *recv_data, ssize_t recv_datalen,
                        MessageInfo *msgInfo) // use default timeout
    {
        return recvDataCmd(recv_data, recv_datalen, dataTimeout, msgInfo);
    }

    int recvDataCmd(new_data* recv_data, ssize_t recv_datalen,
                        uint64_t reply_timeout_ns, MessageInfo *msgInfo);

//
// sendRecvDataCmd
//

    int sendRecvDataCmd(void *send_data, size_t send_datalen,
                            new_data *recv_data, size_t recv_datalen,
                            MessageInfo *msgInfo) // use default timeout
    {
        return sendRecvDataCmd(send_data, send_datalen, recv_data, recv_datalen,
                               dataTimeout, msgInfo);
    }

    int sendRecvDataCmd(void *send_data, size_t send_datalen,
                        new_data *recv_data, size_t recv_datalen,
                        uint64_t reply_timeout_ns, MessageInfo *msgInfo);

};

#endif //__NEW_DATA_MODULE_DATA_H__
