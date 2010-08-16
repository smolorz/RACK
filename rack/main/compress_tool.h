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
#ifndef __COMPRESS_TOOL_H__
#define __COMPRESS_TOOL_H__

#include <perception/scan3d_proxy.h>


//----- ----------------------------------------------------------------------------------------------------
//----- compile switches
//----- The following #defines are uses to include/exclude some parts of the code.
//----- This is done because every algotithm needs some memory
//----- which would be wasted if the specific algorithm isn't used
//----- So turn off unused algorithms...
//----- ----------------------------------------------------------------------------------------------------

//----- COMPR_DEBUG enables time measurement and debug output. This is for testing an debugging
//----- only and should be disabled otherwise.
//#define COMPR_DEBUG

//----- COMPR_UNCOMPR enables the decompression functions. If just compression is needed
//----- (and decompression is done in ScanView3d in Java), turn it off.
//#define COMPR_UNCOMPR

//----- COMPR_USE_LZSS includes the LZSS algorithm. Turn it off, if you don't want to use it.
//#define COMPR_USE_LZSS

//----- COMPR_USE_LZW includes the LZW & LZWV algorithms. Turn it off, if you don't want to use them.
//#define COMPR_USE_LZW

//----- COMPR_USE_LZSS includes the Burrows Wheeler Transform (BWT).
//----- The BWT is very slow and does not work properlay, so it has not been optimised.
//----- So this section of the code ist EXPERIMENTAL.
//----- Turn it off, if you don't want to use it.
//#define COMPR_USE_BWT

//----- COMPR_USE_ADHUFF includes the adaptive Huffmann algorithm. Turn it off, if you don't want to use it.
//#define COMPR_USE_ADHUFF


#define COMPR_S2_NONE         0x00
#define COMPR_S2_LZSS         0x21
#define COMPR_S2_LZW          0x22
#define COMPR_S2_LZWV         0x23
#define COMPR_S2_BWT          0x24
#define COMPR_S2_MTF          0x25
#define COMPR_S2_MTF_LZSS     0x26
#define COMPR_S2_BWT_MTF      0x27
#define COMPR_S3_NONE         0x00
#define COMPR_S3_HUFFMANN     0x31
#define COMPR_S3_AD_HUFFMANN  0x32

#define COMPR_S2_LZSS_PARAM(INDEX_BITS, LEN_BITS) (((INDEX_BITS)<<24)|((LEN_BITS)<<16)|S2_LZSS)
#define COMPR_S2_LZW_PARAM(BITS) (((BITS)<<24)|S2_LZW)
#define COMPR_S2_LZWV_PARAM(MAX_BITS) (((MAX_BITS)<<24)|S2_LZWV)
#define COMPR_S2_BWT_PARAM(BLOCK_SIZE) (((BLOCK_SIZE)<<8)|S2_BWT)

#define GET_TIME rackTime->get()
#define timestamp rack_time_t

#define COMPR_MAX_INPUT_SIZE (int32_t)(SCAN3D_POINT_MAX*sizeof(scan_point)*1.1)


#define LZSS_MAX_INDEX_BITS 16  //----- determined optimum: 6
#define LZSS_MAX_LEN_BITS   5   //----- determined optimum: 4
#define LZSS_MAX_WINDOW_SIZE    (1<<LZSS_MAX_INDEX_BITS)
//----- The following values are used, if no parameters or 0 is given.
#define LZSS_INDEX_BITS     6   //----- determined optimum: 6
#define LZSS_LEN_BITS       4   //----- determined optimum: 4

#define LZSS_EOS            0
#define LZSS_UNUSED         0
#define LZSS_MOD_WND(a)     ((a)&(lzss_window_size-1))

#define LZW_BITS            11
#define LZW_MAX_CODE        ((1<<LZW_BITS)-1)
#define LZW_TABLE_SIZE      5021        //----- Needs to be a prime number of at least 120% the size of 2^LZW_BITS.
//----- bits   9   10   11   12    13    14    15    16
//----- size 631 1277 2521 5021 10079 20161 35023 80611
#define LZW_EOS             256
#define LZW_RISE_CODE       LZW_EOS+1
#define LZW_CLEAR_TABLE     LZW_EOS+2
#define LZW_FIRST_CODE      LZW_EOS+3
#define LZW_UNUSED          -1
#define LZWV_MAX_BITS       11

#define HUFF_EOS            256
#define AD_HUFF_EOS         256
#define AD_HUFF_ESCAPE      257
#define AD_HUFF_N_SYMBOLS   258
#define AD_HUFF_N_NODES     ((AD_HUFF_N_SYMBOLS<<1)-1)
#define AD_HUFF_ROOT        0
#define AD_HUFF_MAX_WEIGHT  0x8000
#define AD_HUFF_UNUSED      -1

