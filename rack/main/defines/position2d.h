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
#ifndef __POSITION2D_H__
#define __POSITION2D_H__

#include <main/angle_tool.h>

//######################################################################
//# Position2D (static size - no message )
//######################################################################

typedef struct {
    int32_t x;
    int32_t y;
    float   rho;
} __attribute__((packed)) position_2d;

class Position2D
{
    public:
        static void le_to_cpu(position_2d *data)
        {
            data->x   = __le32_to_cpu(data->x);
            data->y   = __le32_to_cpu(data->y);
            data->rho = __le32_float_to_cpu(data->rho);
        }

        static void be_to_cpu(position_2d *data)
        {
            data->x   = __be32_to_cpu(data->x);
            data->y   = __be32_to_cpu(data->y);
            data->rho = __be32_float_to_cpu(data->rho);
        }
};

static inline int position_2d_distance(position_2d position, position_2d origin)
{
    double x,y;

    x = position.x - origin.x;
    y = position.y - origin.y;

    return (int)sqrt(x*x + y*y);
}

static inline float position_2d_angle(position_2d position, position_2d origin)
{
    return normaliseAngleSym0(position.rho - origin.rho);
}

#endif /*__POSITION2D_H__*/
