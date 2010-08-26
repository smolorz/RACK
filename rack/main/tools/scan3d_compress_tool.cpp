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

//----- ----------------------------------------------------------------------------------------------------
//----- Scan3dCompressTool compresses and decompresses scan3d_data structures
//----- The compressed data overwrites the input data in the same array.
//----- Compression is done in three steps:
//----- Step 1: Basic algorithms
//----- Step 2: Table based coding and transforms
//----- Step 3: Statistical coding
//-----
//----- The methods of step 1 are implemented in Scan3dCompressTool,
//----- steps 2 and three in CompressTool, a tool for compression of
//----- universal data (not just scan3d_data structures).
//----- Therefore, Scan3dCompressTool accesses CompressTool.
//-----
//----- Several compression methods can be chosen (Parameters flags_s1, flags_s2, flags_s3)
//----- COMPR_S1_NONE
//----- COMPR_S1_REDUCE_BITS
//----- COMPR_S1_REDUCE_BYTES
//----- COMPR_S1_REDUCE_BYTES_222110
//----- COMPR_S1_REDUCE_BYTES_RESORT
//----- COMPR_S1_DELTA_RLE
//----- COMPR_S1_REMOVE_INVALID
//----- COMPR_S2_NONE
//----- COMPR_S2_LZSS
//----- COMPR_S2_LZW
//----- COMPR_S2_LZWV
//----- COMPR_S2_BWT
//----- COMPR_S2_MTF
//----- COMPR_S2_MTF_LZSS
//----- COMPR_S2_BWT_MTF
//----- COMPR_S3_NONE
//----- COMPR_S3_HUFFMANN
//----- COMPR_S3_AD_HUFFMANN
//-----
//----- Some methods are excludes from compling to optimise the needed memory, if
//----- these methods are not used. You can enable them by enabling #defines in compress:tool.h
//----- ----------------------------------------------------------------------------------------------------


#include <main/scan3d_compress_tool.h>

#include <math.h>
#include <string.h>


Scan3dCompressTool::Scan3dCompressTool() {
    gdos = NULL;
    input = NULL;
}

Scan3dCompressTool::Scan3dCompressTool(RackMailbox * p_mbx, int32_t gdos_level, RackTime *rt) {
    gdos = new RackGdos(p_mbx, gdos_level);
    input = NULL;
    compr_Tool.rackTime = rt;
}

Scan3dCompressTool::~Scan3dCompressTool() {
    if (gdos) delete gdos;
}

