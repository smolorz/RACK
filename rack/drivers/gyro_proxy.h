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
 *      Joerg Langenberg  <joerg.langenberg@gmx.net>
 *
 */
#ifndef __GYRO_PROXY_H__
#define __GYRO_PROXY_H__

#include <main/rack_proxy.h>

//######################################################################
//# Gyro Data (static size - MESSAGE)
//######################################################################

/**
 * gyro data structure
 */
typedef struct {
    rack_time_t recordingTime;              /**< [ms] global timestamp (has to be first element)*/
    float       roll;                       /**< [rad] rotation about the x-axis,
                                                       clockwise positive */
    float       pitch;                      /**< [rad] rotation about the y-axis,
                                                       clockwise positive */
    float       yaw;                        /**< [rad] rotation about the z-axis,
                                                       clockwise positive */
    float       aX;                         /**< [mm/s^2] acceleration in x-axis */
    float       aY;                         /**< [mm/s^2] acceleration in y-axis */
    float       aZ;                         /**< [mm/s^2] acceleration in z-axis */
    float       wRoll;                      /**< [rad/s] angular velocity around the x-axis,
                                                         clockwise positive */
    float       wPitch;                     /**< [rad/s] angular velocity around the y-axis,
                                                         clockwise positive */
    float       wYaw;                       /**< [rad/s] angular velocity around the z-axis,
                                                         clockwise positive */
} __attribute__((packed)) gyro_data;

class GyroData
{
    public:
        static void le_to_cpu(gyro_data *data)
        {
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->roll          = __le32_float_to_cpu(data->roll);
            data->pitch         = __le32_float_to_cpu(data->pitch);
            data->yaw           = __le32_float_to_cpu(data->yaw);
            data->aX            = __le32_float_to_cpu(data->aX);
            data->aY            = __le32_float_to_cpu(data->aY);
            data->aZ            = __le32_float_to_cpu(data->aZ);
            data->wRoll         = __le32_float_to_cpu(data->wRoll);
            data->wPitch        = __le32_float_to_cpu(data->wPitch);
            data->wYaw          = __le32_float_to_cpu(data->wYaw);
        }

        static void be_to_cpu(gyro_data *data)
        {
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->roll          = __be32_float_to_cpu(data->roll);
            data->pitch         = __be32_float_to_cpu(data->pitch);
            data->yaw           = __be32_float_to_cpu(data->yaw);
            data->aX            = __be32_float_to_cpu(data->aX);
            data->aY            = __be32_float_to_cpu(data->aY);
            data->aZ            = __be32_float_to_cpu(data->aZ);
            data->wRoll         = __be32_float_to_cpu(data->wRoll);
            data->wPitch        = __be32_float_to_cpu(data->wPitch);
            data->wYaw          = __be32_float_to_cpu(data->wYaw);
        }

        static gyro_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            gyro_data *p_data = (gyro_data *)msgInfo->p_data;

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
 * Hardware abstraction for gyroscopes and Inertial Measurements Units IMUs.
 *
 * @ingroup proxies_drivers
 */
class GyroProxy : public RackDataProxy {

  public:

    GyroProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
          : RackDataProxy(workMbx, sys_id, GYRO, instance)
    {
    };

    ~GyroProxy()
    {
    };

//
// gyro data
//

    int getData(gyro_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp)
    {
          return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(gyro_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp,
                uint64_t reply_timeout_ns);

};

#endif // __GYRO_PROXY_H__
