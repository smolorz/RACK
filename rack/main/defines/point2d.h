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
#ifndef __POINT2D_H__
#define __POINT2D_H__

#include <main/defines/position2d.h>
#include <main/angle_tool.h>

//######################################################################
//# Point2D (static size - no message)
//######################################################################

typedef struct {
    int32_t x;
    int32_t y;
} __attribute__((packed)) point_2d;

class Point2D
{
    public:
        static void le_to_cpu(point_2d *data)
        {
            data->x   = __le32_to_cpu(data->x);
            data->y   = __le32_to_cpu(data->y);
        }

        static void be_to_cpu(point_2d *data)
        {
            data->x   = __be32_to_cpu(data->x);
            data->y   = __be32_to_cpu(data->y);
        }

};

static inline int point_2d_polar_distance(point_2d point)
{
    double x,y;

    x = point.x;
    y = point.y;

    return (int)sqrt(x*x + y*y);
}

static inline int point_2d_polar_distance(point_2d point, point_2d origin)
{
    double x,y;

    x = point.x - origin.x;
    y = point.y - origin.y;

    return (int)sqrt(x*x + y*y);
}

static inline int point_2d_polar_distance(point_2d point, position_2d origin)
{
    double x,y;

    x = point.x - origin.x;
    y = point.y - origin.y;

    return (int)sqrt(x*x + y*y);
}


static inline float point_2d_polar_angle(point_2d point)
{
    return atan2((double)point.y, (double)point.x);
}

static inline float point_2d_polar_angle(point_2d point, point_2d origin)
{
    double x,y;

    x = point.x - origin.x;
    y = point.y - origin.y;

    return atan2(y, x);
}

static inline float point_2d_polar_angle(point_2d point, position_2d origin)
{
    double x,y;

    x = point.x - origin.x;
    y = point.y - origin.y;

    return normaliseAngleSym0(atan2(y, x) - (double)origin.rho);
}

#endif /*__POINT2D_H__*/
