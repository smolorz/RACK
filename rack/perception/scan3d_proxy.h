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
#ifndef __SCAN3D_PROXY_H__
#define __SCAN3D_PROXY_H__

#include <main/rack_proxy.h>

#include <main/defines/scan_point.h>
#include <main/defines/scan3d_range_img_point.h>
#include <main/defines/position3d.h>

// uncomment to use velodyne buffer sizes for all modules
//#define SCAN3D_USE_VELODYNE

//######################################################################
//# Scan3d message types
//######################################################################

#define MSG_SCAN3D_GET_RANGE_IMAGE           (RACK_PROXY_MSG_POS_OFFSET + 1)
#define MSG_SCAN3D_STORE_DATA                (RACK_PROXY_MSG_POS_OFFSET + 2)

#define MSG_SCAN3D_RANGE_IMAGE               (RACK_PROXY_MSG_NEG_OFFSET - 1)

//######################################################################
//# Scan3d scan modes
//######################################################################

#define SCAN3D_ROLL                 1
#define SCAN3D_PITCH                2
#define SCAN3D_YAW                  3
#define SCAN3D_TOP                  4
#define SCAN3D_TOP_DOWN             5

#define SCAN3D_POSITIVE_TURN        0x100
#define SCAN3D_NEGATIVE_TURN        0x000

#define SCAN3D_SQUARE_GRID          0x200
#define SCAN3D_HEXAGONAL_GRID       0x000

#define SCAN3D_START_180            0x400
#define SCAN3D_START_0              0x000

//######################################################################
//# Scan3d data defines
//######################################################################

#ifdef SCAN3D_USE_VELODYNE
#define SCAN3D_SCAN_MAX             4001
#define SCAN3D_POINT_MAX            256000
#else
#define SCAN3D_SCAN_MAX             400
#define SCAN3D_POINT_MAX            400 * 181
#endif

//######################################################################
//# Scan3dData (!!! VARIABLE SIZE !!!)
//######################################################################

/* CREATING A MESSAGE :

typedef {
  scan3d_data   data;
  scan_point    point[ ... ];
} scan3d_msg;

scan3d_msg msg;

ACCESS: msg.data.point[...] OR msg.point[...]; !!!

*/

/**
 * scan3d data structure
 */
typedef struct {
    rack_time_t recordingTime;              /**< [ms] global timestamp (has to be first element)*/
    rack_time_t duration;                   /**< [ms] duration of the ladar scan */
    int32_t     maxRange;                   /**< [mm] maximum range of the sensor */
    int32_t     scanNum;                    /**< number of 2d raw scans */
    int32_t     scanPointNum;               /**< number of points per 2d raw scan */
    int32_t     scanMode;                   /**< scanning mode,
                                                 e.g. rolling scan, pitching scan, etc. */
    int32_t     scanHardware;               /**< hardware used for scan aquisiton,
                                                 e.g. powercube_sick_lms... */
    int32_t     sectorNum;                  /**< number of sectors of the 3d data */
    int32_t     sectorIndex;                /**< current sector index included in the data */
    position_3d refPos;                     /**< global position of the reference coordinate
                                                 system */
    int32_t     pointNum;                   /**< number of following scan3d points */
    int32_t     compressed;                 /**< compression flag,
                                                 0: no compression,
                                                 >0: length of compressed data byte stream */
    scan_point  point[0];                   /**< list of scan3d points */
} __attribute__((packed)) scan3d_data;

class Scan3dData
{
    public:
        static void le_to_cpu(scan3d_data *data)
        {
            int i;
            data->recordingTime  = __le32_to_cpu(data->recordingTime);
            data->duration       = __le32_to_cpu(data->duration);
            data->maxRange       = __le32_to_cpu(data->maxRange);
            data->scanNum        = __le32_to_cpu(data->scanNum);
            data->scanPointNum   = __le32_to_cpu(data->scanPointNum);
            data->scanMode       = __le32_to_cpu(data->scanMode);
            data->scanHardware   = __le32_to_cpu(data->scanHardware);
            data->sectorNum      = __le32_to_cpu(data->sectorNum);
            data->sectorIndex    = __le32_to_cpu(data->sectorIndex);

            Position3D::le_to_cpu(&data->refPos);

            data->pointNum       = __le32_to_cpu(data->pointNum);
            data->compressed     = __le32_to_cpu(data->compressed);

            if (data->compressed == 0)
            {
                for (i=0; i<data->pointNum; i++)
                {
                    ScanPoint::le_to_cpu(&data->point[i]);
                }
            }
        }

