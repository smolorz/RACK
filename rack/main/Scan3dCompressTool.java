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
package rack.main;

import rack.main.defines.ScanPoint;
import rack.main.CompressTool;
import java.io.IOException;

public class Scan3dCompressTool {

    public final int COMPR_S1_NONE                = 0x00000000;
    public final int COMPR_S1_REDUCE_BITS         = 0x00000001;
    public final int COMPR_S1_REDUCE_BYTES        = 0x00000002;
    public final int COMPR_S1_REDUCE_BYTES_222110 = 0x00000003;
    public final int COMPR_S1_REDUCE_BYTES_RESORT = 0x00000004;
    public final int COMPR_S1_DELTA_RLE           = 0x00000005;
    public final int COMPR_S1_REMOVE_INVALID      = 0x01000000;
    public final int SCAN_POINT_TYPE_INVALID      = 0x00000010;

    CompressTool decompressor = new CompressTool();
    MinMax       minMax       = new MinMax();      //----- statistics on input data
    ScanPoint[]  output;

    public Scan3dCompressTool() {
    }

    public int Decompress(byte[] inBuf, ScanPoint[] outBuf, int nC, int nP) throws IOException
    {
       byte[] buf1 = new byte[nC];
       output = outBuf;

    //----- call step 2 and 3 of decompression (in CompressTool)
    if (((inBuf[0] == 'M') && (inBuf[1] == '1'))) {
        decompressor.uncompr_writeStatistics(3);
        decompressor.uncompr_writeStatistics(2);
        decompressor.uncompr_writeStatistics(1);
    } else if (decompressor.Decompress(inBuf) == -1) return -1;


    //----- prepare input stream
    CompressTool.Compr_bitStream inStream1 = decompressor.new Compr_bitStream();
    inStream1.data  = inBuf;
    inStream1.aByte = 0;
    inStream1.aBit  = 0;

    //----- evaluate header: Magic value
    if (decompressor.compr_readByteStream(inStream1, 2) != 0x4D31) {
        System.out.println("Fehler: Die komprimierten Daten der  1. Stufe haben keinen gueltigen Header!\n");
        return 1;
    }
    int flags_s1   = decompressor.compr_readByteStream(inStream1, 4);
    int lenOut     = decompressor.compr_readByteStream(inStream1, 4);
    int lenIn      = decompressor.compr_readByteStream(inStream1, 4);
    int nPoints    = lenOut/20;


    //----- execute decompression functions of step 1
    switch (flags_s1 & 0x000000FF) {
    case COMPR_S1_REDUCE_BYTES:
        uncompr_step1_reduceBytesPerSymbol(inStream1, nPoints);
        break;
    case COMPR_S1_REDUCE_BYTES_222110:
        uncompr_step1_reduceBytesPerSymbol222110(inStream1, nPoints);
        break;
    case COMPR_S1_REDUCE_BYTES_RESORT:
        CompressTool.Compr_bitStream inStream1a = decompressor.new Compr_bitStream();
        inStream1a.data  = buf1;
        inStream1a.aByte = 0;
        inStream1a.aBit  = 0;
        uncompr_step1_resortBytesByColumns(inStream1, inStream1a, nP, 8);
        inStream1a.aByte = 0;
        inStream1a.aBit  = 0;
        uncompr_step1_reduceBytesPerSymbol222110(inStream1a, nPoints);
        break;
    case COMPR_S1_REDUCE_BITS:
        uncompr_step1_reduceBitsPerSymbol(inStream1, nPoints);
        break;
    case COMPR_S1_DELTA_RLE:
        uncompr_step1_delta_rle(inStream1, nP);
        break;
    case COMPR_S1_NONE:
        for (int i=0; i<lenOut/ScanPoint.getDataLen(); i++) {
            output[i].x         =        decompressor.compr_readByteStream(inStream1, 4);
            output[i].y         =        decompressor.compr_readByteStream(inStream1, 4);
            output[i].z         =        decompressor.compr_readByteStream(inStream1, 4);
            output[i].type      =        decompressor.compr_readByteStream(inStream1, 4);
            output[i].segment   = (short)decompressor.compr_readByteStream(inStream1, 2);
            output[i].intensity = (short)decompressor.compr_readByteStream(inStream1, 2);
        }
        break;
    }
    if ((flags_s1 & 0xFF000000) == COMPR_S1_REMOVE_INVALID)
    uncompr_step1_remove_invalid(lenIn/ScanPoint.getDataLen(), lenOut/ScanPoint.getDataLen());

    decompressor.uncompr_writeStatistics(0);

//------ enable for debug output as table or single line
//   System.out.println(decompressor.uncompr_getStatisticsTable());
//   System.out.println(decompressor.uncompr_getStatisticsStr());

    return nP * ScanPoint.getDataLen();

    }

