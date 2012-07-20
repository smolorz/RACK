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
  *      Christian Wieghardt <wieghardt@rts.uni-hannover.de>
 *
 */
#ifndef __DESTINATION_POINT_H__
#define __DESTINATION_POINT_H__

#include <main/defines/point3d.h>
/**
 * destination point structure
 * @ingroup main_defines
 */
typedef struct {
    position_3d   pos;                           /**< 3D position */
    int32_t       speed;                         /**< [mm/s] velocity */
} __attribute__((packed))destination_point;

class DestinationPoint
{
      public:

        static void le_to_cpu(destination_point *data)
        {
            Position3D::le_to_cpu(&data->pos);
            data->speed       = __le32_to_cpu(data->speed);
        }

        static void be_to_cpu(destination_point *data)
        {
            Position3D::be_to_cpu(&data->pos);
            data->speed       = __be32_to_cpu(data->speed);
        }
};

#endif // __DESTINATION_POINT_H__
