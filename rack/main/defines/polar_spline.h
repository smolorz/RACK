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
#ifndef __POLAR_SPLINE_H__
#define __POLAR_SPLINE_H__

#include <main/defines/point2d.h>
#include <main/defines/position2d.h>

//######################################################################
//# PolarSpline (static size - no message)
//######################################################################

typedef struct
{
    position_2d startPos;
    position_2d endPos;
    position_2d centerPos;
    int32_t     length;
    int32_t     radius;
    int32_t     vMax;
    int32_t     vStart;
    int32_t     vEnd;
    int32_t     aMax;
    int32_t     lbo;
} __attribute__((packed)) polar_spline;

class PolarSpline
{
   public:
        static void le_to_cpu(polar_spline *data)
        {
            Position2D::le_to_cpu(&data->startPos);
            Position2D::le_to_cpu(&data->endPos);
            Position2D::le_to_cpu(&data->centerPos);
            data->length = __le32_to_cpu(data->length);
            data->radius = __le32_to_cpu(data->radius);
            data->vMax   = __le32_to_cpu(data->vMax);
            data->vStart = __le32_to_cpu(data->vStart);
            data->vEnd   = __le32_to_cpu(data->vEnd);
            data->aMax   = __le32_to_cpu(data->aMax);
            data->lbo    = __le32_to_cpu(data->lbo);
        }

        static void be_to_cpu(polar_spline *data)
        {
            Position2D::be_to_cpu(&data->startPos);
            Position2D::be_to_cpu(&data->endPos);
            Position2D::be_to_cpu(&data->centerPos);
            data->length = __be32_to_cpu(data->length);
            data->radius = __be32_to_cpu(data->radius);
            data->vMax   = __be32_to_cpu(data->vMax);
            data->vStart = __be32_to_cpu(data->vStart);
            data->vEnd   = __be32_to_cpu(data->vEnd);
            data->aMax   = __be32_to_cpu(data->aMax);
            data->lbo    = __be32_to_cpu(data->lbo);
        }

        /**********************************************************************
         * Converts the global "position" into relative "spline" coordinates. *
         **********************************************************************/
        static void position2spline(position_2d *position, polar_spline *spline,
                                    position_2d *result)
        {
            point_2d        point;
            double          x, y, cosRho, sinRho;
            double          a, a2, a3, a4, b, b2;
            int             r;
            int             lengthSign, radiusSign;

            // transform position into spline coordinate system
            sinRho = sin(spline->centerPos.phi);
            cosRho = cos(spline->centerPos.phi);

            x = (double)(position->x - spline->centerPos.x);
            y = (double)(position->y - spline->centerPos.y);

            point.x = (int)(  x * cosRho + y * sinRho);
            point.y = (int)(- x * sinRho + y * cosRho);

            // set length sign
            if (spline->length >= 0)
                lengthSign = 1;
            else
                lengthSign = -1;

            // set radius sign
            if (spline->radius > 0)
                radiusSign = 1;
            else
                radiusSign = -1;


            // process curved spline
            if (spline->radius != 0)
            {
                // calculate longitudinal position
                result->x = (int)rint(fabs(spline->radius * 
                                      point_2d_polar_angle(point)) * lengthSign);

                // calculate transversal deviation
                a = (double)result->x / (double)spline->radius;
                b = (double)spline->length / (double)spline->radius;
                a2 = a  * a;
                a3 = a2 * a;
                a4 = a3 * a;
                b2 = b  * b;
                r = (int)rint(spline->radius * (1.0 + a2/2.0 - a3/b + a4/(2.0*b2)));
                result->y = r - point_2d_polar_distance(point) * radiusSign;

                // calculate angle error
                result->phi = normaliseAngleSym0(position->phi - spline->startPos.phi -
                                                (result->x / (float)spline->radius));
            }

              // process direct line
            else
            {
                // calculate longitudinal, transversal and angle values
                result->x    = point.x;
                result->y    = point.y;
                result->phi  = normaliseAngleSym0(position->phi - spline->startPos.phi);
            }
        }
};


#endif /*__POLAR_SPLINE_H__*/
