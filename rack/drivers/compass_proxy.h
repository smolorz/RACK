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
 *      Matthias Hentschel      <hentschel@rts.uni-hannover.de>
 *
 */
#ifndef __COMPASS_PROXY_H__
#define __COMPASS_PROXY_H__

#include <main/rack_proxy.h>

//######################################################################
//# Compass Data (static size - MESSAGE)
//######################################################################

/**
 * compass data structure
 */
typedef struct {
    rack_time_t   recordingTime;            /**< [ms]  global timestamp (has to be first element)*/
    float         orientation;              /**< [rad] magnetic orientation with 0 pointing
                                                       towards magnetic north, clockwise positive */
    float         varOrientation;           /**< [rad] standard deviation of magnetic orientation */
} __attribute__((packed)) compass_data;


class CompassData
{
    public:
        static void le_to_cpu(compass_data *data)
        {
            data->recordingTime  = __le32_to_cpu(data->recordingTime);
            data->orientation    = __le32_float_to_cpu(data->orientation);
            data->varOrientation = __le32_float_to_cpu(data->varOrientation);
        }

        static void be_to_cpu(compass_data *data)
        {
            data->recordingTime  = __be32_to_cpu(data->recordingTime);
            data->orientation    = __be32_float_to_cpu(data->orientation);
            data->varOrientation = __be32_float_to_cpu(data->varOrientation);
        }

        static compass_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            compass_data *p_data = (compass_data *)msgInfo->p_data;

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
 * Hardware abstraction for magnetic orientation sensors.
 *
 * @ingroup proxies_drivers
 */
class CompassProxy : public RackDataProxy {

  public:

    CompassProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
             : RackDataProxy(workMbx, sys_id, COMPASS, instance)
    {
    };

    ~CompassProxy()
   {
    };

//
// compass data
//

    int getData(compass_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp)
    {
          return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(compass_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp,
                uint64_t reply_timeout_ns);

};

#endif // __COMPASS_PROXY_H__
