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

#define OBJ_RECOG_OBJECT_TYPE_UNCLASSIFIED  0   /**< @ingroup main_defines */
#define OBJ_RECOG_OBJECT_TYPE_UNKNOWN_SMALL 1   /**< @ingroup main_defines */
#define OBJ_RECOG_OBJECT_TYPE_UNKNOWN_BIG   2   /**< @ingroup main_defines */
#define OBJ_RECOG_OBJECT_TYPE_PEDESTRIAN    3   /**< @ingroup main_defines */
#define OBJ_RECOG_OBJECT_TYPE_BIKE          4   /**< @ingroup main_defines */
#define OBJ_RECOG_OBJECT_TYPE_CAR           5   /**< @ingroup main_defines */
#define OBJ_RECOG_OBJECT_TYPE_TRUCK         6   /**< @ingroup main_defines */

/**
 * object recognition object structure
 * @ingroup main_defines
 */
typedef struct {
    int32_t     objectId;                   /**< id of the object */
    int32_t     type;                       /**< type of the object */
    position_3d pos;                        /**< position of the object center */
    position_3d varPos;                     /**< standard deviation of the position */
    position_3d vel;                        /**< velocity of the object center */
    position_3d varVel;                     /**< standard deviation of the velocity */
    point_3d    dim;                        /**< dimension of the objects bounding box */
    float       prob;                       /**< probability, still in use??? */
    image_rect  imageArea;                  /**< image area (texture) representing the object */
} __attribute__((packed)) obj_recog_object;

class ObjRecogObject {
    public:
        static void le_to_cpu(obj_recog_object *data)
        {
            data->objectId = __le32_to_cpu(data->objectId);
            data->type     = __le32_to_cpu(data->type);
            Position3D::le_to_cpu(&data->pos);
            Position3D::le_to_cpu(&data->varPos);
            Position3D::le_to_cpu(&data->vel);
            Position3D::le_to_cpu(&data->varVel);
            Point3D::le_to_cpu(&data->dim);
            data->prob     = __le32_float_to_cpu(data->prob);
            ImageRect::le_to_cpu(&data->imageArea);
        }

        static void be_to_cpu(obj_recog_object *data)
        {
            data->objectId = __be32_to_cpu(data->objectId);
            data->type     = __be32_to_cpu(data->type);
            Position3D::be_to_cpu(&data->pos);
            Position3D::be_to_cpu(&data->varPos);
            Position3D::be_to_cpu(&data->vel);
            Position3D::be_to_cpu(&data->varVel);
            Point3D::be_to_cpu(&data->dim);
            data->prob     = __be32_float_to_cpu(data->prob);
            ImageRect::be_to_cpu(&data->imageArea);
        }
};

#endif // __OBJ_RECOG_OBJECT_H__