//----- ----------------------------------------------------------------------------------------------------
//----- main compression function
//----- ----------------------------------------------------------------------------------------------------
uint32_t Scan3dCompressTool::Compress(scan3d_data *s3d_in_out, uint32_t flags_s1, uint32_t flags_s2, uint32_t flags_s3) {
    // return if no compression needed
    if ((flags_s1 == COMPR_S1_NONE) && (flags_s2 == COMPR_S2_NONE)
        && (flags_s3 == COMPR_S3_NONE))
    {
        return 0;
    }

    //----- some preparations for compression
    s3d_buf = (scanPoint*)&s3d_buf1;
    pbuf1   = (uint8_t*)&buf1;

    inputHeader      = s3d_in_out;
    uint32_t lenIn   = s3d_in_out->pointNum * sizeof(scan_point);
    uint32_t nPoints = s3d_in_out->pointNum;
    compr_Tool.compr_writeStatistics(0, lenIn);
    uint8_t *pOut    = (uint8_t*)s3d_in_out->point;
    uint8_t *pIn     = (uint8_t*)s3d_buf;
    memcpy(pIn, pOut, lenIn);

    input = s3d_buf;

    //----- prepare output stream
    compr_bitStream  outStream1;                     //----- output stream of step 1
    outStream1.data  = pOut;
    pOut[0]          = 0x4D;
    pOut[1]          = 0x31;
    pOut[14]         = 0x00;
    outStream1.aByte = 2;
    outStream1.aBit  = 0;
    compr_Tool.compr_writeByteStream(&outStream1, flags_s1, sizeof(flags_s1));
    compr_Tool.compr_writeByteStream(&outStream1, compr_Tool.stats.s_step0, sizeof(compr_Tool.stats.s_step0));
    outStream1.aByte = 14;

    //----- execute compression functions of step 1
    if ((flags_s1 & 0xFF000000) == COMPR_S1_REMOVE_INVALID)
        nPoints = compr_step1_remove_invalid(nPoints);

    switch (flags_s1 & 0x000000FF) {
    case COMPR_S1_REDUCE_BYTES:
        compr_step1_searchMinMaxValues(nPoints);
        compr_step1_reduceBytesPerSymbol(&outStream1, nPoints);
        break;
    case COMPR_S1_REDUCE_BYTES_222110:
        compr_step1_reduceBytesPerSymbol222110(&outStream1, nPoints);
        break;
    case COMPR_S1_REDUCE_BYTES_RESORT:
        compr_bitStream outStream1a;
        outStream1a.data  = pbuf1;
        outStream1a.aByte = 0;
        outStream1a.aBit  = 0;
        compr_bitStream inStream1a;
        inStream1a.data   = pbuf1;
        inStream1a.aByte  = 0;
        inStream1a.aBit   = 0;
        compr_step1_reduceBytesPerSymbol222110(&outStream1a, nPoints);
        compr_step1_resortBytesByColumns(&inStream1a, &outStream1, 8, s3d_in_out->pointNum);
        break;
    case COMPR_S1_REDUCE_BITS:
        compr_step1_searchMinMaxValues(nPoints);
        compr_step1_reduceBitsPerSymbol(&outStream1, nPoints);
        break;
    case COMPR_S1_DELTA_RLE:
        compr_step1_searchMinMaxValues(nPoints);
        compr_step1_delta_rle(&outStream1, s3d_in_out->pointNum);
        break;
    case COMPR_S1_NONE:
        for (uint32_t i=0; i<nPoints; i++) {
            compr_Tool.compr_writeByteStream(&outStream1, input[i].x, 4);
            compr_Tool.compr_writeByteStream(&outStream1, input[i].y, 4);
            compr_Tool.compr_writeByteStream(&outStream1, input[i].z, 4);
            compr_Tool.compr_writeByteStream(&outStream1, input[i].t, 4);
            compr_Tool.compr_writeByteStream(&outStream1, input[i].s, 2);
            compr_Tool.compr_writeByteStream(&outStream1, input[i].i, 2);
        }
        break;
    }


    //----- closing operations of step 1
    uint32_t size1 = (outStream1.aBit==0)?outStream1.aByte:outStream1.aByte+1;
    compr_Tool.compr_writeStatistics(1, size1);
    //----- amend header
    outStream1.aByte = 10; outStream1.aBit = 0;
    compr_Tool.compr_writeByteStream(&outStream1, compr_Tool.stats.s_step1, sizeof(compr_Tool.stats.s_step1));

    //----- call step 2 and 3 of compression (in CompressTool)
    if ((flags_s2 != COMPR_S2_NONE) || (flags_s3 != COMPR_S3_NONE))
        compr_Tool.Compress(pOut, flags_s2, flags_s3);
    else {
        compr_Tool.compr_writeStatistics(2, size1);
        compr_Tool.compr_writeStatistics(3, size1);
    }

    //----- closing operations of step 1-3
    inputHeader->compressed = compr_Tool.stats.s_step3;

    //----- amend header
    outStream1.aByte = 10; outStream1.aBit = 0;
    compr_Tool.compr_writeByteStream(&outStream1, compr_Tool.stats.s_step3, sizeof(compr_Tool.stats.s_step3));

#ifdef COMPR_DEBUG
//  GDOS_PRINT(compr_Tool.compr_getStatisticsTable());
    GDOS_PRINT(compr_Tool.compr_getStatisticsStr());
#endif

    return compr_Tool.stats.s_step3;
}

//----- ----------------------------------------------------------------------------------------------------
//----- decompression functions step 1
//----- ----------------------------------------------------------------------------------------------------

