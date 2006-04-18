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
#ifndef __POINT3D_H__
#define __POINT3D_H__

//######################################################################
//# Point3D (static size - no message)
//######################################################################

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} __attribute__((packed)) point_3d;

class Point3D
{
    public:
        static void le_to_cpu(point_3d *data)
        {
            data->x   = __le32_to_cpu(data->x);
            data->y   = __le32_to_cpu(data->y);
            data->z   = __le32_to_cpu(data->z);
        }

        static void be_to_cpu(point_3d *data)
        {
            data->x   = __be32_to_cpu(data->x);
            data->y   = __be32_to_cpu(data->y);
            data->z   = __le32_to_cpu(data->z);
        }

};

#endif /*__POINT3D_H__*/
