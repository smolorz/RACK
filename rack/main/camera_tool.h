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

#ifndef __CAMERA_TOOL_H__
#define __CAMERA_TOOL_H__

#include <main/rack_debug.h>
#include <math.h>
//######################################################################
//# Class CameraTool
//######################################################################

class CameraTool {
  private:

    // only for debugging:
    GdosMailbox    *gdos;
    int32_t         colorLookuptableThermalRed12[4096];

  public:

    CameraTool();
    CameraTool(RackMailbox *p_mbx, int gdos_level);
    void initColorTables();

    int clip(int in);
    int convertCharUYVY2RGB(uint8_t* outputData, uint8_t* inputData, int width, int height);
    int convertCharUYVY2BGR(uint8_t* outputData, uint8_t* inputData, int width, int height);
    int convertCharUYVY2Gray(uint8_t* outputData, uint8_t* inputData, int width, int height);
    int convertCharBGR2RGB(uint8_t* outputData, uint8_t* inputData, int width, int height);
    int convertCharRGB2MONO8(uint8_t* outputData, uint8_t* inputData,int width, int height);
    int convertCharMono82RGBThermalRed(uint8_t* outputData, uint8_t* inputData, int width, int height);
    int convertCharMono122RGBThermalRed(uint8_t* outputData, uint8_t* inputData, int width, int height);
    
    int histogramStrechMono8(uint8_t* outputData, uint8_t* inputData, 
                                    int width, int height, int bottomPercentage, int ceilingPercentage);
    int histogramStrechMono12(unsigned short* outputData, unsigned short* inputData, 
                                    int width, int height, int bottomPercentage, int ceilingPercentage);
    int histogramStrechToMono8(uint8_t *outputData, unsigned short *inputData,
                                    int width, int height, int bottomPercentage, int ceilingPercentage,
                                    int topMargin, int bottomMargin, int leftMargin, int rightMargin);

    int qsMedianFilter16Bit(short *outputData, short *inputData, int width, int height, int radius);
    int lowPassFilter16Bit(short *outputData, short *inputData, int width, int height, int radius);

    ~CameraTool();
};

#endif // __CAMERA_TOOL_H__