//----- Search min and max values of data rows, then calculate
//----- needed bit and byte sizes and write them to MinMax
void Scan3dCompressTool::compr_step1_searchMinMaxValues(uint32_t nPoints) {
    MinMax.xMin = input[0].x;
    MinMax.xMax = input[0].x;
    MinMax.yMin = input[0].y;
    MinMax.yMax = input[0].y;
    MinMax.zMin = input[0].z;
    MinMax.zMax = input[0].z;
    MinMax.tMin = input[0].t;
    MinMax.tMax = input[0].t;
    MinMax.sMin = input[0].s;
    MinMax.sMax = input[0].s;
    MinMax.iMin = input[0].i;
    MinMax.iMax = input[0].i;
    for (uint32_t i=1; i<nPoints; i++) {
        if (input[i].x < MinMax.xMin) MinMax.xMin = input[i].x;
        if (input[i].x > MinMax.xMax) MinMax.xMax = input[i].x;
        if (input[i].y < MinMax.yMin) MinMax.yMin = input[i].y;
        if (input[i].y > MinMax.yMax) MinMax.yMax = input[i].y;
        if (input[i].z < MinMax.zMin) MinMax.zMin = input[i].z;
        if (input[i].z > MinMax.zMax) MinMax.zMax = input[i].z;
        if (input[i].t < MinMax.tMin) MinMax.tMin = input[i].t;
        if (input[i].t > MinMax.tMax) MinMax.tMax = input[i].t;
        if (input[i].s < MinMax.sMin) MinMax.sMin = input[i].s;
        if (input[i].s > MinMax.sMax) MinMax.sMax = input[i].s;
        if (input[i].i < MinMax.iMin) MinMax.iMin = input[i].i;
        if (input[i].i > MinMax.iMax) MinMax.iMax = input[i].i;
    }
    MinMax.xDelta = MinMax.xMax - MinMax.xMin + 1;
    MinMax.yDelta = MinMax.yMax - MinMax.yMin + 1;
    MinMax.zDelta = MinMax.zMax - MinMax.zMin + 1;
    MinMax.tDelta = MinMax.tMax - MinMax.tMin + 1;
    MinMax.sDelta = MinMax.sMax - MinMax.sMin + 1;
    MinMax.iDelta = MinMax.iMax - MinMax.iMin + 1;

//----- calculate needed bits per symbol

    MinMax.xBits = (int32_t)(log((double)MinMax.xDelta)/log(2.0));
    MinMax.yBits = (int32_t)(log((double)MinMax.yDelta)/log(2.0));
    MinMax.zBits = (int32_t)(log((double)MinMax.zDelta)/log(2.0));
    MinMax.tBits = (int32_t)(log((double)MinMax.tDelta)/log(2.0));
    MinMax.sBits = (int32_t)(log((double)MinMax.sDelta)/log(2.0));
   MinMax.iBits = (int32_t)(log((double)MinMax.iDelta)/log(2.0));

    MinMax.xBits = (MinMax.xBits==0)?0:MinMax.xBits+1;
    MinMax.yBits = (MinMax.yBits==0)?0:MinMax.yBits+1;
    MinMax.zBits = (MinMax.zBits==0)?0:MinMax.zBits+1;
    MinMax.tBits = (MinMax.tBits==0)?0:MinMax.tBits+1;
    MinMax.sBits = (MinMax.sBits==0)?0:MinMax.sBits+1;
    MinMax.iBits = (MinMax.iBits==0)?0:MinMax.iBits+1;

//----- calculate needed bytes per symbol
    MinMax.xBytes = (MinMax.xBits/8)+1;
    if ((MinMax.xBits % 8) == 0) MinMax.xBytes--;
    MinMax.yBytes = (MinMax.yBits/8)+1;
    if ((MinMax.yBits % 8) == 0) MinMax.yBytes--;
    MinMax.zBytes = (MinMax.zBits/8)+1;
    if ((MinMax.zBits % 8) == 0) MinMax.zBytes--;
    MinMax.tBytes = (MinMax.tBits/8)+1;
    if ((MinMax.tBits % 8) == 0) MinMax.tBytes--;
    MinMax.sBytes = (MinMax.sBits/8)+1;
    if ((MinMax.sBits % 8) == 0) MinMax.sBytes--;
    MinMax.iBytes = (MinMax.iBits/8)+1;
    if ((MinMax.iBits % 8) == 0) MinMax.iBytes--;

//----- debug output if COMPR_DEBUG is set
#ifdef COMPR_DEBUG
    GDOS_PRINT("xMin: %d, xMax: %d, xDelta: %d, xBits: %d\n", MinMax.xMin, MinMax.xMax, MinMax.xDelta, MinMax.xBits);
    GDOS_PRINT("yMin: %d, yMax: %d, yDelta: %d, yBits: %d\n", MinMax.yMin, MinMax.yMax, MinMax.yDelta, MinMax.yBits);
    GDOS_PRINT("zMin: %d, zMax: %d, zDelta: %d, zBits: %d\n", MinMax.zMin, MinMax.zMax, MinMax.zDelta, MinMax.zBits);
    GDOS_PRINT("tMin: %d, tMax: %d, tDelta: %d, tBits: %d\n", MinMax.tMin, MinMax.tMax, MinMax.tDelta, MinMax.tBits);
    GDOS_PRINT("sMin: %d, sMax: %d, sDelta: %d, sBits: %d\n", MinMax.sMin, MinMax.sMax, MinMax.sDelta, MinMax.sBits);
    GDOS_PRINT("iMin: %d, iMax: %d, iDelta: %d, iBits: %d\n", MinMax.iMin, MinMax.iMax, MinMax.iDelta, MinMax.iBits);
#endif
}

//----- Reduce input data rows to min bit sizes (as stored in MinMax)
void Scan3dCompressTool::compr_step1_reduceBitsPerSymbol(compr_bitStream *out, uint32_t nPoints) {
    //----- write min values as header into the stream
    compr_Tool.compr_writeByteStream(out, -MinMax.xMin, 4);
    compr_Tool.compr_writeByteStream(out, -MinMax.yMin, 4);
    compr_Tool.compr_writeByteStream(out, -MinMax.zMin, 4);
    compr_Tool.compr_writeByteStream(out, -MinMax.tMin, 4);
    compr_Tool.compr_writeByteStream(out, -MinMax.sMin, 2);
    compr_Tool.compr_writeByteStream(out, -MinMax.iMin, 2);
    //----- write bit sizes values as header into the stream
    compr_Tool.compr_writeBitStream(out, MinMax.xBits, 5);
    compr_Tool.compr_writeBitStream(out, MinMax.yBits, 5);
    compr_Tool.compr_writeBitStream(out, MinMax.zBits, 5);
    compr_Tool.compr_writeBitStream(out, MinMax.tBits, 5);
    compr_Tool.compr_writeBitStream(out, MinMax.sBits, 4);
    compr_Tool.compr_writeBitStream(out, MinMax.iBits, 4);
    compr_Tool.compr_writeBitStream(out, 0, 4); //----- stuff bits for byte border
    //----- write reduced values into the stream
    for (uint32_t i=0; i<nPoints; i++) {
        compr_Tool.compr_writeBitStream(out, input[i].x - MinMax.xMin, MinMax.xBits);
        compr_Tool.compr_writeBitStream(out, input[i].y - MinMax.yMin, MinMax.yBits);
        compr_Tool.compr_writeBitStream(out, input[i].z - MinMax.zMin, MinMax.zBits);
        compr_Tool.compr_writeBitStream(out, input[i].t - MinMax.tMin, MinMax.tBits);
        compr_Tool.compr_writeBitStream(out, input[i].s - MinMax.sMin, MinMax.sBits);
        compr_Tool.compr_writeBitStream(out, input[i].i - MinMax.iMin, MinMax.iBits);
    }
}

