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
//----- CompressTool compresses an decompresses universal data (inside an uint8_t array)
//----- The compressed data overwrites the input data in the same array.
//----- Compression is done in two steps:
//----- Step 2: Table based coding and transforms
//----- Step 3: Statistical coding
//----- For compressing scan3d_data structures, there is Scan3dCompressTool,
//----- which includes another step (step 1) containing special methods.
//----- Scan3dCompressTool accesses CompressTool.
//-----
//----- Several compression methods can be chosen (Parameters flags_s1, flags_s2, flags_s3)
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

#include <main/compress_tool.h>

#include <string.h>

CompressTool::CompressTool()  {
    gdos = NULL;
}

CompressTool::CompressTool(RackMailbox * p_mbx, int32_t gdos_level)  {
    gdos = new RackGdos(p_mbx, gdos_level);
}

CompressTool::~CompressTool()  {
    if (gdos) delete gdos;
}

//----- ----------------------------------------------------------------------------------------------------
//----- compression step 2
//----- ----------------------------------------------------------------------------------------------------

uint32_t CompressTool::Compress(uint8_t *in_out, uint32_t flags_s2, uint32_t flags_s3) {

    //----- prepare input stream
    compr_bitStream     inStream2;
    inStream2.data      = in_out;
    inStream2.aByte     = 0;
    inStream2.aBit      = 0;
    //----- prepare output stream
    pbuf23              = (uint8_t*)&buf23;

    pbuf23[0]           = 0x4D;             //----- "Magic value"
    pbuf23[1]           = 0x32;
    pbuf23[2]           = 0;
    compr_bitStream     outStream2;
    outStream2.data     = pbuf23;
    outStream2.aByte    = 2;
    outStream2.aBit     = 0;
    compr_writeByteStream(&outStream2, flags_s2, sizeof(flags_s2));
    compr_writeByteStream(&outStream2, stats.s_step1, sizeof(stats.s_step1));
    outStream2.aByte    = 14;
    uint32_t size2      = 0;

#ifdef COMPR_USE_LZSS
    uint32_t size2a, par1, par2;
#endif

    //----- execute compression functions of step 2
    switch (flags_s2 & 0x000000FF) {
    case COMPR_S2_MTF:
        size2 = compr_step2_mtf(&inStream2, &outStream2, stats.s_step1);
        break;
#ifdef COMPR_USE_LZSS
    case COMPR_S2_LZSS:
        par1 = ((flags_s2 & 0xFF000000) >> 24);
        if (par1 == 0) par1 = LZSS_INDEX_BITS;
        par2 = ((flags_s2 & 0x00FF0000) >> 16);
        if (par2 == 0) par2 = LZSS_LEN_BITS;
        size2 = compr_step2_lzss(&inStream2, &outStream2, stats.s_step1, par1, par2);
        break;
    case COMPR_S2_MTF_LZSS:
        size2a = compr_step2_mtf(&inStream2, &outStream2, stats.s_step1);
        inStream2.data   = pbuf23;
        inStream2.aByte  = 14;
        inStream2.aBit   = 0;
        outStream2.data  = in_out;
        outStream2.aByte = 0;
        outStream2.aBit  = 0;
        par1 = ((flags_s2 & 0xFF000000) >> 24);
        if (par1 == 0) par1 = LZSS_INDEX_BITS;
        par2 = ((flags_s2 & 0x00FF0000) >> 16);
        if (par2 == 0) par2 = LZSS_LEN_BITS;
        size2 = compr_step2_lzss(&inStream2, &outStream2, size2a, par1, par2);
        memcpy(&pbuf23[14], in_out, stats.s_step1);
        outStream2.data  = pbuf23;
        break;
#endif
#ifdef COMPR_USE_BWT
    case COMPR_S2_BWT:
        par1 = ((flags_s2 & 0xFFFFFF00) >> 8);
        if (par1 == 0) par1 = BWT_BLOCKSIZE;
        size2 = compr_step2_bwt(&inStream2, &outStream2, stats.s_step1, par1);
        break;
    case COMPR_S2_BWT_MTF:
        par1 = ((flags_s2 & 0xFFFFFF00) >> 8);
        if (par1 == 0) par1 = BWT_BLOCKSIZE;
        size2a = compr_step2_bwt(&inStream2, &outStream2, stats.s_step1, par1);
        inStream2.data   = pbuf23;
        inStream2.aByte  = 14;
        inStream2.aBit   = 0;
        outStream2.data  = in_out;
        outStream2.aByte = 0;
        outStream2.aBit  = 0;
        size2 = compr_step2_mtf(&inStream2, &outStream2, size2a);
        memcpy(&pbuf23[14], in_out, stats.s_step1);
        outStream2.data  = pbuf23;
        break;
#endif
#ifdef COMPR_USE_LZW
    case COMPR_S2_LZW:
        par1 = ((flags_s2 & 0xFF000000) >> 24);
        if (par1 == 0) par1 = LZW_BITS;
        size2 = compr_step2_lzw(&inStream2, &outStream2, stats.s_step1, par1);
        break;
    case COMPR_S2_LZWV:
        par1 = ((flags_s2 & 0xFF000000) >> 24);
        if (par1 == 0) par1 = LZWV_MAX_BITS;
        size2 = compr_step2_lzwv(&inStream2, &outStream2, stats.s_step1, par1);
        break;
#endif
    case COMPR_S2_NONE:
        memcpy(&pbuf23[14], in_out, stats.s_step1);
        size2 = stats.s_step1+14;
        break;
    }

    outStream2.aByte = 10; outStream2.aBit = 0;
    compr_writeByteStream(&outStream2, size2, sizeof(size2));
    compr_writeStatistics(2, size2);

//----- ----------------------------------------------------------------------------------------------------
//----- compression step 3
//----- ----------------------------------------------------------------------------------------------------

    //----- prepare output stream
    in_out[0]           = 0x4D;             //----- "Magic value"
    in_out[1]           = 0x43;
    in_out[2]           = 0;
    compr_bitStream     outStream3;
    outStream3.data     = in_out;
    outStream3.aByte    = 2;
    outStream3.aBit     = 0;
    compr_writeByteStream(&outStream3, flags_s3, sizeof(flags_s3));
    compr_writeByteStream(&outStream3, stats.s_step2, sizeof(stats.s_step2));
    outStream3.aByte    = 14;
    outStream2.aByte    = 0;

    uint32_t size3 = 0;

    //----- execute compression functions of step 3
    switch (flags_s3) {
    case COMPR_S3_HUFFMANN:
        size3 = compr_step3_huffmann(&outStream2, &outStream3, size2);
        break;
#ifdef COMPR_USE_ADHUFF
    case COMPR_S3_AD_HUFFMANN:
        size3 = compr_step3_adhuff(&outStream2, &outStream3, size2);
        break;
#endif
    case COMPR_S3_NONE:
        memcpy(&in_out[14], pbuf23, stats.s_step2);
        size3 = stats.s_step2+14;
        break;
    }


    outStream3.aByte = 10; outStream3.aBit = 0;
    compr_writeByteStream(&outStream3, size3, sizeof(size3));
    compr_writeStatistics(3, size3);
    return stats.s_step3;
}

//----- ----------------------------------------------------------------------------------------------------
//----- compression functions step 2
//----- ----------------------------------------------------------------------------------------------------

