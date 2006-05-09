 /* RACK - Robotics Application Construction Kit
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

/* Parameter of the Creative Webcam 
 * SPCX
 * calibrated with focus turned max away (max around the clock looking into lens
 * and no lossrate (640 x 480)
 */
#ifndef __CALIBRATION_SPCA__
#define __CALIBRATION_SPCA__

#define    F    3.108
#define    F_x  554 //f in pixel = 4.8 mm
#define    F_y  556

#define    K1  -0.4294 //radial
#define    K2   0.2007 //radial
#define    P1   -0.0005//tangential
#define    P2   0.00036 //tangential

#define    S_x  1 //scaling
#define    S_y  1
#define    D_x  0.0056 //size of sensor element
#define    D_y  0.0056
#define    E0   335   //348
#define    N0   259

#define    CALIBRATION_WIDTH  640
#define    CALIBRATION_HEIGHT  480

#endif