//----- reduce input data rows to min byte sizes (as stored in MinMax)
void Scan3dCompressTool::compr_step1_reduceBytesPerSymbol(compr_bitStream *out, uint32_t nPoints) {
    //----- write min values as header into the stream
    compr_Tool.compr_writeByteStream(out, -MinMax.xMin, 4);
    compr_Tool.compr_writeByteStream(out, -MinMax.yMin, 4);
    compr_Tool.compr_writeByteStream(out, -MinMax.zMin, 4);
    compr_Tool.compr_writeByteStream(out, -MinMax.tMin, 4);
    compr_Tool.compr_writeByteStream(out, -MinMax.sMin, 2);
    compr_Tool.compr_writeByteStream(out, -MinMax.iMin, 2);
    //----- write bit sizes values as header into the stream
    compr_Tool.compr_writeBitStream(out, MinMax.xBytes, 3);
    compr_Tool.compr_writeBitStream(out, MinMax.yBytes, 3);
    compr_Tool.compr_writeBitStream(out, MinMax.zBytes, 3);
    compr_Tool.compr_writeBitStream(out, MinMax.tBytes, 3);
    compr_Tool.compr_writeBitStream(out, MinMax.sBytes, 2);
    compr_Tool.compr_writeBitStream(out, MinMax.iBytes, 2);
    //----- write reduced values into the stream
    for (uint32_t i=0; i<nPoints; i++) {
        compr_Tool.compr_writeByteStream(out, input[i].x - MinMax.xMin, MinMax.xBytes);
        compr_Tool.compr_writeByteStream(out, input[i].y - MinMax.yMin, MinMax.yBytes);
        compr_Tool.compr_writeByteStream(out, input[i].z - MinMax.zMin, MinMax.zBytes);
        compr_Tool.compr_writeByteStream(out, input[i].t - MinMax.tMin, MinMax.tBytes);
        compr_Tool.compr_writeByteStream(out, input[i].s - MinMax.sMin, MinMax.sBytes);
        compr_Tool.compr_writeByteStream(out, input[i].i - MinMax.iMin, MinMax.iBytes);
    }
}

//----- Reduce input data rows to lower byte sizes
//----- These are not calculated as in compr_step1_reduceBytesPerSymbol(),
//----- but x, y, z = 2 bytes, type, segment = 1 byte and intensity = 0 byte
void Scan3dCompressTool::compr_step1_reduceBytesPerSymbol222110(compr_bitStream *out, uint32_t nPoints) {
    for (uint32_t i=0; i<nPoints; i++) {
        compr_Tool.compr_writeByteStream(out, input[i].x, 2);
        compr_Tool.compr_writeByteStream(out, input[i].y, 2);
        compr_Tool.compr_writeByteStream(out, input[i].z, 2);
        compr_Tool.compr_writeByteStream(out, input[i].t, 1);
        compr_Tool.compr_writeByteStream(out, input[i].s, 1);
    }
}

//----- resorts the data byte-by-byte in columns
void Scan3dCompressTool::compr_step1_resortBytesByColumns(compr_bitStream *in, compr_bitStream *out, uint32_t columns, uint32_t nPoints) {
    uint32_t i, j;
    uint32_t nc = columns*nPoints;
    for (i=0; i<nPoints; i++) {
        for (j=0; j<nc; j+=nPoints) {
            out->data[out->aByte+i+j]   = in->data[in->aByte++];
        }
    }
    out->aByte+=nc;
}