#define BWT_BLOCKSIZE       11


#include <main/rack_gdos.h>

//----- ----------------------------------------------------------------------------------------------------
//----- Class CompressTool
//----- ----------------------------------------------------------------------------------------------------

//----- compr_statistics contains statistics of the compression
typedef struct compr_statistics {
    timestamp    ts_step0, ts_step1,  ts_step2,  ts_step3;  //----- time stamp
    uint32_t      t_step0,  t_step1,   t_step2,   t_step3;  //----- time differences
    uint32_t      s_step0,  s_step1,   s_step2,   s_step3;  //----- sizes in bytes
    float                 spr_step1, spr_step2, spr_step3;  //----- sizes in percentages realtive
    float                 spa_step1, spa_step2, spa_step3;  //----- sizes in percentages absolute
} compr_statistics;

//----- compr_bitStream
typedef struct compr_bitStream {
    uint8_t  *data;     //----- pointer to data
    uint32_t aByte;     //----- actual byte
    uint32_t aBit;      //----- actual bit in actual byte
} compr_bitStream;

//----- compr_byteStream
typedef struct compr_byteStream {
    uint8_t  *data;     //----- pointer to data
    uint32_t aByte;     //----- actual byte
} compr_byteStream;

//----- for Huffmann coding
typedef struct huff_node {
    uint32_t count;
    uint32_t child1;
    uint32_t child2;
} huff_node;

typedef struct huff_code {
    uint32_t code;
    uint32_t len;
} huff_code;

#ifdef COMPR_USE_ADHUFF
//----- for adaptive Huffmann coding
typedef struct adhuff_node {
    uint32_t weight;
    int32_t parent;
    bool child_is_leaf;
    int32_t child;
} adhuff_node;

typedef struct adhuff_tree_t {
    int32_t leaf[AD_HUFF_N_SYMBOLS];
    int32_t next_free;
    adhuff_node node[AD_HUFF_N_NODES];
} adhuff_tree_t;
#endif

//----- for LZSS
#ifdef COMPR_USE_LZSS
typedef struct lzss_node {
    int32_t parent;
    int32_t child_small;
    int32_t child_large;
} lzss_node;
#endif

#ifdef COMPR_USE_LZW
//----- für LZW
typedef struct lzw_node {
    int32_t code;
    int32_t parent;
    int8_t  c;
} lzw_node;
#endif

class CompressTool {
//----- ----------------------------------------------------------------------------------------------------
//----- global variables
//----- ----------------------------------------------------------------------------------------------------
  private:
    RackGdos            *gdos; //----- only for debugging

    uint8_t             *pbuf23;
    uint8_t             buf23[COMPR_MAX_INPUT_SIZE];
#ifdef COMPR_DEBUG
    int8_t              statStr[350];                   //----- for statistic output
#endif

    //----- for Huffmann coding
    uint32_t            huff_counters[256];
    huff_node           huff_nodes[514];
    huff_code           huff_codes[257];
#ifdef COMPR_USE_ADHUFF
    //----- für adaptive Huffmann-Komprimierung
    adhuff_tree_t       adhuff_tree;
#endif
#ifdef COMPR_USE_BWT
    //----- for BWT
    uint8_t             bwt_buf[COMPR_MAX_INPUT_SIZE*2];
    uint8_t*            bwt_index[COMPR_MAX_INPUT_SIZE];
#endif
    //----- for MTF
    uint8_t             mtf_alphabet[255];
#ifdef COMPR_USE_LZSS
    //----- for LZSS
    uint32_t            lzss_cost_benefit;
    uint32_t            lzss_window_size;
    uint32_t            lzss_pre_wnd_size;
    uint32_t            lzss_tree_root;
    lzss_node           lzss_tree[LZSS_MAX_WINDOW_SIZE+1];
    uint8_t             lzss_window[LZSS_MAX_WINDOW_SIZE];
#endif
#ifdef COMPR_USE_LZW
    //----- for LZW
    lzw_node            lzw_table[LZW_TABLE_SIZE];
    int8_t              lzw_stack[LZW_TABLE_SIZE];
    uint32_t            lzw_next_code;
    uint32_t            lzw_akt_bits;
    uint32_t            lzw_next_rise;
#endif