    //----- minMax contains min and max values of input data, their ranges
    //----- and the needed amount of bits and bytes
    class MinMax {
        int  xMin,   yMin,   zMin,   tMin,   sMin,   iMin;      //----- min values
        int  xMax,   yMax,   zMax,   tMax,   sMax,   iMax;      //----- max values
        int  xDelta, yDelta, zDelta, tDelta, sDelta, iDelta;    //----- ranges
        int  xBits,  yBits,  zBits,  tBits,  sBits,  iBits;     //----- needed bits
        int  xBytes, yBytes, zBytes, tBytes, sBytes, iBytes;    //----- needed bytes
    }

    //----- Expand data that was compressed with compr_step1_reduceBitsPerSymbol()
    int uncompr_step1_reduceBitsPerSymbol(CompressTool.Compr_bitStream in, int nPoints) {
        //----- read min values from the stream
        minMax.xMin = decompressor.compr_readByteStream(in, 4);
        minMax.yMin = decompressor.compr_readByteStream(in, 4);
        minMax.zMin = decompressor.compr_readByteStream(in, 4);
        minMax.tMin = decompressor.compr_readByteStream(in, 4);
        minMax.sMin = decompressor.compr_readByteStream(in, 2);
        minMax.iMin = decompressor.compr_readByteStream(in, 2);
        //----- read bit sizes from the stream
        minMax.xBits = decompressor.compr_readBitStream(in, 5);
        minMax.yBits = decompressor.compr_readBitStream(in, 5);
        minMax.zBits = decompressor.compr_readBitStream(in, 5);
        minMax.tBits = decompressor.compr_readBitStream(in, 5);
        minMax.sBits = decompressor.compr_readBitStream(in, 4);
        minMax.iBits = decompressor.compr_readBitStream(in, 4);
        decompressor.compr_readBitStream(in, 4); //----- stuff bits for byte border
        //----- read values from the stream and extend to 32/16 bits
        for (int i=0; i<nPoints; i++) {
            output[i].x         =         decompressor.compr_readBitStream(in, minMax.xBits) - minMax.xMin;
            output[i].y         =         decompressor.compr_readBitStream(in, minMax.yBits) - minMax.yMin;
            output[i].z         =         decompressor.compr_readBitStream(in, minMax.zBits) - minMax.zMin;
            output[i].type      =         decompressor.compr_readBitStream(in, minMax.tBits) - minMax.tMin;
            output[i].segment   = (short)(decompressor.compr_readBitStream(in, minMax.sBits) - minMax.sMin);
            output[i].intensity = (short)(decompressor.compr_readBitStream(in, minMax.iBits) - minMax.iMin);
        }
        return nPoints * ScanPoint.getDataLen();
    }
    //----- Expand data that was compressed with compr_step1_reduceBytesPerSymbol()
    int uncompr_step1_reduceBytesPerSymbol(CompressTool.Compr_bitStream in, int nPoints) {
        //----- read min values from the stream
        minMax.xMin = decompressor.compr_readByteStream(in, 4);
        minMax.yMin = decompressor.compr_readByteStream(in, 4);
        minMax.zMin = decompressor.compr_readByteStream(in, 4);
        minMax.tMin = decompressor.compr_readByteStream(in, 4);
        minMax.sMin = decompressor.compr_readByteStream(in, 2);
        minMax.iMin = decompressor.compr_readByteStream(in, 2);
        //----- read byte sizes from the stream
        minMax.xBytes = decompressor.compr_readBitStream(in, 3);
        minMax.yBytes = decompressor.compr_readBitStream(in, 3);
        minMax.zBytes = decompressor.compr_readBitStream(in, 3);
        minMax.tBytes = decompressor.compr_readBitStream(in, 3);
        minMax.sBytes = decompressor.compr_readBitStream(in, 2);
        minMax.iBytes = decompressor.compr_readBitStream(in, 2);
        //----- read values from the stream and extend to 4/2 bytes
        for (int i=0; i<nPoints; i++) {
            output[i].x         =         decompressor.compr_readByteStream(in, minMax.xBytes) - minMax.xMin;
            output[i].y         =         decompressor.compr_readByteStream(in, minMax.yBytes) - minMax.yMin;
            output[i].z         =         decompressor.compr_readByteStream(in, minMax.zBytes) - minMax.zMin;
            output[i].type      =         decompressor.compr_readByteStream(in, minMax.tBytes) - minMax.tMin;
            output[i].segment   = (short)(decompressor.compr_readByteStream(in, minMax.sBytes) - minMax.sMin);
            output[i].intensity = (short)(decompressor.compr_readByteStream(in, minMax.iBytes) - minMax.iMin);
        }
        return nPoints * ScanPoint.getDataLen();
    }