#ifdef COMPR_USE_LZSS
//----- ----------------------------------------------------------------------------------------------------
//----- LZSS (Lempel-Ziv-Storer-Szymanski)
//----- ----------------------------------------------------------------------------------------------------

uint32_t CompressTool::compr_step2_lzss(compr_bitStream *in, compr_bitStream *out, uint32_t size, uint32_t index_bits, uint32_t len_bits) {
    int32_t i, pre_wnd_bytes, replace;
    int32_t pos = 1;
    lzss_cost_benefit = ((1+index_bits+len_bits)/9);
    lzss_window_size  = (1<<index_bits);
    lzss_pre_wnd_size = ((1<<len_bits)+lzss_cost_benefit);
    lzss_tree_root    = lzss_window_size;

    for (i=0; i<lzss_pre_wnd_size; i++) lzss_window[pos+i] = in->data[in->aByte++];
    pre_wnd_bytes = i;
    compr_step2_lzss_init_tree(pos);
    int32_t hit_len = 0;
    int32_t hit_pos = 0;
    while (pre_wnd_bytes > 0) {
        if (hit_len > pre_wnd_bytes) hit_len = pre_wnd_bytes;
        if (hit_len <= lzss_cost_benefit) { //----- cost/benefit to low -> write symbol
            replace = 1;
            compr_writeBit(out, 1);
            compr_writeBitStream(out, lzss_window[pos], 8);
        } else {                            //----- cost/benefit well -> write phrase
            compr_writeBit(out, 0);
            compr_writeBitStream(out, hit_pos, index_bits); //----- phrase position
            compr_writeBitStream(out, hit_len-lzss_cost_benefit-1, len_bits);   //----- phrase length
            replace = hit_len;
        } for (i=0; i<replace; i++) {
            compr_step2_lzss_remove_str(LZSS_MOD_WND(pos+lzss_pre_wnd_size));
            if (in->aByte >= size) pre_wnd_bytes--;
            else lzss_window[LZSS_MOD_WND(pos+lzss_pre_wnd_size)] = in->data[in->aByte++];
            pos = LZSS_MOD_WND(pos+1);
            if (pre_wnd_bytes > 0) hit_len = compr_step2_lzss_add_str(pos, &hit_pos);
        }
    }
    compr_writeBit(out, 0);
    compr_writeBitStream(out, LZSS_EOS, index_bits);
    return out->aByte;
}

void CompressTool::compr_step2_lzss_init_tree(int32_t r) {
    //----- ensure, that everything is zero at the beginning
    for (uint32_t i=0; i<lzss_window_size+1; i++) {
        lzss_tree[i].parent               = LZSS_UNUSED;
        lzss_tree[i].child_large          = LZSS_UNUSED;
        lzss_tree[i].child_small          = LZSS_UNUSED;
    }
    lzss_tree[lzss_tree_root].child_large = r;
    lzss_tree[r].parent                   = lzss_tree_root;
    lzss_tree[r].child_large              = LZSS_UNUSED;
    lzss_tree[r].child_small              = LZSS_UNUSED;
}
int32_t CompressTool::compr_step2_lzss_add_str(int32_t new_node, int32_t *hit_pos) {
    int32_t delta, *child, i;
    if (new_node == LZSS_EOS) return 0;
    int32_t test_node = lzss_tree[lzss_tree_root].child_large;
    int32_t hit_len = 0;
    for (;;) {
        for (i=0; i< lzss_pre_wnd_size; i++) {
            delta = lzss_window[LZSS_MOD_WND(new_node+i)]-lzss_window[LZSS_MOD_WND(test_node+i)];
            if (delta != 0) break;
        }
        if (i >= hit_len) {
            hit_len = i;
            *hit_pos = test_node;
            if (hit_len >= lzss_pre_wnd_size) {
                compr_step2_lzss_replace_nodes(test_node, new_node);
                return hit_len;
            }
        }
        if (delta >= 0) child = &lzss_tree[test_node].child_large;
        else            child = &lzss_tree[test_node].child_small;
        if (*child == LZSS_UNUSED) {
            *child = new_node;
            lzss_tree[new_node].parent      = test_node;
            lzss_tree[new_node].child_large = LZSS_UNUSED;
            lzss_tree[new_node].child_small = LZSS_UNUSED;
            return hit_len;
        }
        test_node = *child;
    }
    return 0;
}
void CompressTool::compr_step2_lzss_combine_nodes(int32_t old_node, int32_t new_node) {
    lzss_tree[new_node].parent = lzss_tree[old_node].parent;
    if (lzss_tree[lzss_tree[old_node].parent].child_large == old_node)
        lzss_tree[lzss_tree[old_node].parent].child_large = new_node;
    else lzss_tree[lzss_tree[old_node].parent].child_small = new_node;
    lzss_tree[old_node].parent = LZSS_UNUSED;
}

void CompressTool::compr_step2_lzss_replace_nodes(int32_t old_node, int32_t new_node) {
    int32_t parent = lzss_tree[old_node].parent;
    if (lzss_tree[parent].child_small == old_node)
        lzss_tree[parent].child_small = new_node;
    else lzss_tree[parent].child_large = new_node;
    lzss_tree[new_node] = lzss_tree[old_node];
    lzss_tree[lzss_tree[new_node].child_small].parent = new_node;
    lzss_tree[lzss_tree[new_node].child_large].parent = new_node;
    lzss_tree[old_node].parent = LZSS_UNUSED;
}

int32_t CompressTool::compr_step2_lzss_search_next_node(int32_t node) {
    int32_t next = lzss_tree[node].child_small;
    while (lzss_tree[next].child_large != LZSS_UNUSED)
        next = lzss_tree[next].child_large;
    return next;
}

void CompressTool::compr_step2_lzss_remove_str(int32_t p) {
    if (lzss_tree[p].parent == LZSS_UNUSED) return;
    if (lzss_tree[p].child_large == LZSS_UNUSED)
        compr_step2_lzss_combine_nodes(p, lzss_tree[p].child_small);
    else if (lzss_tree[p].child_small == LZSS_UNUSED)
        compr_step2_lzss_combine_nodes(p, lzss_tree[p].child_large);
    else {
        int32_t replace = compr_step2_lzss_search_next_node(p);
        compr_step2_lzss_remove_str(replace);
        compr_step2_lzss_replace_nodes(p, replace);
    }
}

#endif

#ifdef COMPR_USE_LZW
//----- ----------------------------------------------------------------------------------------------------
//----- LZW (Lempel-Ziv-Welch)
//----- ----------------------------------------------------------------------------------------------------

uint32_t CompressTool::compr_step2_lzw(compr_bitStream *in, compr_bitStream *out, uint32_t size, uint32_t bits) {
    int32_t i, index;
    uint8_t c;
    int32_t next_code = LZW_FIRST_CODE;
    int32_t string_code = in->data[in->aByte++];
    for (i=0; i<LZW_TABLE_SIZE; i++) lzw_table[i].code = LZW_UNUSED;
    while (in->aByte < size) {
        c = in->data[in->aByte++];
        index = compr_step2_lzw_find_child(string_code, c);
        if (lzw_table[index].code != LZW_UNUSED)
            string_code = lzw_table[index].code;
        else {
            if (next_code <= LZW_MAX_CODE) {
                lzw_table[index].code = next_code++;
                lzw_table[index].parent = string_code;
                lzw_table[index].c = c;
            }
            compr_writeBitStream(out, string_code, LZW_BITS);
            string_code = c;
        }
    }
    compr_writeBitStream(out, string_code, LZW_BITS);
    compr_writeBitStream(out, LZW_EOS, LZW_BITS);
    return out->aByte;
}

