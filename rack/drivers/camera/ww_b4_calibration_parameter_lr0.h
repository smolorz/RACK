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
 *      Marko Reimer <reimer@rts.uni-hannover.de>
 *
 */

/* Parameter of the Basler 102fc
 * with Pentax 96� lens
 * calibrated with "blende" 4
 * and no lossrate
 */
#ifndef __CALIBRATION_WW_B4__
#define __CALIBRATION_WW_B4__

#define    F    4.86
#define    F_x  756 //f in pixel = 4.8 mm
#define    F_y  753

#define    K1  -0.2265 //radial
#define    K2   0.0604 //radial
#define    P1   -0.0004//tangential
#define    P2   0.00013 //tangential

#define    S_x  1 //scaling
#define    S_y  1
#define    D_x  0.00645 //size of sensor element
#define    D_y  0.00645
#define     E0  630
#define     N0  478

#define    CALIBRATION_WIDTH  1280
#define    CALIBRATION_HEIGHT  960

#endif