  public:
    compr_statistics    stats;
    RackTime            *rackTime;

//----- ----------------------------------------------------------------------------------------------------
//----- function prototypes
//----- ----------------------------------------------------------------------------------------------------
    CompressTool();
    CompressTool(RackMailbox * p_mbx, int32_t gdos_level);
    ~CompressTool();
    int8_t*  compr_getStatisticsStr();
    int8_t*  compr_getStatisticsTable();
    void     compr_writeStatistics(uint32_t step, uint32_t size);
    void     compr_writeBit(compr_bitStream *bs, bool value);
    bool     compr_readBit(compr_bitStream *bs);
    void     compr_writeBitStream(compr_bitStream *bs, uint32_t value, uint32_t bits);
    uint32_t compr_readBitStream(compr_bitStream *bs, uint32_t bits);
    void     compr_writeByteStream(compr_bitStream *bs, uint32_t value, uint32_t bytes);
    uint32_t compr_readByteStream(compr_bitStream *bs, uint32_t bytes);

    uint32_t Compress(uint8_t *in_out, uint32_t flags_s2, uint32_t flags_s3);
    uint32_t compr_step2_lzss(compr_bitStream *in, compr_bitStream *out, uint32_t size, uint32_t index_bits, uint32_t len_bits);
    void     compr_step2_lzss_init_tree(int32_t r);
    int32_t  compr_step2_lzss_add_str(int32_t new_node, int32_t *hit_pos);
    void     compr_step2_lzss_remove_str(int32_t p);
    void     compr_step2_lzss_combine_nodes(int32_t old_node, int32_t new_node);
    void     compr_step2_lzss_replace_nodes(int32_t old_node, int32_t new_node);
    int32_t  compr_step2_lzss_search_next_node(int32_t node);
    uint32_t compr_step2_lzw(compr_bitStream *in, compr_bitStream *out, uint32_t size, uint32_t bits);
    uint32_t compr_step2_lzw_find_child(int32_t parent, uint8_t child);
    uint32_t compr_step2_lzwv(compr_bitStream *in, compr_bitStream *out, uint32_t size, uint32_t max_bits);
    void     compr_step2_lzwv_init_table();
    uint32_t compr_step2_mtf(compr_bitStream *in, compr_bitStream *out, uint32_t size);
    uint32_t compr_step2_bwt(compr_bitStream *in, compr_bitStream *out, uint32_t size, uint32_t blocksize);
    int32_t  compr_step2_bwt_strcmpn(uint8_t *a, uint8_t *b, int32_t size);
    void     compr_step2_bwt_quicksort(int32_t n, int32_t l, int32_t r);

    uint32_t compr_step3_huffmann(compr_bitStream *in, compr_bitStream *out, uint32_t size);
    uint32_t compr_step3_adhuff(compr_bitStream *in, compr_bitStream *out, uint32_t size);
    void     compr_step3_huffmann_tree2codes(uint32_t code_so_far, int32_t bits, int32_t node);
    uint32_t compr_step3_huffmann_build_tree();
    void     compr_step3_adhuff_init_tree();
    void     compr_step3_adhuff_build_tree();
    void     compr_step3_adhuff_swap_nodes(int32_t i, int32_t j);
    void     compr_step3_adhuff_add_node(int32_t c);
    void     compr_step3_adhuff_refresh_model(int32_t c);
    void     compr_step3_adhuff_code_symbol(uint32_t c, compr_bitStream *out);

#ifdef COMPR_UNCOMPR
    uint32_t Decompress(uint8_t *in_out);

    int8_t*  uncompr_getStatisticsStr();
    int8_t*  uncompr_getStatisticsTable();
    void     uncompr_writeStatistics(uint32_t step);

    uint32_t uncompr_step2_lzss(compr_bitStream *in, compr_bitStream *out, uint32_t index_bits, uint32_t len_bits);
    uint32_t uncompr_step2_lzw(compr_bitStream *in, compr_bitStream *out, uint32_t bits);
    uint32_t uncompr_step2_lzw_decode_string(int32_t count, int32_t code);
    uint32_t uncompr_step2_lzwv(compr_bitStream *in, compr_bitStream *out, uint32_t max_bits);
    uint32_t uncompr_step2_mtf(compr_bitStream *in, compr_bitStream *out, uint32_t size);
    uint32_t uncompr_step2_bwt(compr_bitStream *in, compr_bitStream *out, uint32_t size, uint32_t blocksize);
    void     uncompr_step2_bwt_mergesort(uint8_t *in, int32_t left, int32_t right);
    uint32_t uncompr_step3_huffmann(compr_bitStream *in, compr_bitStream *out, uint32_t size);
    uint32_t uncompr_step3_adhuff(compr_bitStream *in, compr_bitStream *out, uint32_t size);
    uint32_t uncompr_step3_adhuff_decode_symbol(compr_bitStream *in);
#endif
};

#endif // __COMPRESS_TOOL_H__
