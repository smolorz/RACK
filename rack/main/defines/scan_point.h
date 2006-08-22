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
#ifndef __SCAN_POINT_H__
#define __SCAN_POINT_H__

#define TYPE_BOUNDING           0x1000  // 3d scan

#define TYPE_LINE               0x0100
#define TYPE_LINE_HOR           0x0200
#define TYPE_LINE_VER           0x0400
#define TYPE_WALL               0x0800  // 3d scan

#define TYPE_REDUCE_MASK        0x0070

#define TYPE_INVALID            0x0010
#define TYPE_MAX_RANGE          0x0020
#define TYPE_REDUCE             0x0040
#define TYPE_REFLECTOR          0x0080

#define TYPE_HOR_EDGE           0x0008  // 3d scan
#define TYPE_VER_EDGE           0x0004  // 3d scan
#define TYPE_EDGE               0x0004  // 2d scan

#define TYPE_MASK               0x0003

#define TYPE_UNKNOWN            0x0000
#define TYPE_OBJECT             0x0001
#define TYPE_GROUND             0x0002  // 3d scan
#define TYPE_CEILING            0x0003  // 3d scan
#define TYPE_LANDMARK           0x0001  // 2d scan
#define TYPE_OBSTACLE           0x0002  // 2d scan

//######################################################################
//# ScanPoint (static size - no message )
//######################################################################

typedef struct {
    int32_t x;
    int32_t y;
    /** z coordinate is used to store range information in case of 2d scans */
    int32_t z;
    int32_t type;
    int16_t segment;
    int16_t intensity;
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
