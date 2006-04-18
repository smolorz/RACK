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
 *      Marko Reimer <reimer@l3s.de>
 *
 */

/* Parameter of the Basler 102fc
 * with Pentax 96� lens
 * calibrated with "blende" 4
 * and lossrate of 2 (640x480)
 */
#ifndef __CALIBRATION_WW_B4__
#define __CALIBRATION_WW_B4__

#define    F    4.8
#define    F_x  376 //f in pixel = 4.8 mm
#define    F_y  377

#define    K1  -0.2395 //radial
#define    K2   0.0704 //radial
#define    P1   0.00004//tangential
#define    P2   0.0013 //tangential

#define    S_x  1 //scaling
#define    S_y  1
#define    D_x  0.0129 //(0.00645 * 2) //size of sensor element
#define    D_y  0.0129 //(0.00645 * 2)
#define     E0  315
#define     N0  240

#define    CALIBRATION_WIDTH  640
#define    CALIBRATION_HEIGHT 480

#endif