        static void be_to_cpu(scan3d_data *data)
        {
            int i;
            data->recordingTime  = __be32_to_cpu(data->recordingTime);
            data->duration       = __be32_to_cpu(data->duration);
            data->maxRange       = __be32_to_cpu(data->maxRange);
            data->scanNum        = __be32_to_cpu(data->scanNum);
            data->scanPointNum   = __be32_to_cpu(data->scanPointNum);
            data->scanMode       = __be32_to_cpu(data->scanMode);
            data->scanHardware   = __be32_to_cpu(data->scanHardware);
            data->sectorNum      = __be32_to_cpu(data->sectorNum);
            data->sectorIndex    = __be32_to_cpu(data->sectorIndex);

            Position3D::be_to_cpu(&data->refPos);

            data->pointNum       = __be32_to_cpu(data->pointNum);
            data->compressed     = __be32_to_cpu(data->compressed);

            if (data->compressed == 0)
            {
                for (i=0; i<data->pointNum; i++)
                {
                    ScanPoint::be_to_cpu(&data->point[i]);
                }
            }
        }

        static scan3d_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            scan3d_data *p_data = (scan3d_data *)msgInfo->p_data;

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

        static size_t getDatalen(scan3d_data *data)
        {
            if (data->compressed == 0)
            {
                return (sizeof(scan3d_data) + data->pointNum * sizeof(scan_point));
            }
            else
            {
                return (sizeof(scan3d_data) + data->compressed);
            }
        }
};

//######################################################################
//# Scan3dData (!!! VARIABLE SIZE !!!)
//######################################################################

/* CREATING A MESSAGE :

typedef {
  scan3d_range_img_data        data;
  scan3d_range_img_point    point[ ... ];
} scan3d_range_img_msg;

scan3d_range_img_msg msg;

ACCESS: msg.data.point[...] OR msg.point[...]; !!!

*/

/**
 * scan3d range image data structure
 */
typedef struct {
    rack_time_t             recordingTime;  /**< [ms] global timestamp (has to be first element)*/
    int32_t                 scanMode;       /**< scanning mode,
                                                 e.g. rolling scan, pitching scan, etc. */
    int32_t                 maxRange;       /**< [mm] maximum range of the sensor */
    int16_t                 scanNum;        /**< scanning mode,
                                                 e.g. rolling scan, pitching scan, etc. */
    int16_t                 scanPointNum;   /**< number of following scan3d range image points */
    scan3d_range_img_point  point[0];       /**< list of scan3d range image points */
} __attribute__((packed)) scan3d_range_img_data;

class Scan3dRangeImgData
{
    public:
        static void le_to_cpu(scan3d_range_img_data *data)
        {
            int i;
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->scanMode      = __le32_to_cpu(data->scanMode);
            data->maxRange      = __le32_to_cpu(data->maxRange);
            data->scanNum       = __le16_to_cpu(data->scanNum);
            data->scanPointNum  = __le32_to_cpu(data->scanPointNum);

            for (i=0; i<data->scanPointNum; i++)
            {
                Scan3dRangeImgPoint::le_to_cpu(&data->point[i]);
            }
        }

        static void be_to_cpu(scan3d_range_img_data *data)
        {
            int i;
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->scanMode      = __be32_to_cpu(data->scanMode);
            data->maxRange      = __be32_to_cpu(data->maxRange);
            data->scanNum       = __be16_to_cpu(data->scanNum);
            data->scanPointNum  = __be32_to_cpu(data->scanPointNum);

            for (i=0; i<data->scanPointNum; i++)
            {
                Scan3dRangeImgPoint::be_to_cpu(&data->point[i]);
            }
        }

        static scan3d_range_img_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            scan3d_range_img_data *p_data = (scan3d_range_img_data *)msgInfo->p_data;

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
 * Common data structure for 3D range scans.
 * 3D scans are given in the robot coordinate frame.
 *
 * @ingroup proxies_perception
 */
class Scan3dProxy : public RackDataProxy {

    public:

        Scan3dProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
              : RackDataProxy(workMbx, sys_id, SCAN3D, instance)
        {
        };

        ~Scan3dProxy()
        {
        };

//
// get data
//

        int getData(scan3d_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp)
        {
            return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
        }

        int getData(scan3d_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp, uint64_t reply_timeout_ns);

//
// get range image
//

        int getRangeImage(scan3d_range_img_data *recv_data,
                          ssize_t recv_datalen)
        {
              return getRangeImage(recv_data, recv_datalen, dataTimeout);
        }

        int getRangeImage(scan3d_range_img_data *recv_data,
                          ssize_t recv_datalen, uint64_t reply_timeout_ns);

//
//    store data
//
        int storeData(void)
        {
              return storeData(dataTimeout);
        }

        int storeData(uint64_t reply_timeout_ns)
        {
              return proxySendCmd(MSG_SCAN3D_STORE_DATA, reply_timeout_ns);
        }

};

#endif
