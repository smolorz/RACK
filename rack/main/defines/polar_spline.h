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
#include <main/defines/waypoint2d.h>

#define MAX_ROTATION_ANGLE  (350.0 * M_PI / 180.0)

//######################################################################
//# PolarSpline (static size - no message)
//######################################################################

typedef struct
{
    waypoint_2d basepoint;
    position_2d startPos;
    position_2d endPos;
    position_2d centerPos;
    int32_t     length;
    int32_t     radius;
    int32_t     vMax;
    int32_t     vStart;
    int32_t     vEnd;
    int32_t     accMax;
    int32_t     decMax;
    int32_t     type;
    int32_t     request;
    int32_t     lbo;
} __attribute__((packed)) polar_spline;

class PolarSpline
{
   public:
        static void le_to_cpu(polar_spline *data)
        {
            Waypoint2d::le_to_cpu(&data->basepoint);
            Position2D::le_to_cpu(&data->startPos);
            Position2D::le_to_cpu(&data->endPos);
            Position2D::le_to_cpu(&data->centerPos);
            data->length = __le32_to_cpu(data->length);
            data->radius = __le32_to_cpu(data->radius);
            data->vMax   = __le32_to_cpu(data->vMax);
            data->vStart = __le32_to_cpu(data->vStart);
            data->vEnd   = __le32_to_cpu(data->vEnd);
            data->accMax = __le32_to_cpu(data->accMax);
            data->decMax = __le32_to_cpu(data->decMax);
            data->type   = __le32_to_cpu(data->type);
            data->request= __le32_to_cpu(data->request);
            data->lbo    = __le32_to_cpu(data->lbo);
        }

        static void be_to_cpu(polar_spline *data)
        {
            Waypoint2d::be_to_cpu(&data->basepoint);
            Position2D::be_to_cpu(&data->startPos);
            Position2D::be_to_cpu(&data->endPos);
            Position2D::be_to_cpu(&data->centerPos);
            data->length = __be32_to_cpu(data->length);
            data->radius = __be32_to_cpu(data->radius);
            data->vMax   = __be32_to_cpu(data->vMax);
            data->vStart = __be32_to_cpu(data->vStart);
            data->vEnd   = __be32_to_cpu(data->vEnd);
            data->accMax = __be32_to_cpu(data->accMax);
            data->decMax = __be32_to_cpu(data->decMax);
            data->type   = __be32_to_cpu(data->type);
            data->request= __be32_to_cpu(data->request);
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
            sinRho = sin(spline->centerPos.rho);
            cosRho = cos(spline->centerPos.rho);

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
                double fixedAngle=normaliseAngle(lengthSign * radiusSign * point_2d_polar_angle(point));

                // 360 degree turns are not plausible (wa are not dancing!)
                if(fixedAngle > MAX_ROTATION_ANGLE){
                    fixedAngle=0.0;
                }

                result->x = (int)rint(fabs(spline->radius  * fixedAngle ) * lengthSign);

                /*
                result->x = (int)rint(fabs(spline->radius *
                                 normaliseAngle(lengthSign * radiusSign *
                                               point_2d_polar_angle(point))) * lengthSign);
                */

                // calculate transversal deviation
                a = (double)result->x / (double)spline->radius;
                b = (double)spline->length / (double)spline->radius;
                a2 = a  * a;
                a3 = a2 * a;
                a4 = a3 * a;
                b2 = b  * b;
                r = (int)rint(spline->radius *
                              (1.0 + a2/2.0 - a3/b + a4/(2.0*b2)));
                result->y = r - point_2d_polar_distance(point) * radiusSign;

                // calculate angle error
                result->rho = normaliseAngleSym0(position->rho -
                                            spline->startPos.rho -
                                           (result->x / (float)spline->radius));
            }

              // process direct line
            else
            {
                // calculate longitudinal, transversal and angle values
                result->x    = point.x;
                result->y    = point.y;
                result->rho  = normaliseAngleSym0(position->rho -
                                                  spline->startPos.rho);
            }
        }


        /**********************************************************************
         * Converts the relative spline coordinates "splinePos" into a global *
         * "position"                                                         *
         **********************************************************************/
        static void spline2position(position_2d *splinePos,
                                    polar_spline *spline, position_2d *result)
        {
            point_2d        point;
            double          cosRho, sinRho;
            double          a, a2, a3, a4, b, b2;
            int             r, radius;
            int             radiusSign;


            // set radius sign
            if (spline->radius > 0)
                radiusSign = 1;
            else
                radiusSign = -1;


            // process curved spline
            if (spline->radius != 0)
            {
                // calculate splinePos-radius in spline coordinate system
                a = (double)splinePos->x / (double)spline->radius;
                b = (double)spline->length / (double)spline->radius;
                a2 = a  * a;
                a3 = a2 * a;
                a4 = a3 * a;
                b2 = b  * b;
                r = (int)rint(spline->radius *
                             (1.0 + a2/2.0 - a3/b + a4/(2.0*b2)));
                radius = r - splinePos->y;

                // transform spline position into global coordinate system
                sinRho  = sin(-spline->centerPos.rho - a);
                cosRho  = cos(-spline->centerPos.rho - a);
                point.x = (int)( radius * radiusSign * cosRho);
                point.y = (int)(-radius * radiusSign * sinRho);

                // calculate global position and angle values
                result->x    = spline->centerPos.x + point.x;
                result->y    = spline->centerPos.y + point.y;
                result->rho  = normaliseAngle(spline->startPos.rho +
                                              splinePos->rho + (float)a);
            }

            // process direct line
            else
            {
                // transform spline position into global coordinate system
                sinRho  = sin(-spline->centerPos.rho);
                cosRho  = cos(-spline->centerPos.rho);
                point.x = (int)( splinePos->x * cosRho + splinePos->y * sinRho);
                point.y = (int)(-splinePos->x * sinRho + splinePos->y * cosRho);

                // calculate global position and angle values
                result->x    = spline->centerPos.x + point.x;
                result->y    = spline->centerPos.y + point.y;
                result->rho  = normaliseAngle(spline->startPos.rho +
                                              splinePos->rho);
            }
        }
};



#endif /*__POLAR_SPLINE_H__*/