uint32_t CompressTool::compr_step2_lzw_find_child(int32_t parent, uint8_t child) {
    int32_t offset;
    int32_t index = (child<<(LZW_BITS-8))^parent;
    if (index == 0) offset = 1;
    else offset = LZW_TABLE_SIZE - index;
    for (;;) {
        if ((lzw_table[index].code == LZW_UNUSED) ||
           (lzw_table[index].parent == parent && lzw_table[index].c == child)) return index;
        index -= offset;
        if (index < 0) index += LZW_TABLE_SIZE;
    }
}

//----- some improvements for LZW (LZWV = LZW variable)
void CompressTool::compr_step2_lzwv_init_table() {
    for (uint32_t i=0; i<LZW_TABLE_SIZE; i++) lzw_table[i].code = LZW_UNUSED;
    lzw_next_code = LZW_FIRST_CODE;
    lzw_akt_bits  = 9;
    lzw_next_rise = 511; //----- = 2^9-1
}

uint32_t CompressTool::compr_step2_lzwv(compr_bitStream *in, compr_bitStream *out, uint32_t size, uint32_t max_bits) {
    int32_t index;
    uint8_t c;
    compr_step2_lzwv_init_table();
    int32_t string_code = in->data[in->aByte++];
    while (in->aByte < size) {
        c = in->data[in->aByte++];
        index = compr_step2_lzw_find_child(string_code, c);
        if (lzw_table[index].code != LZW_UNUSED)
            string_code = lzw_table[index].code;
        else {
            lzw_table[index].code = lzw_next_code++;
            lzw_table[index].parent = string_code;
            lzw_table[index].c = c;
            compr_writeBitStream(out, string_code, lzw_akt_bits);
            string_code = c;
            if (lzw_next_code > LZW_MAX_CODE) {
                compr_writeBitStream(out, LZW_CLEAR_TABLE, lzw_akt_bits);
                compr_step2_lzwv_init_table();
            } else if (lzw_next_code > lzw_next_rise) {
                compr_writeBitStream(out, LZW_RISE_CODE, lzw_akt_bits++);
                lzw_next_rise <<=1; lzw_next_rise |= 1;
            }
        }
    }
    compr_writeBitStream(out, string_code, lzw_akt_bits);
    compr_writeBitStream(out, LZW_EOS, lzw_akt_bits);
    return out->aByte;
}
#endif

//----- ----------------------------------------------------------------------------------------------------
//----- Move-To-Front transform (MTF)
//----- ----------------------------------------------------------------------------------------------------

uint32_t CompressTool::compr_step2_mtf(compr_bitStream *in, compr_bitStream *out, uint32_t size) {
    uint32_t i, k, m, z;
    for (i=0; i<256; i++) mtf_alphabet[i] = i;  //----- prepare alphabet indices
    for (i=0; i<size; i++) {
        z = in->data[in->aByte++];
        k = 0;
        while (mtf_alphabet[k] != z) k++;
        out->data[out->aByte++] = k;
        if (k != 0) for (m=k; m>0; m--) mtf_alphabet[m] = mtf_alphabet[m-1];
        mtf_alphabet[0] = z;
    }
    return out->aByte;
}

#ifdef COMPR_USE_BWT
//----- ----------------------------------------------------------------------------------------------------
//----- Burrows Wheeler Transform (BWT)
//----- The BWT is very slow and does not work properlay, so it has not been optimised.
//----- So this section of the code ist EXPERIMENTAL
//----- ----------------------------------------------------------------------------------------------------

//----- Compares two uint8_t strings a and b up to length size lexicographically.
//----- returns 1 if a>b, -1 if a<b, 0 if a=b
int32_t CompressTool::compr_step2_bwt_strcmpn(uint8_t *a, uint8_t *b, int32_t size) {
    uint32_t i=0;
    while ((a[i] == b[i]) && (size > 0) ) { i++; size--; }
    if (size == 0) return 0;
    if (a[i] > b[i]) return 1; else return -1;
}

//----- Sorts an array of uint8_t strings [bwt_buf], that all have the size n
//----- in ascending order using the quicksort algorithm. Input data is not changed.
//----- Output is the uint8_t* array [bwt_index] containing pointers to the input data.
//----- This array needs to be initialized with pointers to each position in the input data.
//----- The parameters l and r point to the left and right borders of the array

void CompressTool::compr_step2_bwt_quicksort(int32_t n, int32_t left, int32_t right) {
    int32_t l, r;
    uint8_t *tmp, *pivot;
    if (right > left){                          //----- as long as the subsequence has more than one element
        pivot = bwt_index[right];
        l = left; r = right;                    //----- initialize l and r with the borders
        do {                                    //----- increment, until higher element is found
            while (compr_step2_bwt_strcmpn(bwt_index[l], pivot, n) == -1) l++;
                                                //----- decrement, until lower element is found
            while (compr_step2_bwt_strcmpn(bwt_index[r], pivot, n) ==  1) r--;
            if (l <= r) {
                tmp=bwt_index[l];               //----- swap lower and higer element (Indices)
                bwt_index[l]=bwt_index[r];
                bwt_index[r]=tmp;
                l++;
                r--;
            }
        } while (l <= r);
        compr_step2_bwt_quicksort(n, left, r);  //----- recursive call for left subset
        compr_step2_bwt_quicksort(n, l, right); //----- recursive call for left subset
    }
}

uint32_t CompressTool::compr_step2_bwt(compr_bitStream *in, compr_bitStream *out, uint32_t size, uint32_t blocksize) {
    uint32_t i;
    //----- copy input data twice in a row
    memcpy(&bwt_buf, &in->data[in->aByte], size);
    memcpy((void*)((int32_t)&bwt_buf+size), &in->data[in->aByte], size);
    //----- Create an array with pointers to every position in the input data
    bwt_index[0] = &bwt_buf[0];
    for (i = 1; i<size; i++) bwt_index[i] = bwt_index[i-1]+1;

    //----- transform input data block-by-block
    uint32_t block = 0;
    uint32_t index;
    while (in->aByte < size) {
        //(block = 0; block <= (int32_t)(size/blocksize); block++) {
        if ((block*(blocksize+1))>size) blocksize = size % block;

        memcpy(&bwt_buf, &in->data[in->aByte], blocksize);
        memcpy((void*)((int32_t)&bwt_buf+blocksize), &in->data[in->aByte], blocksize);
        in->aByte += blocksize;
        //----- Create an array with pointers to every position in the input data
        bwt_index[0] = &bwt_buf[0];
        for (i = 1; i<blocksize; i++) bwt_index[i] = bwt_index[i-1]+1;

        compr_step2_bwt_quicksort(blocksize, 0, blocksize-1);
        index = 0;
        while (bwt_index[index] != &bwt_buf[0]) index++; //----- find index
        compr_writeByteStream(out, index, 4);
        for (i=0; i<blocksize; i++)
            out->data[out->aByte++] = bwt_index[i][blocksize-1];
        block++;
    }
    //----- transform the last block

    //----- Create an array with pointers to every position in the input data
    bwt_index[0] = &bwt_buf[0];
    for (i = 1; i<blocksize; i++) bwt_index[i] = bwt_index[i-1]+1;

    compr_step2_bwt_quicksort(size-block*BWT_BLOCKSIZE, block*BWT_BLOCKSIZE, size-1);
    index = block*BWT_BLOCKSIZE;
    while (bwt_index[index] != &bwt_buf[block*BWT_BLOCKSIZE]) index++; //----- find index
    compr_writeByteStream(out, (index%BWT_BLOCKSIZE), 4);
    for (i=block*BWT_BLOCKSIZE; i<size; i++)
        out->data[out->aByte++] = bwt_index[i][size-1];

    return out->aByte;
}
#endif



