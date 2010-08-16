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

import java.io.IOException;

public class CompressTool {

    public final int   COMPR_S2_NONE        = 0x00;
    public final int   COMPR_S2_LZSS        = 0x21;
    public final int   COMPR_S2_LZW         = 0x22;
    public final int   COMPR_S2_LZWV        = 0x23;
    public final int   COMPR_S2_BWT         = 0x24;
    public final int   COMPR_S2_MTF         = 0x25;
    public final int   COMPR_S2_MTF_LZSS    = 0x26;
    public final int   COMPR_S2_BWT_MTF     = 0x27;
    public final int   COMPR_S3_NONE        = 0x00;
    public final int   COMPR_S3_HUFFMANN    = 0x31;
    public final int   COMPR_S3_AD_HUFFMANN = 0x32;

    public final int   HUFF_EOS             = 256;

    public final int   LZSS_MAX_INDEX_BITS  = 16;   //----- determined optimum: 6
    public final int   LZSS_MAX_LEN_BITS    = 5;    //----- determined optimum: 4
    public final int   LZSS_MAX_WINDOW_SIZE = (1<<LZSS_MAX_INDEX_BITS);
    //----- The following values are used, if no parameters or 0 is given.
    public final int   LZSS_INDEX_BITS      = 6;    //----- determined optimum: 6
    public final int   LZSS_LEN_BITS        = 4;    //----- determined optimum: 4

    public final int   LZSS_EOS             = 0;
    public final int   LZSS_UNUSED          = 0;

    public final int   SCAN3D_POINT_MAX     = 256000;
    public final int   COMPR_MAX_INPUT_SIZE = (int)(SCAN3D_POINT_MAX*20*1.1);



    public Compr_statistics stats = new Compr_statistics();

    //----- for Huffmann coding
    int[]               huff_counters = new int[256];
    Huff_node[]         huff_nodes    = new Huff_node[514];
    Huff_code[]         huff_codes    = new Huff_code[257];

    //----- for MTF
    int[]               mtf_alphabet = new int[256];
    //----- for LZSS
    int                 lzss_cost_benefit;
    int                 lzss_window_size;
    int                 lzss_pre_wnd_size;
    int                 lzss_tree_root;
    Lzss_node[]         lzss_tree = new Lzss_node[LZSS_MAX_WINDOW_SIZE+1];
    byte[]              lzss_window = new byte[LZSS_MAX_WINDOW_SIZE];
    Compr_bitStream     inStream3  = new Compr_bitStream();
    Compr_bitStream     outStream3 = new Compr_bitStream();
    Compr_bitStream     inStream2  = new Compr_bitStream();
    Compr_bitStream     outStream2 = new Compr_bitStream();

    public CompressTool() {
    }