//----- Codes x, y and z fields with delta-coding, type and segment with RLE
//----- Intensity is reduced to minimum byte size
uint32_t Scan3dCompressTool::compr_step1_delta_rle(compr_bitStream *out, uint32_t n) {
    uint32_t i, j, d;
    int32_t a, l;
    //----- write min values as header into the stream
    compr_Tool.compr_writeByteStream(out, -MinMax.xMin, 4);
    compr_Tool.compr_writeByteStream(out, -MinMax.yMin, 4);
    compr_Tool.compr_writeByteStream(out, -MinMax.zMin, 4);
    compr_Tool.compr_writeByteStream(out, -MinMax.tMin, 4);
    compr_Tool.compr_writeByteStream(out, -MinMax.sMin, 2);
    compr_Tool.compr_writeByteStream(out, -MinMax.iMin, 2);
    //----- write byte sizes values as header into the stream
    compr_Tool.compr_writeBitStream(out, MinMax.xBytes, 3);
    compr_Tool.compr_writeBitStream(out, MinMax.yBytes, 3);
    compr_Tool.compr_writeBitStream(out, MinMax.zBytes, 3);
    compr_Tool.compr_writeBitStream(out, MinMax.tBytes, 3);
    compr_Tool.compr_writeBitStream(out, MinMax.sBytes, 2);
    compr_Tool.compr_writeBitStream(out, MinMax.iBytes, 2);

    //----- delta coding for x
    l = input[0].x;
    compr_Tool.compr_writeByteStream(out, l-MinMax.xMin, MinMax.xBytes);
    for (i = 1; i<n; i++) {
        a = input[i].x;
        d = a - l + 128;
        if (d > 254) {
            compr_Tool.compr_writeByteStream(out, 255, 1);
            compr_Tool.compr_writeByteStream(out, a-MinMax.xMin, MinMax.xBytes);
        } else compr_Tool.compr_writeByteStream(out, d, 1);
        l = a;
    }
    //----- delta coding for y
    l = input[0].y;
    compr_Tool.compr_writeByteStream(out, l-MinMax.yMin, MinMax.yBytes);
    for (i = 1; i<n; i++) {
        a = input[i].y;
        d = a - l + 128;
        if (d > 254) {
            compr_Tool.compr_writeByteStream(out, 255, 1);
            compr_Tool.compr_writeByteStream(out, a-MinMax.yMin, MinMax.yBytes);
        } else compr_Tool.compr_writeByteStream(out, d, 1);
        l = a;
    }
    //----- delta coding for z
    l = input[0].z;
    compr_Tool.compr_writeByteStream(out, l-MinMax.zMin, MinMax.zBytes);
    for (i = 1; i<n; i++) {
        a = input[i].z;
        d = a - l + 128;
        if (d > 254) {
            compr_Tool.compr_writeByteStream(out, 255, 1);
            compr_Tool.compr_writeByteStream(out, a-MinMax.zMin, MinMax.zBytes);
        } else compr_Tool.compr_writeByteStream(out, d, 1);
        l = a;
    }
    //----- RLE coding for type
    i = 0;
    while (i<n) {
        l = input[i++].t;
        compr_Tool.compr_writeByteStream(out, l, MinMax.tBytes);
        d = 1;
        while ((input[i].t == l) && (d<256) && (i<n)) { i++; d++; }
        compr_Tool.compr_writeByteStream(out, d, 1);
    }
    //----- RLE coding for segment
    if (MinMax.sBytes > 0) {
        i = 0;
        while (i<n) {
            l = input[i++].s;
            compr_Tool.compr_writeByteStream(out, l, MinMax.sBytes);
            d = 1;
            while ((input[i].s == l) && (d<256) && (i<n)) { i++; d++; }
            compr_Tool.compr_writeByteStream(out, d, 1);
        }
    }
    //----- reduce intensity to minimum byte size
    if (MinMax.iBytes > 0) {
        for (j = 0; j<n; j++)
            compr_Tool.compr_writeByteStream(out, input[j].i - MinMax.iMin, MinMax.iBytes);
    }
    return out->aByte;
}

//----- Removes all points that are tagged with SCAN_POINT_TYPE_INVALID
//----- to keep the information, which point belongs to which scanline,
//----- this is done for each scanline. The amount of invalid points in
//----- each scanline is stored in the z-field of the last point of each line.
uint32_t Scan3dCompressTool::compr_step1_remove_invalid(uint32_t nPoints) {
    uint32_t i = 0;
    uint32_t iOut = 0;
    uint32_t k, n;
    while (i < nPoints) {
        n=0;
        k=0;
        while ((k<(uint32_t)inputHeader->scanPointNum) && i<nPoints) {
            if (!(input[i].t && SCAN_POINT_TYPE_INVALID)) {
                input[iOut].x = input[i].x;
                input[iOut].y = input[i].y;
                input[iOut].z = input[i].z;
                input[iOut].t = input[i].t;
                input[iOut].s = input[i].s;
                input[iOut].i = input[i].i;
                iOut++;
            } else n++;
            i++; k++;
        }
        if (n<(uint32_t)inputHeader->scanPointNum) {  //----- there are invalid points in this scanline
            input[iOut].x = 0;
            input[iOut].y = 0;
            input[iOut].z = n;              //----- number of invalid points
            input[iOut].t = SCAN_POINT_TYPE_INVALID;
            input[iOut].s = 0;
            input[iOut].i = 0;
            iOut++;
        }
    }
    return iOut;
}

