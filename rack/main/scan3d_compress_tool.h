/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Malte Cornelius <Malte.Cornelius@gmx.de>
 */

#ifndef __SCAN3D_COMPRESS_TOOL_H__
#define __SCAN3D_COMPRESS_TOOL_H__

#define COMPR_S1_NONE                 0x00000000
#define COMPR_S1_REDUCE_BITS          0x00000001
#define COMPR_S1_REDUCE_BYTES         0x00000002
#define COMPR_S1_REDUCE_BYTES_222110  0x00000003
#define COMPR_S1_REDUCE_BYTES_RESORT  0x00000004
#define COMPR_S1_DELTA_RLE            0x00000005
#define COMPR_S1_REMOVE_INVALID       0x01000000


#include <main/rack_gdos.h>
#include <main/compress_tool.h>

//----- minMax contains min and max values of input data, their ranges
//----- and the needed amount of bits and bytes
typedef struct minMax {
    int32_t  xMin,   yMin,   zMin,   tMin,   sMin,   iMin;      //----- min values
    int32_t  xMax,   yMax,   zMax,   tMax,   sMax,   iMax;      //----- max values
    uint32_t xDelta, yDelta, zDelta, tDelta, sDelta, iDelta;    //----- ranges
    uint32_t xBits,  yBits,  zBits,  tBits,  sBits,  iBits;     //----- needed bits
    uint32_t xBytes, yBytes, zBytes, tBytes, sBytes, iBytes;    //----- needed bytes
} minMax;


typedef struct scanPoint {
    int32_t x, y, z, t;
    int16_t s, i;
} scanPoint;



//----- ----------------------------------------------------------------------------------------------------
//----- Class Scan3dCompressTool
//----- ----------------------------------------------------------------------------------------------------

class Scan3dCompressTool {
  private:
    // only for debugging:
    RackGdos    *gdos;

//----- ----------------------------------------------------------------------------------------------------
//----- global variables
//----- ----------------------------------------------------------------------------------------------------
    minMax MinMax;                                  //----- statistics of input data
    scanPoint       *input, *output, *s3d_buf;      //----- pointers to input/output data (points)
    scan3d_data     *inputHeader, *outputHeader;    //----- pointers to input/output data (header)
    CompressTool    compr_Tool;                     //----- CompressTool containing compression steps 2 and 3
    uint8_t         *pbuf1;
    uint8_t         buf1[COMPR_MAX_INPUT_SIZE];
    scan_point      s3d_buf1[SCAN3D_POINT_MAX];

  public:
//----- ----------------------------------------------------------------------------------------------------
//----- prototypes
//----- ----------------------------------------------------------------------------------------------------
    Scan3dCompressTool();
    Scan3dCompressTool(RackMailbox * p_mbx, int32_t gdos_level, RackTime *rt);
    ~Scan3dCompressTool();
    uint32_t Compress(scan3d_data *s3d_in_out, uint32_t flags_s1, uint32_t flags_s2, uint32_t flags_s3);
    void     compr_step1_searchMinMaxValues(uint32_t nPoints);
    void     compr_step1_reduceBitsPerSymbol(compr_bitStream *out, uint32_t nPoints);
    void     compr_step1_reduceBytesPerSymbol(compr_bitStream *out, uint32_t nPoints);
    void     compr_step1_reduceBytesPerSymbol222110(compr_bitStream *out, uint32_t nPoints);
    void     compr_step1_resortBytesByColumns(compr_bitStream *in, compr_bitStream *out, uint32_t columns, uint32_t nPoints);
    uint32_t compr_step1_delta_rle(compr_bitStream *out, uint32_t n);
    uint32_t compr_step1_remove_invalid(uint32_t nPoints);

#ifdef COMPR_UNCOMPR
    uint32_t Decompress(scan3d_data *s3d_in_out);
    uint32_t uncompr_step1_reduceBitsPerSymbol(compr_bitStream *in, uint32_t nPoints);
    uint32_t uncompr_step1_reduceBytesPerSymbol(compr_bitStream *in, uint32_t nPoints);
    uint32_t uncompr_step1_reduceBytesPerSymbol222110(compr_bitStream *in, uint32_t nPoints);
    void     uncompr_step1_resortBytesByColumns(compr_bitStream *in, compr_bitStream *out, uint32_t n, uint32_t columns);
    uint32_t uncompr_step1_delta_rle(compr_bitStream *in, uint32_t n);
    uint32_t uncompr_step1_remove_invalid(uint32_t nIn, uint32_t nOut);
#endif

};

#endif // __SCAN3D_COMPRESS_TOOL_H__
