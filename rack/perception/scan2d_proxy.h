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
#ifndef __SCAN_2D_PROXY_H__
#define __SCAN_2D_PROXY_H__

/*!
 * @ingroup perception
 * @defgroup scan2d Scan2d
 *
 * Common data structure for 2D range scans. E.g. from laser, sonar, ...
 * 2D scans are given in the robot coordinate frame.
 *
 * @{
 */

#include <main/rack_proxy.h>

#include <main/defines/scan_point.h>

#define SCAN2D_POINT_MAX 720

//######################################################################
//# Scan2DData (!!! VARIABLE SIZE !!! MESSAGE !!!)
//######################################################################

/* CREATING A MESSAGE :

typedef struct {
  scan2d_data     data;
  scan_point      point[ ... ];
} __attribute__((packed)) scan2d_data_msg;

scan2d_data_msg msg;

ACCESS: msg.data.point[...] OR msg.point[...];

*/

typedef struct {
    rack_time_t       recordingTime;  // have to be first element !!!
    rack_time_t       duration;
    int32_t         maxRange;
    int32_t         pointNum;
    scan_point      point[0];
} __attribute__((packed)) scan2d_data;

class Scan2DData
{
    public:
        static void le_to_cpu(scan2d_data *data)
        {
            int i;

            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->duration      = __le32_to_cpu(data->duration);
            data->maxRange      = __le32_to_cpu(data->maxRange);
            data->pointNum      = __le32_to_cpu(data->pointNum);
            for (i=0; i< data->pointNum; i++) {
                ScanPoint::le_to_cpu(&data->point[i]);
            }
        }

        static void be_to_cpu(scan2d_data *data)
        {
            int i;

            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->duration      = __be32_to_cpu(data->duration);
            data->maxRange      = __be32_to_cpu(data->maxRange);
            data->pointNum      = __be32_to_cpu(data->pointNum);
            for (i=0; i< data->pointNum; i++) {
                ScanPoint::be_to_cpu(&data->point[i]);
            }
        }

        static scan2d_data* parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            scan2d_data *p_data = (scan2d_data *)msgInfo->p_data;

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
//# Chassis Proxy Functions
//######################################################################

class Scan2DProxy : public RackDataProxy {

    public:

//
// constructor / destructor
// WARNING -> look at module class id in constuctor
//

        Scan2DProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
                : RackDataProxy(workMbx, sys_id, SCAN2D, instance)
        {
        };

        ~Scan2DProxy()
        {
        };


//
// overwriting getData proxy function
// (includes parsing and type conversion)
//


        int getData(scan2d_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp)
        {
            return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
        }

        int getData(scan2d_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp, uint64_t reply_timeout_ns);

};

/*@}*/

#endif // __SCAN_2D_H__