#ifdef COMPR_UNCOMPR
//----- ----------------------------------------------------------------------------------------------------
//----- main decompression function
//----- ----------------------------------------------------------------------------------------------------
uint32_t Scan3dCompressTool::Decompress(scan3d_data *s3d_in_out) {
    // return if data is not compressed
    if (s3d_in_out->compressed == 0)
    {
        return 0;
    }

    //----- some preparations for decompression
    s3d_buf         = (scanPoint*)&s3d_buf1;
    output          = (scanPoint*)s3d_in_out->point;
    outputHeader    = s3d_in_out;
    uint8_t *pOut   = (uint8_t*)s3d_in_out->point;
    uint8_t *pIn    = (uint8_t*)s3d_buf;

    //----- prepare input stream
    compr_bitStream inStream1;
    inStream1.data  = pOut;
    inStream1.aByte = 0;
    inStream1.aBit  = 0;

    //----- call step 2 and 3 of decompression (in CompressTool)
    if (!((pOut[0] == 'M') && (pOut[1] == '1')))
        compr_Tool.Decompress(pOut);
    else {
        compr_Tool.uncompr_writeStatistics(3);
        compr_Tool.uncompr_writeStatistics(2);
        compr_Tool.uncompr_writeStatistics(1);
    }

    //----- evaluate header: Magic value
    if (compr_Tool.compr_readByteStream(&inStream1, 2) != 0x4D31) {
        GDOS_PRINT("Fehler: Die komprimierten Daten der 1. Stufe haben keinen gueltigen Header!\n");
        return 1;
    }
    uint32_t flags_s1   = compr_Tool.compr_readByteStream(&inStream1, sizeof(flags_s1));
    uint32_t lenOut     = compr_Tool.compr_readByteStream(&inStream1, sizeof(lenOut));
    uint32_t lenIn      = compr_Tool.compr_readByteStream(&inStream1, sizeof(lenIn));
    uint32_t aLen       = 0;
    uint32_t nPoints    = lenOut/20;
    memcpy(pIn, pOut, lenIn);
    inStream1.data     = pIn;

    pbuf1   = (uint8_t*)&buf1;

    //----- execute decompression functions of step 1
    switch (flags_s1 & 0x000000FF) {
    case COMPR_S1_REDUCE_BYTES:
        aLen = uncompr_step1_reduceBytesPerSymbol(&inStream1, nPoints);
        break;
    case COMPR_S1_REDUCE_BYTES_222110:
        uncompr_step1_reduceBytesPerSymbol222110(&inStream1, nPoints);
        break;
    case COMPR_S1_REDUCE_BYTES_RESORT:
        compr_bitStream inStream1a;
        inStream1a.data  = pbuf1;
        inStream1a.aByte = 0;
        inStream1a.aBit  = 0;
        uncompr_step1_resortBytesByColumns(&inStream1, &inStream1a, s3d_in_out->pointNum, 8);
        inStream1a.aByte = 0;
        inStream1a.aBit  = 0;
        uncompr_step1_reduceBytesPerSymbol222110(&inStream1a, nPoints);
        break;
    case COMPR_S1_REDUCE_BITS:
        aLen = uncompr_step1_reduceBitsPerSymbol(&inStream1, nPoints);
        break;
    case COMPR_S1_DELTA_RLE:
        uncompr_step1_delta_rle(&inStream1, s3d_in_out->pointNum);
        break;
    case COMPR_S1_NONE:
        for (uint32_t i=0; i<lenOut/20; i++) {
            output[i].x = compr_Tool.compr_readByteStream(&inStream1, 4);
            output[i].y = compr_Tool.compr_readByteStream(&inStream1, 4);
            output[i].z = compr_Tool.compr_readByteStream(&inStream1, 4);
            output[i].t = compr_Tool.compr_readByteStream(&inStream1, 4);
            output[i].s = compr_Tool.compr_readByteStream(&inStream1, 2);
            output[i].i = compr_Tool.compr_readByteStream(&inStream1, 2);
        }
        break;
    }
    if ((flags_s1 & 0xFF000000) == COMPR_S1_REMOVE_INVALID)
        uncompr_step1_remove_invalid(lenIn/sizeof(scanPoint), lenOut/sizeof(scanPoint));

    outputHeader->compressed = 0;
    compr_Tool.uncompr_writeStatistics(0);
#ifdef COMPR_DEBUG
//  GDOS_PRINT(compr_Tool.uncompr_getStatisticsTable());
    GDOS_PRINT(compr_Tool.uncompr_getStatisticsStr());
#endif
    return 0;
}

//----- ----------------------------------------------------------------------------------------------------
//----- decompression functions step 1
//----- ----------------------------------------------------------------------------------------------------

