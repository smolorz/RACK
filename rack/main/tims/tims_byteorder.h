/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either version 2 
 * of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg  <joerg.langenberg@gmx.net>
 *
 */
#ifndef __TIMS_BYTEORDER_H__
#define __TIMS_BYTEORDER_H__

#ifndef __KERNEL__

#include <endian.h>

#if BYTE_ORDER == LITTLE_ENDIAN

#undef  __BIG_ENDIAN_BITFIELD
#define __LITTLE_ENDIAN_BITFIELD

#else // __BYTE_ORDER == __LITTLE_ENDIAN

#define  __BIG_ENDIAN_BITFIELD
#undef   __LITTLE_ENDIAN_BITFIELD

#endif // __BYTE_ORDER == __LITTLE_ENDIAN

//
// float types
//
typedef float 	float32_t;
typedef double 	float64_t;

//
// swapping functions
//

static inline int64_t swap64(int64_t x)
{
    return((((u_int64_t)x & 0x00000000000000ffLLU) << 56) |
           (((u_int64_t)x & 0x000000000000ff00LLU) << 40) |
           (((u_int64_t)x & 0x0000000000ff0000LLU) << 24) |
           (((u_int64_t)x & 0x00000000ff000000LLU) <<  8) |
           (((u_int64_t)x & 0x000000ff00000000LLU) >>  8) |
           (((u_int64_t)x & 0x0000ff0000000000LLU) >> 24) |
           (((u_int64_t)x & 0x00ff000000000000LLU) >> 40) |
           (((u_int64_t)x & 0xff10000000000000LLU) >> 56));
}

static inline int32_t swap32(int32_t x)
{
    return((((u_int32_t)x & 0x000000ffU) << 24) |
           (((u_int32_t)x & 0x0000ff00U) <<  8) |
           (((u_int32_t)x & 0x00ff0000U) >>  8) |
           (((u_int32_t)x & 0xff000000U) >> 24));
}

static inline int16_t swap16(int16_t x)
{
    return((((u_int16_t)x & 0x00ffU) <<  8) |
           (((u_int16_t)x & 0xff00U) >>  8));
}


#ifdef __LITTLE_ENDIAN_BITFIELD

//
// --- BE to LE ---
//

static inline int64_t __be64_to_cpu(int64_t x)
{
    return(swap64(x));
}

static inline int32_t __be32_to_cpu(int32_t x)
{
    return(swap32(x));
}

static inline int16_t __be16_to_cpu(int16_t x)
{
    return(swap16(x));
}

static inline float64_t  __be64_float_to_cpu(float64_t x)
{
    int64_t i64 = *((int*)((void*)&x));
    i64         =  __be64_to_cpu(i64);
    x           = *((float64_t*)((void*)&i64));
    return (x);
}

static inline float32_t  __be32_float_to_cpu(float32_t x)
{
    int32_t i32 = *((int*)((void*)&x));
    i32         =  __be32_to_cpu(i32);
    x           = *((float32_t*)((void*)&i32));
    return (x);
}

//
// --- LE to LE ---
//

static inline int64_t __le64_to_cpu(int64_t x)
{
    return(x);
}

static inline int32_t __le32_to_cpu(int32_t x)
{
    return(x);
}

static inline int16_t __le16_to_cpu(int16_t x)
{
    return(x);
}

static inline float64_t  __le64_float_to_cpu(float64_t x)
{
    return(x);
}

static inline float32_t  __le32_float_to_cpu(float32_t x)
{
    return(x);
}

#else // ! __LITTLE_ENDIAN_BITFIELD

//
// --- BE to BE ---
//

static inline int64_t __be64_to_cpu(int64_t x){
  return(x);
}

static inline int32_t __be32_to_cpu(int32_t x){
  return(x);
}

static inline int16_t __be16_to_cpu(int16_t x){
  return(x);
}

static inline float64_t  __be64_float_to_cpu(float64_t x)
{
    return(x);
}

static inline float32_t  __be32_float_to_cpu(float32_t x)
{
    return(x);
}

//
// --- LE to BE ---
//

static inline int64_t __le64_to_cpu(int64_t x)
{
  return(swap64(x));
}

static inline int32_t __le32_to_cpu(int32_t x)
{
  return(swap32(x));
}

static inline int16_t __le16_to_cpu(int16_t x)
{
  return(swap16(x));
}

static inline float64_t  __le64_float_to_cpu(float64_t x)
{
    int64_t i64 = *((int*)((void*)&x));
    i64         =  __le64_to_cpu(i64);
    x           = *((float64_t*)((void*)&i64));
    return (x);
}

static inline float32_t  __le32_float_to_cpu(float32_t x)
{
    int32_t i32 = *((int*)((void*)&x));
    i32         =  __le32_to_cpu(i32);
    x           = *((float32_t*)((void*)&i32));
    return (x);
}

#endif // ! __LITTLE_ENDIAN_BITFIELD

static inline float64_t __cpu_to_be64_float(int64_t x)
{
    return(__be64_float_to_cpu(x));
}

static inline float32_t __cpu_to_be32_float(int32_t x)
{
    return(__be32_float_to_cpu(x));
}

static inline int32_t __cpu_to_be32(int32_t x)
{
    return(__be32_to_cpu(x));
}

static inline int32_t __cpu_to_le32(int32_t x)
{
    return(__le32_to_cpu(x));
}

static inline int16_t __cpu_to_be16(int16_t x)
{
    return(__be16_to_cpu(x));
}

static inline int16_t __cpu_to_le16(int16_t x)
{
    return(__le16_to_cpu(x));
}

#endif // __KERNEL__

#endif // __TIMS_BYTEORDER_H__
