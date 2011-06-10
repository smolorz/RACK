/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Matthias Hentschel  <hentschel@rts.uni-hannover.de>
 *
 */
#ifndef __IO_PROXY_H__
#define __IO_PROXY_H__

#include <main/rack_proxy.h>

//######################################################################
//# Io Message Types
//######################################################################
#define MSG_IO_SET_DATA                 (RACK_PROXY_MSG_POS_OFFSET + 1)

//######################################################################
//# Io data defines
//######################################################################
#if defined (__MSG_VELODYNE__)
#define IO_BYTE_NUM_MAX 800000              /**< maximum number of byte values */
#else // __MSG_SCANDRIVE__
#define IO_BYTE_NUM_MAX 32 * 4              /**< maximum number of byte values */
#endif

//######################################################################
//# IoData (!!! VARIABLE SIZE !!! MESSAGE !!!)
//######################################################################

/* CREATING A MESSAGE :

typedef struct {
    io_data     data;
    uint8_t     value[0][ ... ];
} __attribute__((packed)) io_data_msg;

io_data_msg msg;

ACCESS: msg.data.value[...] OR msg.value[...];

*/

/**
 * io data structure
 */
typedef struct {
    rack_time_t     recordingTime;          /**< [ms] global timestamp (has to be first element)*/
    int32_t         valueNum;               /**< number of following byte values */
    uint8_t         value[0];               /**< list of byte values */
} __attribute__((packed)) io_data;

class IoData
{
    public:
        static void le_to_cpu(io_data *data)
        {
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->valueNum      = __le32_to_cpu(data->valueNum);
        }

        static void be_to_cpu(io_data *data)
        {
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->valueNum      = __be32_to_cpu(data->valueNum);
        }

        static io_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            io_data *p_data = (io_data *)msgInfo->p_data;

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

/**
 * Hardware abstraction for simple I/O - units.
 *
 * @ingroup proxies_drivers
 */
class IoProxy : public RackDataProxy
{

  public:

    IoProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
          : RackDataProxy(workMbx, sys_id, IO, instance)
    {
    };

    ~IoProxy()
    {
    };

// get data
    int getData(io_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp)
    {
          return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(io_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp,
                uint64_t reply_timeout_ns);

// set data
    int setData(io_data *recv_data, ssize_t recv_datalen)
    {
        return setData(recv_data, recv_datalen, dataTimeout);
    }

    int setData(io_data *recv_data, ssize_t recv_datalen,
                uint64_t reply_timeout_ns);
};

#endif // __IO_PROXY_H__
