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
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#ifndef __LADAR_PROXY_H__
#define __LADAR_PROXY_H__

/*!
 * @ingroup drivers
 * @defgroup ladar Ladar
 *
 * Hardware abstraction for laser range sensors.
 *
 * @{
 */

#include <main/rack_proxy.h>

//######################################################################
//# Ladar Data (!!! VARIABLE SIZE !!! MESSAGE !!!)
//######################################################################

/* CREATING A MESSAGE :

typedef struct {
  ladar_data    data;
  int32_t       distance[ ... ];
} __attribute__((packed)) ladar_data_msg;

ladar_data_msg msg;

ACCESS: msg.data.distance[...] OR msg.distance[...];

*/

#define LADAR_DATA_MAX_DISTANCE_NUM 720

typedef struct {
    RACK_TIME   recordingTime;    // have to be first element !!!
    RACK_TIME   duration;
    int32_t     maxRange;
    float32_t   startAngle;
    float32_t   angleResolution;
    int32_t     distanceNum;
    int32_t     distance[0];
} __attribute__((packed)) ladar_data;

class LadarData
{
    public:
        static void le_to_cpu(ladar_data *data)
        {
            int i;

            data->recordingTime   = __le32_to_cpu(data->recordingTime);
            data->duration        = __le32_to_cpu(data->duration);
            data->maxRange        = __le32_to_cpu(data->maxRange);
            data->startAngle      = __le32_float_to_cpu(data->startAngle);
            data->angleResolution = __le32_float_to_cpu(data->angleResolution);
            data->distanceNum     = __le32_to_cpu(data->distanceNum);
            for (i = 0; i < data->distanceNum; i++)
            {
                data->distance[i] = __le32_to_cpu(data->distance[i]);
            }
        }

        static void be_to_cpu(ladar_data *data)
        {
            int i;

            data->recordingTime   = __be32_to_cpu(data->recordingTime);
            data->duration        = __be32_to_cpu(data->duration);
            data->maxRange        = __be32_to_cpu(data->maxRange);
            data->startAngle      = __be32_float_to_cpu(data->startAngle);
            data->angleResolution = __be32_float_to_cpu(data->angleResolution);
            data->distanceNum     = __be32_to_cpu(data->distanceNum);
            for (i = 0; i < data->distanceNum; i++)
            {
                data->distance[i] = __be32_to_cpu(data->distance[i]);
            }
        }

        static ladar_data *parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            ladar_data *p_data = (ladar_data *)msgInfo->p_data;

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
//# Ladar Proxy Functions
//######################################################################

class LadarProxy : public RackDataProxy {
  public:

//
// constructor / destructor
// WARNING -> look at module class id in constuctor
//

    LadarProxy(RackMailbox *workMbx, int32_t sys_id, int32_t instance)
        : RackDataProxy(workMbx, sys_id, LADAR, instance)
    {
    };

    ~LadarProxy()
    {
    };

//
// overwriting getData proxy function
// (includes parsing and type conversion)
//

    int getData(ladar_data *recv_data, ssize_t recv_datalen, RACK_TIME timeStamp,
                MessageInfo *msgInfo)
    {
        return getData(recv_data, recv_datalen, timeStamp, dataTimeout, msgInfo);
    }

    int getData(ladar_data *recv_data, ssize_t recv_datalen, RACK_TIME timeStamp,
                uint64_t reply_timeout_ns, MessageInfo *msgInfo);

};

/*@}*/

#endif