    public int Decompress(byte[] in_out) throws IOException
    {
    //----- ----------------------------------------------------------------------------------------------------
    //----- decompression step 3
    //----- ----------------------------------------------------------------------------------------------------

    //----- prepare input stream
    inStream3.data      = in_out;
    inStream3.aByte     = 0;
    inStream3.aBit      = 0;

    //----- evaluate header: Magic value
    if (compr_readByteStream(inStream3, 2) != 0x4D43) {
        System.out.println("Fehler: Die komprimierten Daten der 3. Stufe haben keinen gueltigen Header!\n");
        return 1;
    }

    int flags_s3 = compr_readByteStream(inStream3, 4);
    int size2    = compr_readByteStream(inStream3, 4);
    int size3    = compr_readByteStream(inStream3, 4);
    int aLen     = 0;
    uncompr_writeStatistics(3);

    //----- prepare output stream
    byte[] buf23        = new byte[COMPR_MAX_INPUT_SIZE];
    outStream3.data     = buf23;
    outStream3.aByte    = 0;
    outStream3.aBit     = 0;


    //----- execute decompression functions of step 3
    switch (flags_s3) {
    case COMPR_S3_HUFFMANN:
        aLen = uncompr_step3_huffmann(inStream3, outStream3, size3);
        break;
    /*
    //----- Adaptive Huffmann coding has not been implemented, because it showed to be slow
    case COMPR_S3_AD_HUFFMANN:
        aLen = uncompr_step3_adhuff(inStream3, outStream3, size3);
        break;
    */
    case COMPR_S3_NONE:
        for (int i=0; i<size2; i++) buf23[i] = in_out[i+14];
        break;
    default:
        System.out.println("Fehler: Die Daten der Stufe 3 sind " +
                    "mit einem nicht untersttzten Verfahren komprimiert." +
                    "Code: " + flags_s3);
        return -1;

    }

    uncompr_writeStatistics(2);
    //----- ----------------------------------------------------------------------------------------------------
    //----- decompression step 2
    //----- ----------------------------------------------------------------------------------------------------

    //----- prepare input stream
    inStream2.data      = buf23;
    inStream2.aByte     = 0;
    inStream2.aBit      = 0;
    //----- prepare output stream
    outStream2.data     = in_out;
    outStream2.aByte    = 0;
    outStream2.aBit     = 0;
    //----- evaluate header: Magic value
    if (compr_readByteStream(inStream2, 2) != 0x4D32) {
        System.out.println("Fehler: Die komprimierten Daten der 2. Stufe haben keinen gueltigen Header!\n");
        return 1;
    }

    int flags_s2 = compr_readByteStream(inStream2, 4);
                   compr_readByteStream(inStream2, 4);
        size2    = compr_readByteStream(inStream2, 4);
        aLen     = 0;
    int aLen1, par1, par2;

      //----- execute decompression functions of step 2
    switch (flags_s2 & 0x000000FF) {
        //----- The BWT has not been implemented, because it showed to be very slow
/*      case COMPR_S2_BWT:
            par1 = ((flags_s2 & 0xFFFFFF00) >> 8);
            if (par1 == 0) par1 = BWT_BLOCKSIZE;
            aLen = uncompr_step2_bwt(inStream2, outStream2, size2, par1);
            break;
        case COMPR_S2_BWT_MTF:
            aLen1 = uncompr_step2_mtf(inStream2, outStream2, size2);
            inStream2.data   = in_out;
            inStream2.aByte  = 0;
            inStream2.aBit   = 0;
            outStream2.data  = buf23;
            outStream2.aByte = 14;
            outStream2.aBit  = 0;
            par1 = ((flags_s2 & 0xFFFFFF00) >> 8);
            if (par1 == 0) par1 = BWT_BLOCKSIZE;
            aLen = uncompr_step2_bwt(inStream2, outStream2, aLen1, par1);
            //memcpy(in_out, &pbuf23[14], aLen);
            for (int i=0; i<aLen; i++) in_out[i] = buf23[i+14];
            break;
*/      case COMPR_S2_MTF:
            aLen = uncompr_step2_mtf(inStream2, outStream2, size2);
            break;
        case COMPR_S2_LZSS:
            par1 = ((flags_s2 & 0xFF000000) >> 24);
            if (par1 == 0) par1 = LZSS_INDEX_BITS;
            par2 = ((flags_s2 & 0x00FF0000) >> 16);
            if (par2 == 0) par2 = LZSS_LEN_BITS;
            aLen = uncompr_step2_lzss(inStream2, outStream2, par1, par2);
            break;
        case COMPR_S2_MTF_LZSS:
            par1 = ((flags_s2 & 0xFF000000) >> 24);
            if (par1 == 0) par1 = LZSS_INDEX_BITS;
            par2 = ((flags_s2 & 0x00FF0000) >> 16);
            if (par2 == 0) par2 = LZSS_LEN_BITS;
            aLen1 = uncompr_step2_lzss(inStream2, outStream2, par1, par2);
            inStream2.data   = in_out;
            inStream2.aByte  = 0;
            inStream2.aBit   = 0;
            outStream2.data  = buf23;
            outStream2.aByte = 14;
            outStream2.aBit  = 0;
            aLen = uncompr_step2_mtf(inStream2, outStream2, aLen1);
            for (int i=0; i<aLen; i++) in_out[i] = buf23[i+14];
            break;
            //----- LZW has not been implemented, because it showed to be ineffient and slow
/*      case COMPR_S2_LZW:
            par1 = ((flags_s2 & 0xFF000000) >> 24);
            if (par1 == 0) par1 = LZW_BITS;
            aLen = uncompr_step2_lzw(inStream2, outStream2, par1);
            break;
        case COMPR_S2_LZWV:
            par1 = ((flags_s2 & 0xFF000000) >> 24);
            if (par1 == 0) par1 = LZWV_MAX_BITS;
            aLen = uncompr_step2_lzwv(inStream2, outStream2, par1);
            break;
*/      case COMPR_S2_NONE:
            for (int i=0; i<size2; i++) in_out[i] = buf23[i+14];
            break;
        default:
            System.out.println("Fehler: Die Daten der Stufe 2 sind " +
                    "mit einem nicht untersttzten Verfahren komprimiert." +
                    "Code: " + flags_s2);
            return -1;
        }
    uncompr_writeStatistics(1);
    return 0;
    }