    //----- Expand data that was compressed with compr_step1_reduceBytesPerSymbol222110()
    int uncompr_step1_reduceBytesPerSymbol222110(CompressTool.Compr_bitStream in, int nPoints) {
        for (int i=0; i<nPoints; i++) {
            output[i].x         =         decompressor.compr_readByteStream(in, 2);
            output[i].y         =         decompressor.compr_readByteStream(in, 2);
            output[i].z         =         decompressor.compr_readByteStream(in, 2);
            output[i].type      =         decompressor.compr_readByteStream(in, 1);
            output[i].segment   = (short) decompressor.compr_readByteStream(in, 1);
            output[i].intensity = 0;
            if (output[i].x >= 0x8000) output[i].x += 0xFFFF0000;
            if (output[i].y >= 0x8000) output[i].y += 0xFFFF0000;
            if (output[i].z >= 0x8000) output[i].z += 0xFFFF0000;
        }
        return nPoints * ScanPoint.getDataLen();
    }

    //----- Expand data that was transformed with compr_step1_resortBytesByColumns()
    void uncompr_step1_resortBytesByColumns(CompressTool.Compr_bitStream in, CompressTool.Compr_bitStream out, int n, int columns) {
        int i, j;
        int nc = columns*n;
        for (i=0; i<n; i++) {
            for (j=0; j<nc; j+=n) {
                out.data[out.aByte++] = in.data[14+i+j];
            }
        }
    }