//----- Expand data that was compressed with compr_step1_reduceBitsPerSymbol()
uint32_t Scan3dCompressTool::uncompr_step1_reduceBitsPerSymbol(compr_bitStream *in, uint32_t nPoints) {
    //----- read min values from the stream
    MinMax.xMin = compr_Tool.compr_readByteStream(in, 4);
    MinMax.yMin = compr_Tool.compr_readByteStream(in, 4);
    MinMax.zMin = compr_Tool.compr_readByteStream(in, 4);
    MinMax.tMin = compr_Tool.compr_readByteStream(in, 4);
    MinMax.sMin = compr_Tool.compr_readByteStream(in, 2);
    MinMax.iMin = compr_Tool.compr_readByteStream(in, 2);
    //----- read bit sizes from the stream
    MinMax.xBits = compr_Tool.compr_readBitStream(in, 5);
    MinMax.yBits = compr_Tool.compr_readBitStream(in, 5);
    MinMax.zBits = compr_Tool.compr_readBitStream(in, 5);
    MinMax.tBits = compr_Tool.compr_readBitStream(in, 5);
    MinMax.sBits = compr_Tool.compr_readBitStream(in, 4);
    MinMax.iBits = compr_Tool.compr_readBitStream(in, 4);
    compr_Tool.compr_readBitStream(in, 4); //----- stuff bits for byte border
    //----- read values from the stream and extend to 32/16 bits
    for (uint32_t i=0; i<nPoints; i++) {
        output[i].x = compr_Tool.compr_readBitStream(in, MinMax.xBits) - MinMax.xMin;
        output[i].y = compr_Tool.compr_readBitStream(in, MinMax.yBits) - MinMax.yMin;
        output[i].z = compr_Tool.compr_readBitStream(in, MinMax.zBits) - MinMax.zMin;
        output[i].t = compr_Tool.compr_readBitStream(in, MinMax.tBits) - MinMax.tMin;
        output[i].s = compr_Tool.compr_readBitStream(in, MinMax.sBits) - MinMax.sMin;
        output[i].i = compr_Tool.compr_readBitStream(in, MinMax.iBits) - MinMax.iMin;
    }
    return nPoints*20;
}

//----- Expand data that was compressed with compr_step1_reduceBytesPerSymbol()
uint32_t Scan3dCompressTool::uncompr_step1_reduceBytesPerSymbol(compr_bitStream *in, uint32_t nPoints) {
    //----- read min values from the stream
    MinMax.xMin = compr_Tool.compr_readByteStream(in, 4);
    MinMax.yMin = compr_Tool.compr_readByteStream(in, 4);
    MinMax.zMin = compr_Tool.compr_readByteStream(in, 4);
    MinMax.tMin = compr_Tool.compr_readByteStream(in, 4);
    MinMax.sMin = compr_Tool.compr_readByteStream(in, 2);
    MinMax.iMin = compr_Tool.compr_readByteStream(in, 2);
    //----- read byte sizes from the stream
    MinMax.xBytes = compr_Tool.compr_readBitStream(in, 3);
    MinMax.yBytes = compr_Tool.compr_readBitStream(in, 3);
    MinMax.zBytes = compr_Tool.compr_readBitStream(in, 3);
    MinMax.tBytes = compr_Tool.compr_readBitStream(in, 3);
    MinMax.sBytes = compr_Tool.compr_readBitStream(in, 2);
    MinMax.iBytes = compr_Tool.compr_readBitStream(in, 2);
    //----- read values from the stream and extend to 4/2 bytes
    for (uint32_t i=0; i<nPoints; i++) {
        output[i].x = compr_Tool.compr_readByteStream(in, MinMax.xBytes) - MinMax.xMin;
        output[i].y = compr_Tool.compr_readByteStream(in, MinMax.yBytes) - MinMax.yMin;
        output[i].z = compr_Tool.compr_readByteStream(in, MinMax.zBytes) - MinMax.zMin;
        output[i].t = compr_Tool.compr_readByteStream(in, MinMax.tBytes) - MinMax.tMin;
        output[i].s = compr_Tool.compr_readByteStream(in, MinMax.sBytes) - MinMax.sMin;
        output[i].i = compr_Tool.compr_readByteStream(in, MinMax.iBytes) - MinMax.iMin;
    }
    return nPoints*20;
}

//----- Expand data that was compressed with compr_step1_reduceBytesPerSymbol222110()
uint32_t Scan3dCompressTool::uncompr_step1_reduceBytesPerSymbol222110(compr_bitStream *in, uint32_t nPoints) {
    for (uint32_t i=0; i<nPoints; i++) {
        output[i].x = compr_Tool.compr_readByteStream(in, 2);
        output[i].y = compr_Tool.compr_readByteStream(in, 2);
        output[i].z = compr_Tool.compr_readByteStream(in, 2);
        output[i].t = compr_Tool.compr_readByteStream(in, 1);
        output[i].s = compr_Tool.compr_readByteStream(in, 1);
        output[i].i = 0;
        if (output[i].x >= 0x8000) output[i].x += 0xFFFF0000;
        if (output[i].y >= 0x8000) output[i].y += 0xFFFF0000;
        if (output[i].z >= 0x8000) output[i].z += 0xFFFF0000;
    }
    return nPoints*20;
}

//----- Expand data that was transformed with compr_step1_resortBytesByColumns()
void Scan3dCompressTool::uncompr_step1_resortBytesByColumns(compr_bitStream *in, compr_bitStream *out, uint32_t n, uint32_t columns) {
    uint32_t i, j;
    uint32_t nc = columns*n;
    for (i=0; i<n; i++) {
        for (j=0; j<nc; j+=n) {
            out->data[out->aByte++] = in->data[14+i+j];
        }
    }
}