    public class Compr_bitStream {
        public byte[] data;     //----- pointer to data
        public int aByte;       //----- actual byte
        public int aBit;        //----- actual bit in actual byte
    }

    //----- for Huffmann coding
    class  Huff_node {
        int count;
        int child1;
        int child2;
    }

    class Huff_code {
        int code;
        int len;
    }

    //----- for LZSS
    class Lzss_node {
        int parent;
        int child_small;
        int child_large;
    }

    public class Compr_statistics {
        long        ts_step0, ts_step1,  ts_step2,  ts_step3;   //----- time stamp
        int          t_step0,  t_step1,   t_step2,   t_step3;   //----- time differences
        int          s_step0,  s_step1,   s_step2,   s_step3;   //----- sizes in bytes
        float                spr_step1, spr_step2, spr_step3;   //----- sizes in percentages realtive
        float                spa_step1, spa_step2, spa_step3;   //----- sizes in percentages absolute
    }

    //----- reads one bit from the bitstream [bs]
    public boolean compr_readBit(Compr_bitStream bs) {
        int output = bs.data[bs.aByte];
        output >>= (7-bs.aBit++);
        output &= 1;
        if (bs.aBit >= 8) { bs.aBit = 0; bs.aByte++; }
        return (output==1)?true:false;
    }

    //----- writes the lowest bit from [value] into the bitstream [bs]
    public void compr_writeBit(Compr_bitStream bs, boolean value) {
        int v = value?1:0;
        if (bs.aBit == 0) bs.data[bs.aByte] = 0;
        bs.data[bs.aByte] |= (v<<(7-bs.aBit++));
        if (bs.aBit >= 8) { bs.aBit = 0; bs.aByte++; }
    }

    //----- writes the [bits] lowest bit from [value] into the bitstream [bs] (max 32)
    public void compr_writeBitStream(Compr_bitStream bs, int value, int bits) {
        if (bs.aBit == 0) bs.data[bs.aByte] = 0;
        int nBits = 8-bs.aBit;
        if (nBits >= bits) {
            bs.data[bs.aByte] = (byte)((bs.data[bs.aByte]) | (value << (nBits-bits)));
            bs.aBit += bits;
            bits = 0;
        } else  {
            bs.data[bs.aByte] = (byte)((bs.data[bs.aByte]) | (value >> (bits-nBits)));
            bits-=nBits;
            bs.aBit = 8;
        }
        if (bs.aBit == 8) { bs.aByte++; bs.data[bs.aByte] = 0; bs.aBit = 0; }
        while (bits >= 8) {
            bits-=8;
            bs.data[bs.aByte] = (byte) ((value >> bits)  & 0x000000FF);
            bs.aByte++; bs.data[bs.aByte] = 0;
        }
        if (bits > 0) {
            bs.data[bs.aByte] = (byte) ((value << (8-bits)) & 0x000000FF);
            bs.aBit += bits;
        }
        if (bs.aBit == 8) { bs.aByte++; bs.data[bs.aByte] = 0; bs.aBit = 0; }
    }

