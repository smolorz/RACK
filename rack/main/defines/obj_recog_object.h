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
#ifndef __OBJ_RECOG_OBJECT_H__
#define __OBJ_RECOG_OBJECT_H__

#include <main/defines/position3d.h>
#include <main/defines/point3d.h>
#include <main/defines/image_rect.h>

//######################################################################
//# object recognition object (static size)
//######################################################################

typedef struct {
    int32_t     objectId;
    position_3d pos;
    position_3d vel;
    point_3d    dim;
    float       prob;
    image_rect  imageArea;
} __attribute__((packed)) obj_recog_object;

class ObjRecogObject {
    public:
        static void le_to_cpu(obj_recog_object *data)
        {
            data->objectId = __le32_to_cpu(data->objectId);
            Position3D::le_to_cpu(&data->pos);
            Position3D::le_to_cpu(&data->vel);
            Point3D::le_to_cpu(&data->dim);
            data->prob     = __le32_float_to_cpu(data->prob);
            ImageRect::le_to_cpu(&data->imageArea);
        }

        static void be_to_cpu(obj_recog_object *data)
        {
            data->objectId = __be32_to_cpu(data->objectId);
            Position3D::be_to_cpu(&data->pos);
            Position3D::be_to_cpu(&data->vel);
            Point3D::be_to_cpu(&data->dim);
            data->prob     = __be32_float_to_cpu(data->prob);
            ImageRect::be_to_cpu(&data->imageArea);
        }
};

#endif // __OBJ_RECOG_OBJECT_H__