//----- Expand data that was compressed with compr_step1_delta_rle()
uint32_t Scan3dCompressTool::uncompr_step1_delta_rle(compr_bitStream *in, uint32_t n) {
    int32_t a, l;
    uint32_t i, j;
    //----- read min values from the stream
    MinMax.xMin = compr_Tool.compr_readByteStream(in, 4);
    MinMax.yMin = compr_Tool.compr_readByteStream(in, 4);
    MinMax.zMin = compr_Tool.compr_readByteStream(in, 4);
    MinMax.tMin = compr_Tool.compr_readByteStream(in, 4);
    MinMax.sMin = compr_Tool.compr_readByteStream(in, 2);
    MinMax.iMin = compr_Tool.compr_readByteStream(in, 2);
    //----- read byte sizes from the stream
    MinMax.xBytes = compr_Tool.compr_readBitStream(in, 3);
    MinMax.yBytes = compr_Tool.compr_readBitStream(in, 3);
    MinMax.zBytes = compr_Tool.compr_readBitStream(in, 3);
    MinMax.tBytes = compr_Tool.compr_readBitStream(in, 3);
    MinMax.sBytes = compr_Tool.compr_readBitStream(in, 2);
    MinMax.iBytes = compr_Tool.compr_readBitStream(in, 2);
    //----- delta decode x
    l = compr_Tool.compr_readByteStream(in, MinMax.xBytes) - MinMax.xMin;
    output[0].x = l;
    for (i=1; i<n; i++) {
        a = compr_Tool.compr_readByteStream(in, 1);
        if (a == 255) a = compr_Tool.compr_readByteStream(in, MinMax.xBytes) - MinMax.xMin;
        else a = l+a-128;
        output[i].x = a;
        l=a;
    }
    //----- delta decode y
    l = compr_Tool.compr_readByteStream(in, MinMax.yBytes) - MinMax.yMin;
    output[0].y = l;
    for (i=1; i<n; i++) {
        a = compr_Tool.compr_readByteStream(in, 1);
        if (a == 255) a = compr_Tool.compr_readByteStream(in, MinMax.yBytes) - MinMax.yMin;
        else a = l+a-128;
        output[i].y = a;
        l=a;
    }
    //----- delta decode z
    l = compr_Tool.compr_readByteStream(in, MinMax.zBytes) - MinMax.zMin;
    output[0].z = l;
    for (i=1; i<n; i++) {
        a = compr_Tool.compr_readByteStream(in, 1);
        if (a == 255) a = compr_Tool.compr_readByteStream(in, MinMax.zBytes) - MinMax.zMin;
        else a = l+a-128;
        output[i].z = a;
        l=a;
    }
    //----- RLE decode type
    if (MinMax.tBytes > 0) {
        j = 0;
        while (j < n) {
            a = compr_Tool.compr_readByteStream(in, MinMax.tBytes);
            l = compr_Tool.compr_readByteStream(in, 1);
            for (i=0; i<l; i++) output[j++].t = a;
        }
    } else for (i=0; i<n; i++) output[i].t = 0;
    //----- RLE decode segment
    if (MinMax.sBytes > 0) {
        j = 0;
        while (j < n) {
            a = compr_Tool.compr_readByteStream(in, MinMax.sBytes);
            l = compr_Tool.compr_readByteStream(in, 1);
            for (i=0; i<l; i++) output[j++].s = a;
        }
    } else for (i=0; i<n; i++) output[i].s = 0;
    //----- expand intensity to 2 bytes
    if (MinMax.iBytes > 0) {
        for (j=0; j<n; j++)
            output[j].i = compr_Tool.compr_readByteStream(in, MinMax.iBytes) - MinMax.iMin;
    } else for (j=0; j<n; j++) output[j].i = 0;
    return n*20;
}

//----- Expand data that was compressed with compr_step1_remove_invalid()
//----- Invalid points are filled with zeros, and SCAN_POINT_TYPE_INVALID in the type-field
uint32_t Scan3dCompressTool::uncompr_step1_remove_invalid(uint32_t nIn, uint32_t nOut) {
    uint32_t pOut = nOut-1;
    uint32_t pIn  = nIn-1;
    uint32_t k;
    if (nIn < nOut)
        while (pIn>0) {
            if ((output[pIn].x == 0) && (output[pIn].y == 0) && (output[pIn].t == SCAN_POINT_TYPE_INVALID)) {
                for (k = 0; k<output[pIn].z; k++) {
                    output[pOut].x = 0;
                    output[pOut].y = 0;
                    output[pOut].z = 0;
                    output[pOut].t = SCAN_POINT_TYPE_INVALID;
                    output[pOut].s = 0;
                    output[pOut].i = 0;
                    pOut--;
                }
            } else {
                output[pOut].x = output[pIn].x;
                output[pOut].y = output[pIn].y;
                output[pOut].z = output[pIn].z;
                output[pOut].t = output[pIn].t;
                output[pOut].s = output[pIn].s;
                output[pOut].i = output[pIn].i;
                pOut--;
            }
            pIn--;
        }
    return nOut;
}

#endif
/*
//----- header of step 1
00  2   magic value (0x4D31, "M1")
02  4   flags step 1
06  4   length uncompressed
10  4   length compressed
14  ?   compressed data


*/