//----- ----------------------------------------------------------------------------------------------------
//----- compression functions step 3
//----- ----------------------------------------------------------------------------------------------------

//----- ----------------------------------------------------------------------------------------------------
//----- Huffmann coding
//----- ----------------------------------------------------------------------------------------------------

uint32_t CompressTool::compr_step3_huffmann(compr_bitStream *in, compr_bitStream *out, uint32_t size) {
    uint8_t *pIn = in->data;
    uint32_t i;
    for (i=in->aByte; i<256; i++) huff_counters[i] = 0;
    //----- determine the occurence of each symbol
    for (i=0; i<size; i++)
        huff_counters[pIn[i]]++;
    //----- determine max counter value
    uint32_t cmax = 0;
    for (i=0; i<256; i++) if (huff_counters[i] > cmax) cmax=huff_counters[i];
    if (cmax == 0) { huff_counters[0] = 1; cmax = 1; }
    //----- scale counters to 16 bit
    cmax/=255; cmax++;
    for (i=0; i<256; i++) {
        huff_nodes[i].count = huff_counters[i]/cmax;
        if ((huff_nodes[i].count == 0) && (huff_counters[i] != 0)) huff_nodes[i].count = 1;
    }
    huff_nodes[HUFF_EOS].count = 1;

    //-----write counters into output stream
    for (i=0; i<256; i++) out->data[out->aByte++] = huff_nodes[i].count;

    //----- create huffmann tree
    int32_t root = compr_step3_huffmann_build_tree();

    //----- create codes from tree
    compr_step3_huffmann_tree2codes(0, 0, root);

    //----- code data using the created model
    for (i = 0; i<size; i++)
        compr_writeBitStream(out, huff_codes[pIn[i]].code, huff_codes[pIn[i]].len);
    compr_writeBitStream(out, huff_codes[HUFF_EOS].code, huff_codes[HUFF_EOS].len);


    if (out->aBit > 0) out->aByte++;  //----- round up bytes
    return out->aByte;
}

void CompressTool::compr_step3_huffmann_tree2codes(uint32_t code_so_far, int32_t bits, int32_t node) {
    if (node <= HUFF_EOS) {
        huff_codes[node].code = code_so_far;
        huff_codes[node].len = bits;
        return;
    } else {
        code_so_far <<= 1;
        bits++;
        compr_step3_huffmann_tree2codes(code_so_far, bits, huff_nodes[node].child1);
        compr_step3_huffmann_tree2codes(code_so_far | 1, bits, huff_nodes[node].child2);
    }
}

