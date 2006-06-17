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
#ifndef __PILOT_TOOL_H__
#define __PILOT_TOOL_H__

#include <main/defines/scan_point.h>
#include <perception/scan2d_proxy.h>
#include <drivers/chassis_proxy.h>

#define MOVING    0
#define HOLD      1

/*****************************************************************************
 * Converts radius to curviness                                              *
 *****************************************************************************/
static inline float radius2Curve(int radius)
{
    if(radius == 0)
    {
        return 0.0f;
    }
    else
    {
        return (1.0f / (float)radius);
    }
}

/*****************************************************************************
 * Converts curviness to radius                                              *
 *****************************************************************************/
static inline int curve2Radius(float curve)
{
    float radiusF;

    if (curve == 0.0f)
    {
        return 0;
    }
    else
    {
        radiusF = 1.0f / curve;

        if ((radiusF > 500000) | (radiusF < -500000))
        {
            return 0;
        }
        else
        {
            return (int)radiusF;
        }
    }
}

/*****************************************************************************
 * This function is an assistant for the speed reduction of the robot. The   *
 * parameters "speed" and "radius" are the current speed and radius value of *
 * the robot. The flag "moveStatus" indicates if the robot is currently      *
 * holding or moving. This value is necessary for the hysteresis             *
 * functionality of this assistant (0 if no functionality is desired).       *
 * This assistant calculates for each point in the scan2D "scan"             *
 * the max. driveable speed without collision. If this speed value is less   *
 * then the current speed the current speed is reduced.                      *
 *****************************************************************************/
static inline int safeSpeed(int speed, int radius, int *moveStatus,
                            scan2d_data *scan,
                            chassis_param_data *param)
{
    int     i;
    int     xMax, xMin;
    int     yMax, yMin;
    int     sign;
    int     vMax, vMaxX, vMaxY;
    float   rotationSpeed;

    // set parameters for forward movement

    if (speed > 0)  // set hysteresis boundaries
    {
        if ((moveStatus != NULL) && (*moveStatus == HOLD))
        {
            xMax =  param->boundaryFront + 2 * param->safetyMargin +
                    param->safetyMarginMove;
              xMin = -param->boundaryBack - param->safetyMargin;
              yMax =  param->boundaryRight + 2 * param->safetyMargin;
              yMin = -param->boundaryLeft  - 2 * param->safetyMargin;
        }
        else    // set normal boundaries
        {
            xMax =  param->boundaryFront + param->safetyMargin +
                    param->safetyMarginMove;
              xMin = -param->boundaryBack;
              yMax =  param->boundaryRight + param->safetyMargin;
              yMin = -param->boundaryLeft  - param->safetyMargin;
        }

        // reduce max speed
        if (speed > param->vxMax)
        {
            speed = param->vxMax;
        }

        // set movement flag
        sign = 1;

    }
    else if (speed < 0) // backward
    {
        if ((moveStatus != NULL) && (*moveStatus == HOLD))
        { // set hysteresis boundaries
            xMax =  param->boundaryFront + param->safetyMargin;
            xMin = -param->boundaryBack - 2 * param->safetyMargin -
                    param->safetyMarginMove;
            yMax =  param->boundaryRight + 2 * param->safetyMargin;
            yMin = -param->boundaryLeft  - 2 * param->safetyMargin;
        }
        else // set normal boundaries
        {
            xMax =  param->boundaryFront;
            xMin = -param->boundaryBack - param->safetyMargin -
                    param->safetyMarginMove;
            yMax =  param->boundaryRight + param->safetyMargin;
            yMin = -param->boundaryLeft - param->safetyMargin;
        }

        // limit reverse speed
        if (speed < -500)
        {
            speed = -500;
        }

        // set movement flag
        sign    = -1;
        speed   = abs(speed);

    }
    else // no movement
    {
        speed = 0;

        // set move status
        if (moveStatus != NULL)
        {
            *moveStatus = HOLD;
        }

        return speed;
    }

    // reduce max rotation speed
    if (radius != 0)
    {
        rotationSpeed = (float)(speed) / (float)abs(radius);

        if (rotationSpeed > param->omegaMax)
        {
            speed = (int)(abs(radius) * param->omegaMax);
        }
    }

    // check Scan2D
    for (i = 0; i < scan->pointNum; i += 2)
    {
        if (((scan->point[i].type & TYPE_INVALID) == 0) &
            ((scan->point[i].type & TYPE_MASK) != TYPE_LANDMARK))
        {
            // check x-coordinate
              if (scan->point[i].x > xMax)
              {
                if (sign > 0)
                    vMaxX = (int)rint((float)abs(scan->point[i].x - xMax) / param->breakConstant);
                else
                    vMaxX = speed;
            }
            else if (scan->point[i].x < xMin)
            {
                if (sign > 0)
                    vMaxX = speed;
                else
                    vMaxX = (int)rint((float)abs(scan->point[i].x - xMin) / param->breakConstant);
            }
            else
            {
                vMaxX = 0;
              }


            // check y coordinate
            if (scan->point[i].y > yMax)
            {
                vMaxY  =  (int)rint((float)abs(scan->point[i].y - yMax) * 2.0f / param->breakConstant);
            }
            else if (scan->point[i].y < yMin)
            {
                vMaxY  =  (int)rint((float)abs(scan->point[i].y - yMin) * 2.0f / param->breakConstant);
            }
            else
            {
                vMaxY = 0;
            }

            // reduce speed if necessary
            if (vMaxX > vMaxY)
                vMax = vMaxX;
            else
                vMax = vMaxY;

            if (vMax < speed)
                speed = vMax;
        }
    }


    // set min speed
    if ((speed != 0) && (speed < param->vxMin))
        speed = param->vxMin;

    // set new move status
    if (moveStatus != NULL)
    {
        if (speed != 0)
            *moveStatus = MOVING;
        else
            *moveStatus = HOLD;
    }

    return speed * sign;
}

#endif // __PILOT_TOOL_H__
