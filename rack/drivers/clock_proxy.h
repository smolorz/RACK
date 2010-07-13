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
#ifndef __CLOCK_PROXY_H__
#define __CLOCK_PROXY_H__

/*!
 * @ingroup drivers
 * @defgroup clock Clock
 *
 * Hardware abstraction for real-time clock sensors.
 *
 * @{
 */

#include <main/rack_proxy.h>

#define CLOCK_SYNC_MODE_NONE   0
#define CLOCK_SYNC_MODE_REMOTE 1

//######################################################################
//# Clock Data (static size - MESSAGE)
//######################################################################

typedef struct {
    rack_time_t   recordingTime;  // has to be first element
    int32_t       hour;           // hour   [00...23]
    int32_t       minute;         // monute [00...59]
    int32_t       second;         // second [00...59]
    int32_t       day;            // day    [01...31]
    int32_t       month;          // month  [01...12]
    int32_t       year;           // year   [00...99] without century
    int64_t       utcTime;        // POSIX time in sec since 1.1.1970
    int32_t       dayOfWeek;      // day of week, 1=monday...7=sunday
    int32_t       syncMode;       // mode for clock synchronisation
    int32_t       varT;           // variance of the clock sensor in ms
} __attribute__((packed)) clock_data;


class ClockData
{
    public:
        static void le_to_cpu(clock_data *data)
        {
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->hour          = __le32_to_cpu(data->hour);
            data->minute        = __le32_to_cpu(data->minute);
            data->second        = __le32_to_cpu(data->second);
            data->day           = __le32_to_cpu(data->day);
            data->month         = __le32_to_cpu(data->month);
            data->year          = __le32_to_cpu(data->year);
            data->utcTime       = __le64_to_cpu(data->utcTime);
            data->dayOfWeek     = __le32_to_cpu(data->dayOfWeek);
            data->syncMode      = __le32_to_cpu(data->syncMode);
        }

        static void be_to_cpu(clock_data *data)
        {
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->hour          = __be32_to_cpu(data->hour);
            data->minute        = __be32_to_cpu(data->minute);
            data->second        = __be32_to_cpu(data->second);
            data->day           = __be32_to_cpu(data->day);
            data->month         = __be32_to_cpu(data->month);
            data->year          = __be32_to_cpu(data->year);
            data->utcTime       = __be64_to_cpu(data->utcTime);
            data->dayOfWeek     = __be32_to_cpu(data->dayOfWeek);
            data->syncMode      = __be32_to_cpu(data->syncMode);
        }

        static clock_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            clock_data *p_data = (clock_data *)msgInfo->p_data;

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
//# Clock Proxy Functions
//######################################################################

class ClockProxy : public RackDataProxy {

  public:

    ClockProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
             : RackDataProxy(workMbx, sys_id, CLOCK, instance)
    {
    };

    ~ClockProxy()
   {
    };

//
// clock data
//

    int getData(clock_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp)
    {
          return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(clock_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp,
                uint64_t reply_timeout_ns);

};

/*@}*/

#endif // __CLOCK_PROXY_H__
