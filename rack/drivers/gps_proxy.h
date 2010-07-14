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
#ifndef __GPS_PROXY_H__
#define __GPS_PROXY_H__

/*!
 * @ingroup drivers
 * @defgroup gps GPS
 *
 * Hardware abstraction for GPS receivers.
 * GPS = Global Positioning System
 *
 * @{
 */

#include <main/rack_proxy.h>
#include <main/defines/position3d.h>

#define GPS_MODE_INVALID 1                  /**< no valid gps position fix */
#define GPS_MODE_2D      2                  /**< latitude and longitude of gps position fix valid */
#define GPS_MODE_3D      3                  /**< latitude, longitude and altitude of gps
                                                 position fix valid */

//######################################################################
//# Gps Data (static size - MESSAGE)
//######################################################################

/**
 * gps data structure
 */
typedef struct {
    rack_time_t   recordingTime;            /**< [ms] global timestamp (has to be first element)*/
    int32_t       mode;                     /**< gps mode used for position calculation */
    double        latitude;                 /**< [rad] latitude in WGS84 reference frame */
    double        longitude;                /**< [rad] longitude in WGS84 reference frame */
    int32_t       altitude;                 /**< [mm] height over mean sea level, positive up */
    float         heading;                  /**< [rad] 0 = north, positive clockwise */
    int32_t       speed;                    /**< [mm/s] speed over ground */
    int32_t       satelliteNum;             /**< number of satellites used for position fix
                                                 calculation */
    int64_t       utcTime;                  /**< [s] POSIX time since 1.1.1970 */
    float         pdop;                     /**< gps pdop, positional dilution of precision */
    position_3d   pos;                      /**< global position and orientation */
    position_3d   var;                      /**< standard deviation of position and orientation */
} __attribute__((packed)) gps_data;


class GpsData
{
    public:
        static gps_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            gps_data *data = (gps_data *)msgInfo->p_data;

            data->recordingTime = msgInfo->data32ToCpu(data->recordingTime);
            data->mode          = msgInfo->data32ToCpu(data->mode);
            data->latitude      = msgInfo->data64FloatToCpu(data->latitude);
            data->longitude     = msgInfo->data64FloatToCpu(data->longitude);
            data->altitude      = msgInfo->data32ToCpu(data->altitude);
            data->heading       = msgInfo->data32FloatToCpu(data->heading);
            data->speed         = msgInfo->data32ToCpu(data->speed);
            data->satelliteNum  = msgInfo->data32ToCpu(data->satelliteNum);
            data->utcTime       = msgInfo->data64ToCpu(data->utcTime);
            data->pdop          = msgInfo->data32FloatToCpu(data->pdop);

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                Position3D::le_to_cpu(&data->pos);
                Position3D::le_to_cpu(&data->var);
            }
            else // data in big endian
            {
                Position3D::be_to_cpu(&data->pos);
                Position3D::be_to_cpu(&data->var);
            }

            msgInfo->setDataByteorder();

            return data;
        }
};

//######################################################################
//# Gps Proxy Functions
//######################################################################

class GpsProxy : public RackDataProxy {

  public:

    GpsProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
          : RackDataProxy(workMbx, sys_id, GPS, instance)
    {
    };

    ~GpsProxy()
   {
    };

//
// gps data
//

    int getData(gps_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp)
    {
          return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(gps_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp,
                uint64_t reply_timeout_ns);

};

/*@}*/

#endif // __GPS_PROXY_H__
