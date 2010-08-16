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
 *      Oliver Wulf  <oliver.wulf@gmx.de>
 *
 */
#ifndef __MCL_PROXY_H__
#define __MCL_PROXY_H__

/*!
 * @ingroup rtsnavigation
 * @defgroup mcl MCL
 *
 * Navigation components that do Monte Carlo Localization.
 *
 * @{
 */

#include <main/rack_proxy.h>
#include <main/defines/position3d.h>

//######################################################################
//# MCL Message Types
//######################################################################

#define MSG_MCL_LOAD_MAP                (RACK_PROXY_MSG_POS_OFFSET + 1)

#define MCL_DATA_POINT_MAX 1000

#define MCL_TYPE_SAMPLE         1
#define MCL_TYPE_MEASUREMENT    2
#define MCL_TYPE_REFLECTOR      3

#define MCL_TYPE_LINE_FEATURE   10

//######################################################################
//# MCLDataPoint
//######################################################################

typedef struct {
    int32_t         x;
    int32_t         y;
    int32_t         z;
    int32_t         type;
    int32_t         layer;
} __attribute__((packed)) mcl_data_point;

class MCLDataPoint
{
    public:
        static void le_to_cpu(mcl_data_point *data)
        {
            data->x      = __le32_to_cpu(data->x);
            data->y      = __le32_to_cpu(data->y);
            data->z      = __le32_to_cpu(data->z);
            data->type   = __le32_to_cpu(data->type);
            data->layer  = __le32_to_cpu(data->layer);
        }

        static void be_to_cpu(mcl_data_point *data)
        {
            data->x      = __be32_to_cpu(data->x);
            data->y      = __be32_to_cpu(data->y);
            data->z      = __be32_to_cpu(data->z);
            data->type   = __be32_to_cpu(data->type);
            data->layer  = __be32_to_cpu(data->layer);
        }
};

//######################################################################
//# MCLData (!!! VARIABLE SIZE !!! MESSAGE !!!)
//######################################################################

/* CREATING A MESSAGE :

typedef struct {
    mcl_data     data;
    mcl_data_point    point[ ... ];
} __attribute__((packed)) mcl_data_msg;

mcl_data_msg msg;

ACCESS: msg.data.point[...] OR msg.point[...];

*/

typedef struct {
    rack_time_t     recordingTime; // has to be first element
    position_3d     pos;
    int32_t         pointNum;
    mcl_data_point  point[0];
} __attribute__((packed)) mcl_data;

class MCLData
{
    public:
        static void le_to_cpu(mcl_data *data)
        {
            int i;
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            Position3D::le_to_cpu(&data->pos);
            data->pointNum      = __le32_to_cpu(data->pointNum);
            for (i=0; i < data->pointNum; i++)
            {
                MCLDataPoint::le_to_cpu(&data->point[i]);
            }
        }

        static void be_to_cpu(mcl_data *data)
        {
            int i;
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            Position3D::be_to_cpu(&data->pos);
            data->pointNum      = __be32_to_cpu(data->pointNum);
            for (i=0; i < data->pointNum; i++)
            {
                MCLDataPoint::be_to_cpu(&data->point[i]);
            }
        }

        static mcl_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            mcl_data *p_data = (mcl_data *)msgInfo->p_data;

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

typedef struct {
    int32_t     filenameLen;
    char        filename[128];
} __attribute__((packed)) mcl_filename;

class MCLFilename
{
    public:
        static void le_to_cpu(mcl_filename *data)
        {
            data->filenameLen = __le32_to_cpu(data->filenameLen);
        }

        static void be_to_cpu(mcl_filename *data)
        {
            data->filenameLen = __be32_to_cpu(data->filenameLen);
        }

        static mcl_filename* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            mcl_filename *p_data = (mcl_filename*)msgInfo->p_data;

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
//# MCL Proxy Functions
//######################################################################

class MCLProxy : public RackDataProxy {

  public:

//
// constructor / destructor
// WARNING -> look at module class id in constuctor
//

    MCLProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
            : RackDataProxy(workMbx, sys_id, MCL, instance)
    {
    };

    ~MCLProxy()
    {
    };


//
// overwriting getData proxy function
// (includes parsing and type conversion)
//

    int getData(mcl_data *recv_data, ssize_t recv_datalen,
                rack_time_t timeStamp)
    {
        return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(mcl_data *recv_data, ssize_t recv_datalen,
                rack_time_t timeStamp, uint64_t reply_timeout_ns);

};

/*@}*/

#endif // __MCL_PROXY_H__
