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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
#ifndef __SCAN_POINT_H__
#define __SCAN_POINT_H__

#define SCAN_POINT_TYPE_INVALID            0x0010   /**< @ingroup main_defines */
#define SCAN_POINT_TYPE_MAX_RANGE          0x0020   /**< @ingroup main_defines */
#define SCAN_POINT_TYPE_REDUCE             0x0040   /**< @ingroup main_defines */
#define SCAN_POINT_TYPE_REFLECTOR          0x0080   /**< @ingroup main_defines */

#define SCAN_POINT_TYPE_HOR_EDGE           0x0008   /**< 3d scan @ingroup main_defines */
#define SCAN_POINT_TYPE_VER_EDGE           0x0004   /**< 3d scan @ingroup main_defines */
#define SCAN_POINT_TYPE_EDGE               0x0004   /**< 2d scan @ingroup main_defines */

#define SCAN_POINT_TYPE_MASK               0x0003   /**< @ingroup main_defines */

#define SCAN_POINT_TYPE_UNKNOWN            0x0000   /**< @ingroup main_defines */
#define SCAN_POINT_TYPE_OBJECT             0x0001   /**< 3d scan @ingroup main_defines */
#define SCAN_POINT_TYPE_GROUND             0x0002   /**< 3d scan @ingroup main_defines */
#define SCAN_POINT_TYPE_CEILING            0x0003   /**< 3d scan @ingroup main_defines */
#define SCAN_POINT_TYPE_LANDMARK           0x0001   /**< 2d scan @ingroup main_defines */
#define SCAN_POINT_TYPE_OBSTACLE           0x0002   /**< 2d scan @ingroup main_defines */
#define SCAN_POINT_TYPE_DYN_OBSTACLE       0x0003   /**< 2d scan @ingroup main_defines */

/**
 * scan point structure
 * @ingroup main_defines
 */
typedef struct {
    int32_t x;                              /**< [mm] x-coordinate */
    int32_t y;                              /**< [mm] y-coordinate */
    int32_t z;                              /**< [mm] z-coordinate, in case of 2d scanse the
                                                      z-coordinate is used to store range
                                                      information */
    int32_t type;                           /**< bitmask defining the type of the scan point */
    int16_t segment;                        /**< segment number of the scan point */
    int16_t intensity;                      /**< intensity of the scan point */
} __attribute__((packed)) scan_point;


class ScanPoint {
      public:

        static void le_to_cpu(scan_point *data)
        {
            data->x         = __le32_to_cpu(data->x);
            data->y         = __le32_to_cpu(data->y);
            data->z         = __le32_to_cpu(data->z);
            data->type      = __le32_to_cpu(data->type);
            data->segment   = __le16_to_cpu(data->segment);
            data->intensity = __le16_to_cpu(data->intensity);
        }

        static void be_to_cpu(scan_point *data)
        {
            data->x         = __be32_to_cpu(data->x);
            data->y         = __be32_to_cpu(data->y);
            data->z         = __be32_to_cpu(data->z);
            data->type      = __be32_to_cpu(data->type);
            data->segment   = __be16_to_cpu(data->segment);
            data->intensity = __be16_to_cpu(data->intensity);
        }
};

#endif // __SCAN_POINT_H__
