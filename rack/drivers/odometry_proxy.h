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
#ifndef __ODOMETRY_PROXY_H__
#define __ODOMETRY_PROXY_H__

/*!
 * @ingroup drivers
 * @defgroup odometry Odometry
 *
 * Hardware abstraction for incremental positioning sensors.
 * E.g. encoders, gyros, ...
 *
 * @{
 */

#include <main/rack_proxy.h>
#include <drivers/chassis_proxy.h>
#include <main/defines/position3d.h>

//######################################################################
//# Odometry Message Types
//######################################################################

#define MSG_ODOMETRY_RESET      (RACK_PROXY_MSG_POS_OFFSET + 1)

//######################################################################
//# Odometry Data (static size - MESSAGE)
//######################################################################

typedef struct{
    rack_time_t    recordingTime; // have to be first element
    position_3d  pos;
} __attribute__((packed)) odometry_data;

class OdometryData
{
    public:
        static void le_to_cpu(odometry_data *data)
        {
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            Position3D::le_to_cpu(&data->pos);
        }

        static void be_to_cpu(odometry_data *data)
        {
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            Position3D::be_to_cpu(&data->pos);
        }

        static odometry_data* parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            odometry_data *p_data = (odometry_data *)msgInfo->p_data;

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
//# Odometry Proxy Functions
//######################################################################

class OdometryProxy : public RackDataProxy {

      public:

        OdometryProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
              : RackDataProxy(workMbx, sys_id, ODOMETRY, instance)
        {
        };

        ~OdometryProxy()
        {
        };

//
// overwriting getData (includes parsing and type conversion)
//

    int getData(odometry_data *recv_data, ssize_t recv_datalen,
                rack_time_t timeStamp)
    {
      return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(odometry_data *recv_data, ssize_t recv_datalen,
                rack_time_t timeStamp, uint64_t reply_timeout_ns);

    int reset(void)
    {
      return reset(dataTimeout);
    }

    int reset(uint64_t reply_timeout_ns)
    {
      return proxySendCmd(MSG_ODOMETRY_RESET, reply_timeout_ns);
    }

};

/*@}*/

#endif // __ODOMETRY_PROXY_H__