    //----- Expand data that was compressed with compr_step1_delta_rle()
    int uncompr_step1_delta_rle(CompressTool.Compr_bitStream in, int n) {
        int a, l;
        int i, j;
        //----- read min values from the stream
        minMax.xMin = decompressor.compr_readByteStream(in, 4);
        minMax.yMin = decompressor.compr_readByteStream(in, 4);
        minMax.zMin = decompressor.compr_readByteStream(in, 4);
        minMax.tMin = decompressor.compr_readByteStream(in, 4);
        minMax.sMin = decompressor.compr_readByteStream(in, 2);
        minMax.iMin = decompressor.compr_readByteStream(in, 2);
        //----- read byte sizes from the stream
        minMax.xBytes = decompressor.compr_readBitStream(in, 3);
        minMax.yBytes = decompressor.compr_readBitStream(in, 3);
        minMax.zBytes = decompressor.compr_readBitStream(in, 3);
        minMax.tBytes = decompressor.compr_readBitStream(in, 3);
        minMax.sBytes = decompressor.compr_readBitStream(in, 2);
        minMax.iBytes = decompressor.compr_readBitStream(in, 2);

        //----- delta decode x
        l = decompressor.compr_readByteStream(in, minMax.xBytes) - minMax.xMin;
        output[0].x = l;
        for (i=1; i<n; i++) {
            a = decompressor.compr_readByteStream(in, 1);
            if (a == 255) a = decompressor.compr_readByteStream(in, minMax.xBytes) - minMax.xMin;
            else a = l+a-128;
            output[i].x = a;
            l=a;
        }
        //----- delta decode y
        l = decompressor.compr_readByteStream(in, minMax.yBytes) - minMax.yMin;
        output[0].y = l;
        for (i=1; i<n; i++) {
            a = decompressor.compr_readByteStream(in, 1);
            if (a == 255) a = decompressor.compr_readByteStream(in, minMax.yBytes) - minMax.yMin;
            else a = l+a-128;
            output[i].y = a;
            l=a;
        }
        //----- delta decode z
        l = decompressor.compr_readByteStream(in, minMax.zBytes) - minMax.zMin;
        output[0].z = l;
        for (i=1; i<n; i++) {
            a = decompressor.compr_readByteStream(in, 1);
            if (a == 255) a = decompressor.compr_readByteStream(in, minMax.zBytes) - minMax.zMin;
            else a = l+a-128;
            output[i].z = a;
            l=a;
        }
        //----- RLE decode type
        if (minMax.tBytes > 0) {
            j = 0;
            while (j < n) {
                a = decompressor.compr_readByteStream(in, minMax.tBytes);
                l = decompressor.compr_readByteStream(in, 1);
                for (i=0; i<l; i++) output[j++].type = a;
            }
        } else for (i=0; i<n; i++) output[i].type = 0;
        //----- RLE decode segment
        if (minMax.sBytes > 0) {
            j = 0;
            while (j < n) {
                a = decompressor.compr_readByteStream(in, minMax.sBytes);
                l = decompressor.compr_readByteStream(in, 1);
                for (i=0; i<l; i++) output[j++].segment = (short) a;
            }
        } else for (i=0; i<n; i++) output[i].segment = 0;
        //----- expand intensity to 2 bytes
        if (minMax.iBytes > 0) {
            for (j=0; j<n; j++)
                output[j].intensity = (short) (decompressor.compr_readByteStream(in, minMax.iBytes) - minMax.iMin);
        } else for (i=0; i<n; i++) output[i].intensity = 0;
        return n * ScanPoint.getDataLen();
    }

    //----- Expand data that was compressed with compr_step1_remove_invalid()
    //----- Invalid points are filled with zeros, and SCAN_POINT_TYPE_INVALID in the type-field
    int uncompr_step1_remove_invalid(int nIn, int nOut) {
        int pOut = nOut-1;
        int pIn  = nIn-1;
        int k;
        if (nIn < nOut)
            while (pIn>0) {
                if ((output[pIn].x == 0) && (output[pIn].y == 0) && (output[pIn].type == SCAN_POINT_TYPE_INVALID)) {
                    for (k = 0; k<output[pIn].z; k++) {
                        output[pOut].x         = 0;
                        output[pOut].y         = 0;
                        output[pOut].z         = 0;
                        output[pOut].type      = SCAN_POINT_TYPE_INVALID;
                        output[pOut].segment   = 0;
                        output[pOut].intensity = 0;
                        pOut--;
                    }
                } else {
                    output[pOut].x         = output[pIn].x;
                    output[pOut].y         = output[pIn].y;
                    output[pOut].z         = output[pIn].z;
                    output[pOut].type      = output[pIn].type;
                    output[pOut].segment   = output[pIn].segment;
                    output[pOut].intensity = output[pIn].intensity;
                    pOut--;
                }
                pIn--;
            }
        return nOut;
    }


}
