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
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#ifndef __SCAN3D_RANGE_IMG_POINT_H__
#define __SCAN3D_RANGE_IMG_POINT_H__

//######################################################################
//# scan3d range image point (static size)
//######################################################################

typedef struct {
    int16_t     range;
    int16_t     type;
} __attribute__((packed)) scan3d_range_img_point;

class Scan3dRangeImgPoint {
    public:
        static void le_to_cpu(scan3d_range_img_point *data)
        {
            data->range = __le16_to_cpu(data->range);
            data->type  = __le16_to_cpu(data->type);
        }

        static void be_to_cpu(scan3d_range_img_point *data)
        {
            data->range = __be16_to_cpu(data->range);
            data->type  = __be16_to_cpu(data->type);
        }
};

#endif // __SCAN3D_RANGE_IMG_POINT_H__