    //----- returns [bits] bits from the bitstream [bs] (max 32)
    public int compr_readBitStream(Compr_bitStream bs, int bits) {
        if (bits == 0) return 0;
        int output = 0;
        int tmp = 0;
        int nBits = 8-bits-bs.aBit;
        if (bits <= (8-bs.aBit)) { //----- all bits in actual byte
            output = bs.data[bs.aByte];
            if (nBits > 0) output >>= nBits;
            output <<= 32-bits-1;
            output &= 0x7FFFFFFF;
            output >>= 32-bits-1;
            if ((bs.aBit + bits) == 8) { bs.aByte++; bs.aBit = 0; }
            else bs.aBit += bits;
            bits = 0;
        } else {                    //----- read bits from first byte
            output = bs.data[bs.aByte];
            output <<= 24+bs.aBit-1;
            output &= 0x7FFFFFFF;
            output >>= 24+bs.aBit-1;
            bits -= (8-bs.aBit);
            bs.aByte++; bs.aBit = 0;
        }
        while (bits >= 8) {         //----- read full bytes
            tmp = bs.data[bs.aByte++];
            if (tmp < 0) tmp += 256;
            bits -= 8;
            output <<= 8;
            output += tmp;
        }
        if (bits > 0) {             //----- read remaining bits
            tmp = bs.data[bs.aByte];
            if (tmp < 0) tmp += 256;
            output <<= bits;
            tmp >>= 8-bits;
            output += tmp;
            bs.aBit=bits;
        }
        return output;
    }

    //----- writes [bytes] bytes of [value] into the byte stream [bs] (max 4)
    public void compr_writeByteStream(Compr_bitStream bs, int value, int bytes) {

        while (bytes > 0) {
            bs.data[bs.aByte++] = (byte)((value >> ((bytes-1) << 3)) & 0xFF);
            bytes--;
        }
    }

    //----- reads [bytes] bytes from the byte stream [bs] (max 4)
    public int compr_readByteStream(Compr_bitStream bs, int bytes) {
        int output = 0;
        int tmp;
        for (int i = 0; i<bytes; i++) {
            output <<= 8;
            tmp = bs.data[bs.aByte++];
            if (tmp < 0) tmp += 256;
            output |= tmp;
        }
        return (int)output;
    }

    //----- write decompression statistics (time)
    public void uncompr_writeStatistics(int step) {
        switch(step) {
            case 0:
                stats.ts_step0  = System.currentTimeMillis();
                stats.t_step0   = (int)(stats.ts_step0 - stats.ts_step1);
                break;
            case 1:
                stats.ts_step1  = System.currentTimeMillis();
                stats.t_step1   = (int)(stats.ts_step1 - stats.ts_step2);
                break;
            case 2:
                stats.ts_step2  = System.currentTimeMillis();
                stats.t_step2   = (int)(stats.ts_step2 - stats.ts_step3);
                break;
            case 3:
                stats.ts_step3  = System.currentTimeMillis();
                break;
        }
    }

    //----- returns decompression statistics as string
    public String uncompr_getStatisticsStr() {
        return "DeCompress: durations: " + stats.t_step0 + " " + stats.t_step1 + " " + stats.t_step2;// + "\n";
    }

    //----- returns decompression statistics as table
    public String uncompr_getStatisticsTable() {
        return "\nStatistiken:\t\tAnfang\t\tStufe 1\t\tStufe 2\t\tStufe 3\n--------------------------------------------------------------------------------\nZeit (ms):\t\t0\t\t" + stats.t_step0 + "\t\t" + stats.t_step1 + "\t\t" + stats.t_step2 + "\n";
    }

    //----- ----------------------------------------------------------------------------------------------------
    //----- Huffmann decoding
    //----- ----------------------------------------------------------------------------------------------------

