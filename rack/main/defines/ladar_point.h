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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
#ifndef __LADAR_POINT_H__
#define __LADAR_POINT_H__

#define LADAR_POINT_TYPE_UNKNOWN            0x0000  /**< @ingroup main_defines */
#define LADAR_POINT_TYPE_TRANSPARENT        0x0001  /**< @ingroup main_defines */
#define LADAR_POINT_TYPE_RAIN               0x0002  /**< @ingroup main_defines */
#define LADAR_POINT_TYPE_DIRT               0x0003  /**< @ingroup main_defines */

#define LADAR_POINT_TYPE_INVALID            0x0010  /**< @ingroup main_defines */
#define LADAR_POINT_TYPE_REFLECTOR          0x0080  /**< @ingroup main_defines */

/**
 * ladar point structure
 * @ingroup main_defines
 */
typedef struct {
    float       angle;                      /**< [rad] angle of the ladar point in sensor
                                                       reference frame, positive clockwise */
    int32_t     distance;                   /**< [mm] distance of the ladar point in sensor
                                                      reference frame */
    int32_t     type;                       /**< bitmask defining the type of the ladar point */
    int32_t     intensity;                  /**< intensity of the measurement */
} __attribute__((packed)) ladar_point;

class LadarPoint {
      public:

        static void le_to_cpu(ladar_point *data)
        {
            data->angle     = __le32_float_to_cpu(data->angle);
            data->distance  = __le32_to_cpu(data->distance);
            data->type      = __le32_to_cpu(data->type);
            data->intensity = __le32_to_cpu(data->intensity);
        }

        static void be_to_cpu(ladar_point *data)
        {
            data->angle     = __be32_float_to_cpu(data->angle);
            data->distance  = __be32_to_cpu(data->distance);
            data->type      = __be32_to_cpu(data->type);
            data->intensity = __be32_to_cpu(data->intensity);
        }
};

#endif // __LADAR_POINT_H__
