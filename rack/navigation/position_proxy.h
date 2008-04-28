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
#ifndef __POSITION_PROXY_H__
#define __POSITION_PROXY_H__

/*!
 * @ingroup navigation
 * @defgroup position Position
 *
 * The global position of the mobile robot.
 *
 * @{
 */

#include <main/rack_proxy.h>
#include <navigation/position_proxy.h>
#include <main/defines/position3d.h>

//######################################################################
//# Position Message Types
//######################################################################

#define MSG_POSITION_UPDATE              (RACK_PROXY_MSG_POS_OFFSET + 1)
#define MSG_POSITION_WGS84_TO_POS        (RACK_PROXY_MSG_POS_OFFSET + 2)
#define MSG_POSITION_POS_TO_WGS84        (RACK_PROXY_MSG_POS_OFFSET + 3)
#define MSG_POSITION_GK_TO_POS           (RACK_PROXY_MSG_POS_OFFSET + 4)
#define MSG_POSITION_POS_TO_GK           (RACK_PROXY_MSG_POS_OFFSET + 5)
#define MSG_POSITION_UTM_TO_POS          (RACK_PROXY_MSG_POS_OFFSET + 6)
#define MSG_POSITION_POS_TO_UTM          (RACK_PROXY_MSG_POS_OFFSET + 7)

#define MSG_POSITION_POS                 (RACK_PROXY_MSG_NEG_OFFSET - 2)
#define MSG_POSITION_WGS84               (RACK_PROXY_MSG_NEG_OFFSET - 3)
#define MSG_POSITION_GK                  (RACK_PROXY_MSG_NEG_OFFSET - 5)
#define MSG_POSITION_UTM                 (RACK_PROXY_MSG_NEG_OFFSET - 7)

//######################################################################
//# Position Data (static size - MESSAGE)
//######################################################################

typedef struct{
    rack_time_t  recordingTime; // has to be first element
    position_3d  pos;
    position_3d  var;
} __attribute__((packed)) position_data;

class PositionData
{
    public:
        static void le_to_cpu(position_data *data)
        {
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            Position3D::le_to_cpu(&data->pos);
            Position3D::le_to_cpu(&data->var);
        }

        static void be_to_cpu(position_data *data)
        {
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            Position3D::be_to_cpu(&data->pos);
            Position3D::be_to_cpu(&data->var);
        }

        static position_data* parse(message_info *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            position_data *p_data = (position_data *)msgInfo->p_data;

            if (isDataByteorderLe(msgInfo)) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            setDataByteorder(msgInfo);
            return p_data;
        }
};

//######################################################################
//# Position WGS84 Data (static size - MESSAGE)
//######################################################################
typedef struct{
    double        latitude;       // rad
    double        longitude;      // rad
    int32_t       altitude;       // mm over mean sea level
    float         heading;        // rad
} __attribute__((packed)) position_wgs84_data;

class PositionWgs84Data
{
    public:
        static void le_to_cpu(position_wgs84_data *data)
        {
            data->latitude      = __le64_float_to_cpu(data->latitude);
            data->longitude     = __le64_float_to_cpu(data->longitude);
            data->altitude      = __le32_to_cpu(data->altitude);
            data->heading       = __le32_float_to_cpu(data->heading);
        }

        static void be_to_cpu(position_wgs84_data *data)
        {
            data->latitude      = __be64_float_to_cpu(data->latitude);
            data->longitude     = __be64_float_to_cpu(data->longitude);
            data->altitude      = __be32_to_cpu(data->altitude);
            data->heading       = __be32_float_to_cpu(data->heading);
        }

        static position_wgs84_data* parse(message_info *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            position_wgs84_data *p_data = (position_wgs84_data *)msgInfo->p_data;

            if (isDataByteorderLe(msgInfo)) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            setDataByteorder(msgInfo);
            return p_data;
        }
};

//######################################################################
//# Position GK Data (static size - MESSAGE)
//######################################################################
typedef struct{
    double        northing;       // mm
    double        easting;        // mm
    int32_t       altitude;       // mm over mean sea level
    float         heading;        // rad
} __attribute__((packed)) position_gk_data;

class PositionGkData
{
    public:
        static void le_to_cpu(position_gk_data *data)
        {
            data->northing      = __le64_float_to_cpu(data->northing);
            data->easting       = __le64_float_to_cpu(data->easting);
            data->altitude      = __le32_to_cpu(data->altitude);
            data->heading       = __le32_float_to_cpu(data->heading);
        }