    //----- Expand data that was compressed with compr_step3_huffmann()
    int uncompr_step3_huffmann(Compr_bitStream in, Compr_bitStream out, int size) {
        int i;
        for (i=0; i<514; i++) huff_nodes[i] = new Huff_node();
        for (i=0; i<257; i++) huff_codes[i] = new Huff_code();

        //----- read counters
        for (i=0; i<256; i++)
            huff_nodes[i].count = compr_readByteStream(in, 1);
        huff_nodes[HUFF_EOS].count = 1;
        //----- create huffmann tree
        int root = compr_step3_huffmann_build_tree();
        //----- decode data using the created model
        int node;
        for (;;) {
            node = root;
            do {
                if (compr_readBit(in)) node = huff_nodes[node].child2;
                else node = huff_nodes[node].child1;
            } while (node > HUFF_EOS);
            if (node == HUFF_EOS) break;
            out.data[out.aByte++] = (byte)node;
        }
        return out.aByte;
    }
    int compr_step3_huffmann_build_tree() {
        huff_nodes[513].count = 0x7FFFFFFF;
        int min1, min2, next_free;
        for (next_free = HUFF_EOS + 1;; next_free++) {
            min1 = min2 = 513;
            for (int i = 0; i < next_free; i++) {
                if (huff_nodes[i].count != 0) {
                    if      (huff_nodes[i].count < huff_nodes[min1].count) { min2 = min1; min1 = i; }
                    else if (huff_nodes[i].count < huff_nodes[min2].count)                min2 = i;
                }
            }
            if (min2 == 513) break;
            huff_nodes[next_free].count = huff_nodes[min1].count + huff_nodes[min2].count;
            huff_nodes[min1].count = 0;
            huff_nodes[min2].count = 0;
            huff_nodes[next_free].child1 = min1;
            huff_nodes[next_free].child2 = min2;
        }
        return --next_free;
    }
    //----- ----------------------------------------------------------------------------------------------------
    //----- LZSS (Lempel-Ziv-Storer-Szymanski)
    //----- ----------------------------------------------------------------------------------------------------

    //----- Expand data that was compressed with compr_step2_lzss()
    int uncompr_step2_lzss(Compr_bitStream in, Compr_bitStream out, int index_bits, int len_bits) {
        int i, hit_len, hit_pos;
        int pos = 1;
        lzss_cost_benefit = ((1+index_bits+len_bits)/9);
        lzss_window_size  = (1<<index_bits);
        lzss_pre_wnd_size = ((1<<len_bits)+lzss_cost_benefit);
        lzss_tree_root    = lzss_window_size;

        for (;;) {
            if (compr_readBit(in)) {    //----- read symbol
                out.data[out.aByte] = (byte)compr_readBitStream(in, 8);
                lzss_window[pos] = out.data[out.aByte++];
                pos = ((pos+1)&(lzss_window_size-1));
            } else {                    //----- read phrase
                hit_pos = compr_readBitStream(in, index_bits);
                if (hit_pos == LZSS_EOS) break;
                hit_len = compr_readBitStream(in, len_bits) + lzss_cost_benefit;
                for (i=0; i<=hit_len; i++) {
                    out.data[out.aByte] = lzss_window[((hit_pos+i)&(lzss_window_size-1))];

                    lzss_window[pos] = out.data[out.aByte++];
                    pos = ((pos+1)&(lzss_window_size-1));
                }
            }
        }
        return out.aByte;
    }

    //----- ----------------------------------------------------------------------------------------------------
    //----- Move-To-Front transform (MTF)
    //----- ----------------------------------------------------------------------------------------------------

    int uncompr_step2_mtf(Compr_bitStream in, Compr_bitStream out, int size) {
        int i, k, m, z;
        for (i=0; i<256; i++) mtf_alphabet[i] = i;  //----- prepare alphabet indices
        for (i=0; i<size; i++) {
            k = in.data[in.aByte++];
            if (k<0) k += 256;
            z = mtf_alphabet[k];
            out.data[out.aByte++] = (byte)z;
            for (m = k; m!=0; --m) mtf_alphabet[m] = mtf_alphabet[m-1];
            mtf_alphabet[0] = z;
        }
        return out.aByte;
    }


}