uint32_t CompressTool::compr_step3_huffmann_build_tree() {
    huff_nodes[513].count = -1;
    uint32_t min1, min2, next_free;
    for (next_free = HUFF_EOS + 1;; next_free++) {
        min1 = min2 = 513;
        for (uint32_t i = 0; i < next_free; i++) {
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

#ifdef COMPR_USE_ADHUFF
//----- ----------------------------------------------------------------------------------------------------
//----- adaptive Huffmann coding
//----- ----------------------------------------------------------------------------------------------------

uint32_t CompressTool::compr_step3_adhuff(compr_bitStream *in, compr_bitStream *out, uint32_t size) {
    uint32_t c;
    compr_step3_adhuff_init_tree();
    while (in->aByte < size+14) {
        c = in->data[in->aByte++];
        compr_step3_adhuff_code_symbol(c, out);
        compr_step3_adhuff_refresh_model(c);
    }
    compr_step3_adhuff_code_symbol(AD_HUFF_EOS, out);
    return out->aByte;
}

void CompressTool::compr_step3_adhuff_init_tree() {
    adhuff_tree.node[AD_HUFF_ROOT].child           = AD_HUFF_ROOT + 1;
    adhuff_tree.node[AD_HUFF_ROOT].child_is_leaf   = false;
    adhuff_tree.node[AD_HUFF_ROOT].weight          = 2;
    adhuff_tree.node[AD_HUFF_ROOT].parent          = AD_HUFF_UNUSED;
    adhuff_tree.node[AD_HUFF_ROOT+1].child         = AD_HUFF_EOS;
    adhuff_tree.node[AD_HUFF_ROOT+1].child_is_leaf = true;
    adhuff_tree.node[AD_HUFF_ROOT+1].weight        = 1;
    adhuff_tree.node[AD_HUFF_ROOT+1].parent        = AD_HUFF_ROOT;
    adhuff_tree.node[AD_HUFF_ROOT+2].child         = AD_HUFF_ESCAPE;
    adhuff_tree.node[AD_HUFF_ROOT+2].child_is_leaf = true;
    adhuff_tree.node[AD_HUFF_ROOT+2].weight        = 1;
    adhuff_tree.node[AD_HUFF_ROOT+2].parent        = AD_HUFF_ROOT;
    adhuff_tree.leaf[AD_HUFF_EOS]                  = AD_HUFF_ROOT + 1;
    adhuff_tree.leaf[AD_HUFF_ESCAPE]               = AD_HUFF_ROOT + 2;
    adhuff_tree.next_free                          = AD_HUFF_ROOT + 3;
    for (uint32_t i=0; i<AD_HUFF_EOS; i++) adhuff_tree.leaf[i] = AD_HUFF_UNUSED;
}

void CompressTool::compr_step3_adhuff_build_tree() {
    int32_t i, j, k;
    uint32_t weight;
    //----- place all leaves at the end and halve counters
    j = adhuff_tree.next_free-1;
    for (i=j; i>=AD_HUFF_ROOT; i--) {
        if (adhuff_tree.node[i].child_is_leaf) {
            adhuff_tree.node[j] = adhuff_tree.node[i];
            adhuff_tree.node[j].weight = (adhuff_tree.node[j].weight+1)>>1;
            j--;
        }
    }
    //----- create internal nodes
    for (i= adhuff_tree.next_free-2; j>=AD_HUFF_ROOT; i-=2, j--) {
        k=i+1;
        adhuff_tree.node[j].weight = adhuff_tree.node[i].weight + adhuff_tree.node[k].weight;
        weight = adhuff_tree.node[j].weight;
        adhuff_tree.node[j].child_is_leaf = false;
        for (k=j+1; weight < adhuff_tree.node[k].weight; k++); k--;
        memmove(&adhuff_tree.node[j], &adhuff_tree.node[j+1], (k-j)*sizeof(adhuff_node));
        adhuff_tree.node[k].weight = weight;
        adhuff_tree.node[k].child = i;
        adhuff_tree.node[k].child_is_leaf = false;
    }
    //----- update all lefes and parent elements
    for (i=adhuff_tree.next_free-1;i>=AD_HUFF_ROOT; i--) {
        if (adhuff_tree.node[i].child_is_leaf) {
            k = adhuff_tree.node[i].child;
            adhuff_tree.leaf[k] = i;
        } else {
            k = adhuff_tree.node[i].child;
            adhuff_tree.node[k].parent = adhuff_tree.node[k+1].parent = i;
        }
    }
}

void CompressTool::compr_step3_adhuff_swap_nodes(int32_t i, int32_t j) {
    adhuff_node temp;
    if (adhuff_tree.node[i].child_is_leaf) adhuff_tree.leaf[adhuff_tree.node[i].child] = j;
    else {
        adhuff_tree.node[adhuff_tree.node[i].child].parent = j;
        adhuff_tree.node[adhuff_tree.node[i].child+1].parent = j;
    }
    if (adhuff_tree.node[j].child_is_leaf) adhuff_tree.leaf[adhuff_tree.node[j].child] = i;
    else {
        adhuff_tree.node[adhuff_tree.node[j].child].parent = i;
        adhuff_tree.node[adhuff_tree.node[j].child+1].parent = i;
    }
    temp = adhuff_tree.node[i];
    adhuff_tree.node[i] = adhuff_tree.node[j];
    adhuff_tree.node[i].parent = temp.parent;
    temp.parent = adhuff_tree.node[j].parent;
    adhuff_tree.node[j] = temp;
}

void CompressTool::compr_step3_adhuff_add_node(int32_t c) {
    int32_t lowest_node = adhuff_tree.next_free - 1;
    int32_t new_node    = adhuff_tree.next_free;
    int32_t zero_node   = adhuff_tree.next_free + 1;
    adhuff_tree.next_free += 2;
    adhuff_tree.node[new_node]                         = adhuff_tree.node[lowest_node];
    adhuff_tree.node[new_node].parent                  = lowest_node;
    adhuff_tree.leaf[adhuff_tree.node[new_node].child] = new_node;
    adhuff_tree.node[lowest_node].child                = new_node;
    adhuff_tree.node[lowest_node].child_is_leaf        = false;
    adhuff_tree.node[zero_node].child                  = c;
    adhuff_tree.node[zero_node].child_is_leaf          = true;
    adhuff_tree.node[zero_node].weight                 = 0;
    adhuff_tree.node[zero_node].parent                 = lowest_node;
    adhuff_tree.leaf[c]                                = zero_node;
}

void CompressTool::compr_step3_adhuff_refresh_model(int32_t c) {
    int32_t new_node;

    if (adhuff_tree.node[AD_HUFF_ROOT].weight == AD_HUFF_MAX_WEIGHT) compr_step3_adhuff_build_tree();
    int32_t akt_node = adhuff_tree.leaf[c];
    while (akt_node != AD_HUFF_UNUSED) {
        adhuff_tree.node[akt_node].weight++;
        for(new_node = akt_node; new_node>AD_HUFF_ROOT; new_node--)
            if (adhuff_tree.node[new_node-1].weight >= adhuff_tree.node[akt_node].weight) break;
        if (akt_node != new_node) {
            compr_step3_adhuff_swap_nodes(akt_node, new_node);
            akt_node = new_node;
        }
        akt_node = adhuff_tree.node[akt_node].parent;
    }
}

void CompressTool::compr_step3_adhuff_code_symbol(uint32_t c, compr_bitStream *out) {
    uint32_t code = 0;
    uint32_t akt_bit = 1;
    uint32_t len = 0;
    int32_t akt_node = adhuff_tree.leaf[c];
    if (akt_node == AD_HUFF_UNUSED) akt_node = adhuff_tree.leaf[AD_HUFF_ESCAPE];
    while (akt_node != AD_HUFF_ROOT) {
        if ((akt_node & 1) == 0) code |= akt_bit;
        akt_bit <<= 1;
        len++;
        akt_node = adhuff_tree.node[akt_node].parent;
    }
    compr_writeBitStream(out, code, len);
    if (adhuff_tree.leaf[c] == AD_HUFF_UNUSED) {
        compr_writeBitStream(out, c, 8);
        compr_step3_adhuff_add_node(c);
    }
}

uint32_t CompressTool::uncompr_step3_adhuff_decode_symbol(compr_bitStream *in) {
    int32_t akt_node = AD_HUFF_ROOT;
    while (!adhuff_tree.node[akt_node].child_is_leaf) {
        akt_node = adhuff_tree.node[akt_node].child;
        akt_node += compr_readBit(in);
    }
    uint32_t c = adhuff_tree.node[akt_node].child;
    if (c == AD_HUFF_ESCAPE) {
        c = compr_readBitStream(in, 8);
        compr_step3_adhuff_add_node(c);
    }
    return c;
}

#endif

#ifdef COMPR_UNCOMPR
uint32_t CompressTool::Decompress(uint8_t *in_out) {
//----- ----------------------------------------------------------------------------------------------------
//----- decompression step 3
//----- ----------------------------------------------------------------------------------------------------
    //----- prepare input stream
    compr_bitStream     inStream3;
    inStream3.data      = in_out;
    inStream3.aByte     = 0;
    inStream3.aBit      = 0;
    //----- prepare output stream
    pbuf23              = (uint8_t*)&buf23;
    compr_bitStream     outStream3;
    outStream3.data     = pbuf23;
    outStream3.aByte    = 0;
    outStream3.aBit     = 0;

    //----- evaluate header: Magic value
    if (compr_readByteStream(&inStream3, 2) != 0x4D43) {
        GDOS_PRINT("Fehler: Die komprimierten Daten der 3. Stufe haben keinen gueltigen Header!\n");
        return 1;
    }

    uint32_t flags_s3 = compr_readByteStream(&inStream3, sizeof(flags_s3));
    uint32_t size2    = compr_readByteStream(&inStream3, sizeof(size2));
    uint32_t size3    = compr_readByteStream(&inStream3, sizeof(size3));
    uint32_t aLen     = 0;

    uncompr_writeStatistics(3);
    //----- execute decompression functions of step 3
    switch (flags_s3) {
    case COMPR_S3_HUFFMANN:
        aLen = uncompr_step3_huffmann(&inStream3, &outStream3, size3);
        break;
#ifdef COMPR_USE_ADHUFF
    case COMPR_S3_AD_HUFFMANN:
        aLen = uncompr_step3_adhuff(&inStream3, &outStream3, size3);
        break;
#endif
    case COMPR_S3_NONE:
        memcpy(pbuf23, &in_out[14], size2);
        break;
    }

    uncompr_writeStatistics(2);
//----- ----------------------------------------------------------------------------------------------------
//----- decompression step 2
//----- ----------------------------------------------------------------------------------------------------

    //----- prepare input stream
    compr_bitStream     inStream2;
    inStream2.data      = pbuf23;
    inStream2.aByte     = 0;
    inStream2.aBit      = 0;
    //----- prepare output stream
    compr_bitStream     outStream2;
    outStream2.data     = in_out;
    outStream2.aByte    = 0;
    outStream2.aBit     = 0;
    //----- evaluate header: Magic value
    if (compr_readByteStream(&inStream2, 2) != 0x4D32) {
        GDOS_PRINT("Fehler: Die komprimierten Daten der 2. Stufe haben keinen gueltigen Header!\n");
        return 1;
    }

    uint32_t flags_s2 = compr_readByteStream(&inStream2, sizeof(flags_s2));
    uint32_t size1    = compr_readByteStream(&inStream2, sizeof(size1));
         size2    = compr_readByteStream(&inStream2, sizeof(size2));
         aLen     = 0;
    uint32_t aLen1, par1, par2;

    //----- execute decompression functions of step 2
    switch (flags_s2 & 0x000000FF) {
    case COMPR_S2_MTF:
        aLen = uncompr_step2_mtf(&inStream2, &outStream2, size2);
        break;
#ifdef COMPR_USE_LZSS
    case COMPR_S2_LZSS:
        par1 = ((flags_s2 & 0xFF000000) >> 24);
        if (par1 == 0) par1 = LZSS_INDEX_BITS;
        par2 = ((flags_s2 & 0x00FF0000) >> 16);
        if (par2 == 0) par2 = LZSS_LEN_BITS;
        aLen = uncompr_step2_lzss(&inStream2, &outStream2, par1, par2);
        break;
    case COMPR_S2_MTF_LZSS:
        par1 = ((flags_s2 & 0xFF000000) >> 24);
        if (par1 == 0) par1 = LZSS_INDEX_BITS;
        par2 = ((flags_s2 & 0x00FF0000) >> 16);
        if (par2 == 0) par2 = LZSS_LEN_BITS;
        aLen1 = uncompr_step2_lzss(&inStream2, &outStream2, par1, par2);
        inStream2.data   = in_out;
        inStream2.aByte  = 0;
        inStream2.aBit   = 0;
        outStream2.data  = pbuf23;
        outStream2.aByte = 14;
        outStream2.aBit  = 0;
        aLen = uncompr_step2_mtf(&inStream2, &outStream2, aLen1);
        memcpy(in_out, &pbuf23[14], aLen);
        break;
#endif
#ifdef COMPR_USE_BWT
    case COMPR_S2_BWT:
        par1 = ((flags_s2 & 0xFFFFFF00) >> 8);
        if (par1 == 0) par1 = BWT_BLOCKSIZE;
        aLen = uncompr_step2_bwt(&inStream2, &outStream2, size2, par1);
        break;
    case COMPR_S2_BWT_MTF:
        aLen1 = uncompr_step2_mtf(&inStream2, &outStream2, size2);
        inStream2.data   = in_out;
        inStream2.aByte  = 0;
        inStream2.aBit   = 0;
        outStream2.data  = pbuf23;
        outStream2.aByte = 14;
        outStream2.aBit  = 0;
        par1 = ((flags_s2 & 0xFFFFFF00) >> 8);
        if (par1 == 0) par1 = BWT_BLOCKSIZE;
        aLen = uncompr_step2_bwt(&inStream2, &outStream2, aLen1, par1);
        memcpy(in_out, &pbuf23[14], aLen);
        break;
#endif
#ifdef COMPR_USE_LZW
    case COMPR_S2_LZW:
        par1 = ((flags_s2 & 0xFF000000) >> 24);
        if (par1 == 0) par1 = LZW_BITS;
        aLen = uncompr_step2_lzw(&inStream2, &outStream2, par1);
        break;
    case COMPR_S2_LZWV:
        par1 = ((flags_s2 & 0xFF000000) >> 24);
        if (par1 == 0) par1 = LZWV_MAX_BITS;
        aLen = uncompr_step2_lzwv(&inStream2, &outStream2, par1);
        break;
#endif
    case COMPR_S2_NONE:
        memcpy(in_out, &pbuf23[14], size2);
        break;
    }
    uncompr_writeStatistics(1);
    return 0;
}

//----- ----------------------------------------------------------------------------------------------------
//----- decompression functions step 2
//----- ----------------------------------------------------------------------------------------------------

#ifdef COMPR_USE_LZSS
//----- Expand data that was compressed with compr_step2_lzss()
uint32_t CompressTool::uncompr_step2_lzss(compr_bitStream *in, compr_bitStream *out, uint32_t index_bits, uint32_t len_bits) {
    int32_t i, hit_len, hit_pos;
    int32_t pos       = 1;
    lzss_cost_benefit = ((1+index_bits+len_bits)/9);
    lzss_window_size  = (1<<index_bits);
    lzss_pre_wnd_size = ((1<<len_bits)+lzss_cost_benefit);
    lzss_tree_root    = lzss_window_size;

    for (;;) {
        if (compr_readBit(in)) {    //----- read symbol
            out->data[out->aByte] = compr_readBitStream(in, 8);
            lzss_window[pos] = out->data[out->aByte++];
            pos = LZSS_MOD_WND(pos+1);
        } else {                    //----- read phrase
            hit_pos = compr_readBitStream(in, index_bits);
            if (hit_pos == LZSS_EOS) break;
            hit_len = compr_readBitStream(in, len_bits) + lzss_cost_benefit;
            for (i=0; i<=hit_len; i++) {
                out->data[out->aByte] = lzss_window[LZSS_MOD_WND(hit_pos+i)];
                lzss_window[pos] = out->data[out->aByte++];
                pos = LZSS_MOD_WND(pos+1);
            }
        }
    }
    return out->aByte;
}
#endif

#ifdef COMPR_USE_LZW
uint32_t CompressTool::uncompr_step2_lzw(compr_bitStream *in, compr_bitStream *out, uint32_t bits) {
    uint32_t new_code, old_code, stack_counter;
    uint32_t next_code = LZW_FIRST_CODE;
    old_code = compr_readBitStream(in, LZW_BITS);
    uint8_t c = (uint8_t) old_code;
    out->data[out->aByte++] = c;
    while ((new_code = compr_readBitStream(in, LZW_BITS)) != LZW_EOS) {
        if (new_code >= next_code) {
            lzw_stack[0] = c;
            stack_counter = uncompr_step2_lzw_decode_string(1, old_code);
        } else stack_counter = uncompr_step2_lzw_decode_string(0, new_code);
        c = lzw_stack[stack_counter-1];
        while (stack_counter > 0) out->data[out->aByte++] = lzw_stack[--stack_counter];
        if (next_code <= LZW_MAX_CODE) {
            lzw_table[next_code].parent = old_code;
            lzw_table[next_code].c = c;
            next_code++;
        }
        old_code = new_code;
    }
    return out->aByte;
}
uint32_t CompressTool::uncompr_step2_lzwv(compr_bitStream *in, compr_bitStream *out, uint32_t max_bits) {
    uint32_t new_code, old_code, stack_counter;
    uint8_t c;
    for (;;) {
        compr_step2_lzwv_init_table();
        old_code = compr_readBitStream(in, lzw_akt_bits);
        if (old_code == LZW_EOS) return out->aByte;
        c = (uint8_t) old_code;
        out->data[out->aByte++] = c;
        for (;;) {
            new_code = compr_readBitStream(in, lzw_akt_bits);
            if (new_code == LZW_EOS)          return out->aByte;
            if (new_code == LZW_CLEAR_TABLE)  break;
            if (new_code == LZW_RISE_CODE)  { lzw_akt_bits++; continue; }
            if (new_code >= lzw_next_code) {
                lzw_stack[0] = c;
                stack_counter = uncompr_step2_lzw_decode_string(1, old_code);
            } else stack_counter = uncompr_step2_lzw_decode_string(0, new_code);
            c = lzw_stack[stack_counter-1];
            while (stack_counter > 0) out->data[out->aByte++] = lzw_stack[--stack_counter];
            lzw_table[lzw_next_code].parent = old_code;
            lzw_table[lzw_next_code].c = c;
            lzw_next_code++;
            old_code = new_code;
        }
    }
    return out->aByte;
}
uint32_t CompressTool::uncompr_step2_lzw_decode_string(int32_t count, int32_t code) {
    while (code > 255) {
        lzw_stack[count++] = lzw_table[code].c;
        code = lzw_table[code].parent;
    }
    lzw_stack[count++] = (int8_t) code;
    return count;
}

#endif


//----- Expand data that was compressed with compr_step2_mtf()
uint32_t CompressTool::uncompr_step2_mtf(compr_bitStream *in, compr_bitStream *out, uint32_t size) {
    uint32_t i, k, m, z;
    for (i=0; i<256; i++) mtf_alphabet[i] = i;  //----- prepare alphabet indices
    for (i=0; i<size; i++) {
        k = in->data[in->aByte++];
        z = mtf_alphabet[k];
        out->data[out->aByte++] = z;
        for (m = k; m!=0; --m) mtf_alphabet[m] = mtf_alphabet[m-1];
        mtf_alphabet[0] = z;
    }
    return out->aByte;
}


#ifdef COMPR_USE_BWT
//----- Expand data that was compressed with compr_step2_bwt()
//----- The BWT is very slow and does not work properlay, so it has not been optimised.
//----- So this section of the code ist EXPERIMENTAL
uint32_t CompressTool::uncompr_step2_bwt(compr_bitStream *in, compr_bitStream *out, uint32_t size, uint32_t blocksize) {
    int32_t index, i;
    uint32_t bytesdone = 0;
    while (bytesdone < size) {
        if (bytesdone > size-blocksize) blocksize = size - bytesdone;
        index = compr_readByteStream(in, 4);
        for (i = 0; i<blocksize; i++) bwt_index[i] = (uint8_t*)i;
        uncompr_step2_bwt_mergesort(&in->data[in->aByte], 0, blocksize-1);
        for (i = 0; i<blocksize; i++) {
            out->data[out->aByte++] = in->data[index+in->aByte];
            index = (int32_t)bwt_index[index];
        }
        in->aByte += blocksize;
        bytesdone += blocksize+sizeof(int32_t);
        if (bytesdone+blocksize < size) blocksize = size - bytesdone;
    }
    return out->aByte;
}


void CompressTool::uncompr_step2_bwt_mergesort(uint8_t *in, int32_t left, int32_t right) {
    if (right>left){
        int32_t i, j, k, m;
        m=(right + left)/2;                                    //----- determine middle
        uncompr_step2_bwt_mergesort(in, left, m);              //----- left subsequence
        uncompr_step2_bwt_mergesort(in, m+1, right);           //----- right subsequence
        for (i=m+1; i>left; i--) {
            bwt_buf[i-1]=in[i-1];          //----- left subsequence in auxilliary array
            bwt_index[BWT_BLOCKSIZE+i-1]=bwt_index[i-1];
        }
        for (j=m; j<right; j++) {
            bwt_buf[right+m-j]=in[j+1];    //----- left subsequence reversed in auxilliary array
            bwt_index[BWT_BLOCKSIZE+right+m-j]=bwt_index[j+1];
        }
        for (k=left; k<=right; k++) {
//            in[k]=(bwt_buf[i]<bwt_buf[j])?bwt_buf[i++]:bwt_buf[j--]; //----- sorted insertion
//          in[k]=(bwt_buf[i]<bwt_buf[j])?bwt_buf[i++]:bwt_buf[j--];
            if (bwt_buf[i]<bwt_buf[j]) {
                bwt_index[k] = bwt_index[BWT_BLOCKSIZE+i];
                in[k] = bwt_buf[i++];
            } else {
                bwt_index[k] = bwt_index[BWT_BLOCKSIZE+j];
                in[k] = bwt_buf[j--];
            }
        }
    }
}

#endif


//----- ----------------------------------------------------------------------------------------------------
//----- decompression functions step 3
//----- ----------------------------------------------------------------------------------------------------

//----- Expand data that was compressed with compr_step3_huffmann()
uint32_t CompressTool::uncompr_step3_huffmann(compr_bitStream *in, compr_bitStream *out, uint32_t size) {
    uint32_t i;
    //----- read counters
    for (i=0; i<256; i++) huff_nodes[i].count = in->data[in->aByte++];
    huff_nodes[HUFF_EOS].count = 1;
    //----- create huffmann tree
    int32_t root = compr_step3_huffmann_build_tree();
    //----- decode data using the created model
    int32_t node;
    for (;;) {
        node = root;
        do {
            if (compr_readBit(in)) node = huff_nodes[node].child2;
            else node = huff_nodes[node].child1;
        } while (node > HUFF_EOS);
        if (node == HUFF_EOS) break;
        out->data[out->aByte++] = node;
    }
    return out->aByte;
}

#ifdef COMPR_USE_ADHUFF
//----- Expand data that was compressed with compr_step3_ad_huffmann()
uint32_t CompressTool::uncompr_step3_adhuff(compr_bitStream *in, compr_bitStream *out, uint32_t size) {
    uint32_t c;
    compr_step3_adhuff_init_tree();
    while ((c = uncompr_step3_adhuff_decode_symbol(in)) != AD_HUFF_EOS) {
        out->data[out->aByte++] = c;
        compr_step3_adhuff_refresh_model(c);
    }
    return out->aByte;
}
#endif
#endif

//----- ----------------------------------------------------------------------------------------------------
//----- functions for debugging and measurement
//----- ----------------------------------------------------------------------------------------------------


//----- write compression statistics (time, byte sizes, relative and absolute percentages)
void CompressTool::compr_writeStatistics(uint32_t step, uint32_t size) {
    switch(step) {
        case 0:
#ifdef COMPR_DEBUG
            stats.ts_step0  = GET_TIME;
#endif
            stats.s_step0   = size;
            break;
        case 1:
#ifdef COMPR_DEBUG
            stats.ts_step1  = GET_TIME;
            stats.t_step1   = stats.ts_step1 - stats.ts_step0;
#endif
            stats.s_step1   = size;
            stats.spa_step1 = (1.0-(float)stats.s_step1/(float)stats.s_step0)*100.0;
            stats.spr_step1 = stats.spa_step1;
            break;
        case 2:
#ifdef COMPR_DEBUG
            stats.ts_step2  = GET_TIME;
            stats.t_step2   = stats.ts_step2 - stats.ts_step1;
#endif
            stats.s_step2   = size;
            stats.spa_step2 = (1.0-(float)stats.s_step2/(float)stats.s_step0)*100.0;
            stats.spr_step2 = (1.0-(float)stats.s_step2/(float)stats.s_step1)*100.0;
            break;
        case 3:
#ifdef COMPR_DEBUG
            stats.ts_step3  = GET_TIME;
            stats.t_step3   = stats.ts_step3 - stats.ts_step2;
#endif
            stats.s_step3   = size;
            stats.spa_step3 = (1.0-(float)stats.s_step3/(float)stats.s_step0)*100.0;
            stats.spr_step3 = (1.0-(float)stats.s_step3/(float)stats.s_step2)*100.0;
            break;
    }
}

#ifdef COMPR_UNCOMPR
//----- write decompression statistics (time)
void CompressTool::uncompr_writeStatistics(uint32_t step) {
#ifdef COMPR_DEBUG
    switch(step) {
        case 0:
            stats.ts_step0  = GET_TIME;
            stats.t_step0   = stats.ts_step0 - stats.ts_step1;
            break;
        case 1:
            stats.ts_step1  = GET_TIME;
            stats.t_step1   = stats.ts_step1 - stats.ts_step2;
            break;
        case 2:
            stats.ts_step2  = GET_TIME;
            stats.t_step2   = stats.ts_step2 - stats.ts_step3;
            break;
        case 3:
            stats.ts_step3  = GET_TIME;
            break;
    }
#endif
}
#endif
#ifdef COMPR_DEBUG
//----- returns compression statistics as string
int8_t* CompressTool::compr_getStatisticsStr() {
    sprintf(statStr, "durations: %d %d %d, bytes: %d %d %d %d, rel. size: %.2f %.2f %.2f\n",
        stats.t_step1, stats.t_step2, stats.t_step3, stats.s_step0, stats.s_step1, stats.s_step2, stats.s_step3, stats.spr_step1, stats.spr_step2, stats.spr_step3);
    return statStr;
}

//----- returns compression statistics as table
int8_t* CompressTool::compr_getStatisticsTable() {
    sprintf(statStr, "\nStatistiken:\t\tAnfang\t\tStufe 1\t\tStufe 2\t\tStufe 3\n--------------------------------------------------------------------------------\nZeit (ms):\t\t0\t\t%d\t\t%d\t\t%d\nGroesse (Byte):\t\t%d\t\t%d\t\t%d\t\t%d\nGroesse (absolut):\t0.00\t\t%.2f\t\t%.2f\t\t%.2f\nGroesse (relativ):\t0.00\t\t%.2f\t\t%.2f\t\t%.2f\n\n",
        stats.t_step1, stats.t_step2, stats.t_step3, stats.s_step0, stats.s_step1, stats.s_step2, stats.s_step3, stats.spa_step1, stats.spa_step2, stats.spa_step3, stats.spr_step1, stats.spr_step2, stats.spr_step3);
    return statStr;
}

#ifdef COMPR_UNCOMPR
//----- returns decompression statistics as string
int8_t* CompressTool::uncompr_getStatisticsStr() {
    sprintf(statStr, "DeCompress: durations: %d %d %d\n",
        stats.t_step2, stats.t_step1, stats.t_step0);
    return statStr;
}

//----- returns decompression statistics as table
int8_t* CompressTool::uncompr_getStatisticsTable() {
    sprintf(statStr, "\nStatistiken:\t\tAnfang\t\tStufe 3\t\tStufe 2\t\tStufe 1\n--------------------------------------------------------------------------------\nZeit (ms):\t\t0\t\t%d\t\t%d\t\t%d\n",
        stats.t_step2, stats.t_step1, stats.t_step0);
    return statStr;
}
#endif
#endif

//----- ----------------------------------------------------------------------------------------------------
//----- functions for accessing bit- and bytestreams
//----- ----------------------------------------------------------------------------------------------------


//----- reads one bit from the bitstream [bs]
bool CompressTool::compr_readBit(compr_bitStream *bs) {
    uint32_t output = bs->data[bs->aByte];
    output >>= (7-bs->aBit++);
    output &= 1;
    if (bs->aBit >= 8) { bs->aBit = 0; bs->aByte++; }
    return output;
}

//----- writes the lowest bit from [value] into the bitstream [bs]
void CompressTool::compr_writeBit(compr_bitStream *bs, bool value) {
    if (bs->aBit == 0) bs->data[bs->aByte] = 0;
    bs->data[bs->aByte] |= (value<<(7-bs->aBit++));
    if (bs->aBit >= 8) { bs->aBit = 0; bs->aByte++; }
}

//----- writes the [bits] lowest bit from [value] into the bitstream [bs] (max 32)
void CompressTool::compr_writeBitStream(compr_bitStream *bs, uint32_t value, uint32_t bits) {
    if (bs->aBit == 0) bs->data[bs->aByte] = 0;
    uint32_t nBits = 8-bs->aBit;
    if (nBits >= bits) {
        bs->data[bs->aByte] = ((bs->data[bs->aByte]) | (value << (nBits-bits)));
        bs->aBit += bits;
        bits = 0;
    } else  {
        bs->data[bs->aByte] = ((bs->data[bs->aByte]) | (value >> (bits-nBits)));
        bits-=nBits;
        bs->aBit = 8;
    }
    if (bs->aBit == 8) { bs->aByte++; bs->data[bs->aByte] = 0; bs->aBit = 0; }
    while (bits >= 8) {
        bits-=8;
        bs->data[bs->aByte] = (uint8_t) ((value >> bits)  & 0x000000FF);
        bs->aByte++; bs->data[bs->aByte] = 0;
    }
    if (bits > 0) {
        bs->data[bs->aByte] = (uint8_t) ((value << (8-bits)) & 0x000000FF);
        bs->aBit += bits;
    }
    if (bs->aBit == 8) { bs->aByte++; bs->data[bs->aByte] = 0; bs->aBit = 0; }
}

//----- returns [bits] bits from the bitstream [bs] (max 32)
uint32_t CompressTool::compr_readBitStream(compr_bitStream *bs, uint32_t bits) {
    if (bits == 0) return 0;
    uint32_t output = 0;
    uint8_t tmp = 0;
    uint32_t nBits = 8-bits-bs->aBit;
    if (bits <= (8-bs->aBit)) { //----- all bits in actual byte
        output = bs->data[bs->aByte];
        if (nBits > 0) output >>= nBits;
        output <<= 32-bits;
        output >>= 32-bits;
        if ((bs->aBit + bits) == 8) { bs->aByte++; bs->aBit = 0; }
        else bs->aBit += bits;
        bits = 0;
    } else {                    //----- read bits from first byte
        output = bs->data[bs->aByte];
        output <<= 24+bs->aBit;
        output >>= 24+bs->aBit;
        bits -= (8-bs->aBit);
        bs->aByte++; bs->aBit = 0;
    }
    while (bits >= 8) {         //----- read full bytes
        tmp = bs->data[bs->aByte++];
        bits -= 8;
        output <<= 8;
        output += tmp;
    }
    if (bits > 0) {             //----- read remaining bits
        tmp = bs->data[bs->aByte];
        output <<= bits;
        tmp >>= 8-bits;
        output += tmp;
        bs->aBit=bits;
    }
    return output;
}

//----- writes [bytes] bytes of [value] into the byte stream [bs] (max 4)
void CompressTool::compr_writeByteStream(compr_bitStream *bs, uint32_t value, uint32_t bytes) {
    while (bytes > 0) {
        bs->data[bs->aByte++] = (value >> ((bytes-1) << 3)) & 0xFF;
        bytes--;
    }
}

//----- reads [bytes] bytes from the byte stream [bs] (max 4)
uint32_t CompressTool::compr_readByteStream(compr_bitStream *bs, uint32_t bytes) {
    uint32_t output = 0;
    for (uint32_t i = 0; i<bytes; i++) {
        output <<= 8;
        output |= bs->data[bs->aByte++];
    }
    return output;
}


/*

//----- header of step 2
00  2   magic value (0x4D33, "M2")
02  4   Flags step 2
06  4   length uncompressed (= length compressed of step 1)
10  4   length compressed
14  ?   compressed data (contains also the compressed header of step 1)

//----- header of step 3
00  2   magic value (0x4D43, "MC")
02  4   flags step 3
06  4   length uncompressed (= length compressed of step 2)
10  4   length compressed
14  ?   compressed data (contains also the compressed headers of step 1 and 2)


*/