        static void be_to_cpu(position_gk_data *data)
        {
            data->northing      = __be64_float_to_cpu(data->northing);
            data->easting       = __be64_float_to_cpu(data->easting);
            data->altitude      = __be32_to_cpu(data->altitude);
            data->heading       = __be32_float_to_cpu(data->heading);
        }

        static position_gk_data* parse(message_info *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            position_gk_data *p_data = (position_gk_data *)msgInfo->p_data;

            if (isDataByteorderLe(msgInfo)) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            setDataByteorder(msgInfo);
            return p_data;
        }
};

//######################################################################
//# Position UTM Data (static size - MESSAGE)
//######################################################################
typedef struct{
    int           zone;           // utm zone
    double        northing;       // mm
    double        easting;        // mm
    int32_t       altitude;       // mm over mean sea level
    float         heading;        // rad
} __attribute__((packed)) position_utm_data;

class PositionUtmData
{
    public:
        static void le_to_cpu(position_utm_data *data)
        {
            data->zone          = __le32_to_cpu(data->zone);
            data->northing      = __le64_float_to_cpu(data->northing);
            data->easting       = __le64_float_to_cpu(data->easting);
            data->altitude      = __le32_to_cpu(data->altitude);
            data->heading       = __le32_float_to_cpu(data->heading);
        }

        static void be_to_cpu(position_utm_data *data)
        {
            data->zone          = __be32_to_cpu(data->zone);
            data->northing      = __be64_float_to_cpu(data->northing);
            data->easting       = __be64_float_to_cpu(data->easting);
            data->altitude      = __be32_to_cpu(data->altitude);
            data->heading       = __be32_float_to_cpu(data->heading);
        }

        static position_utm_data* parse(message_info *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            position_utm_data *p_data = (position_utm_data *)msgInfo->p_data;

            if (isDataByteorderLe(msgInfo)) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            setDataByteorder(msgInfo);
            return p_data;
        }
};

//######################################################################
//# Position Proxy Functions
//######################################################################

class PositionProxy : public RackDataProxy {

    public:

        PositionProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
              : RackDataProxy(workMbx, sys_id, POSITION, instance)
        {
        };

        ~PositionProxy()
        {
        };

        //
        // overwriting getData (includes parsing and type conversion)
        //

        int getData(position_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp, uint64_t reply_timeout_ns);

        int getData(position_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp)
        {
            return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
        }


        int update(position_data *posData, uint64_t reply_timeout_ns);

        int update(position_data *posData)
        {
            return update(posData, dataTimeout);
        }


        int wgs84ToPos(position_wgs84_data *wgs84Data, position_data *posData,
                       uint64_t reply_timeout_ns);

        int wgs84ToPos(position_wgs84_data *wgs84Data, position_data *posData)
        {
            return wgs84ToPos(wgs84Data, posData, dataTimeout);
        }

        int posToWgs84(position_data *posData, position_wgs84_data *wgs84Data,
                       uint64_t reply_timeout_ns);

        int posToWgs84(position_data *posData, position_wgs84_data *wgs84Data)
        {
            return posToWgs84(posData, wgs84Data, dataTimeout);
        }

        int gkToPos(position_gk_data *gkData, position_data *posData,
                       uint64_t reply_timeout_ns);

        int gkToPos(position_gk_data *gkData, position_data *posData)
        {
            return gkToPos(gkData, posData, dataTimeout);
        }

        int posToGk(position_data *posData, position_gk_data *gkData,
                     uint64_t reply_timeout_ns);

        int posToGk(position_data *posData, position_gk_data *gkData)
        {
            return posToGk(posData, gkData, dataTimeout);
        }

        int utmToPos(position_utm_data *utmData, position_data *posData,
                       uint64_t reply_timeout_ns);

        int utmToPos(position_utm_data *utmData, position_data *posData)
        {
            return utmToPos(utmData, posData, dataTimeout);
        }

        int posToUtm(position_data *posData, position_utm_data *utmData,
                     uint64_t reply_timeout_ns);

        int posToUtm(position_data *posData, position_utm_data *utmData)
        {
            return posToUtm(posData, utmData, dataTimeout);
        }
};

/*@}*/

#endif // __POSITION_PROXY_H__
