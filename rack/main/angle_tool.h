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
#ifndef __ANGLE_TOOL_H__
#define __ANGLE_TOOL_H__

#include <math.h>

/**
 * Normalises the angle between: 0 <= angle <= 2*pi (single precision)
 * @ingroup main_tools
 */
static inline float normaliseAngle(float angle)
{
  if (angle < 0.0f)
  {
    return normaliseAngle(angle + 2.0f * (float)M_PI);
  }

  if (angle >= 2.0f * (float)M_PI)
  {
    return normaliseAngle(angle - 2.0f * (float)M_PI);
  }

  return angle;
}

/**
 * Normalises the angle between: 0 <= angle <= 2*pi (double precision)
 * @ingroup main_tools
 */
static inline double normaliseAngle(double angle)
{
  if (angle < 0.0)
  {
    return normaliseAngle(angle + 2.0 * M_PI);
  }

  if (angle >= 2.0 * M_PI)
  {
    return normaliseAngle(angle - 2.0 * M_PI);
  }

  return angle;
}

/**
 * Normalises the angle between: -pi < angle <= pi (single precision)
 * @ingroup main_tools
 */
static inline float normaliseAngleSym0(float angle)
{
    if (angle <= (float)-M_PI)
    {
        return normaliseAngleSym0(angle + 2.0f * (float)M_PI);
    }

    if (angle > (float)M_PI)
    {
        return normaliseAngleSym0(angle - 2.0f * (float)M_PI);
    }

  return angle;
}

/**
 * Normalises the angle between: -pi < angle <= pi (double precision)
 * @ingroup main_tools
 */
static inline double normaliseAngleSym0(double angle)
{
    if (angle <= -M_PI)
    {
        return normaliseAngleSym0(angle + 2.0 * M_PI);
    }

    if (angle > M_PI)
    {
        return normaliseAngleSym0(angle - 2.0 * M_PI);
    }

    return angle;
}

/**
 * Calculates the difference between angle "angleB" and "angle_A" (single precision)
 * @ingroup main_tools
 */
static inline float deltaAngle(float angleA, float angleB)
{
    return normaliseAngleSym0(angleB - angleA);
}

/**
 * Calculates the difference between angle "angleB" and "angle_A" (double precision)
 * @ingroup main_tools
 */
static inline double deltaAngle(double angleA, double angleB)
{
    return normaliseAngleSym0(angleB - angleA);
}

#endif // __ANGLE_TOOL_H__
