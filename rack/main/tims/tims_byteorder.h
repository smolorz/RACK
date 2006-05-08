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
#include <byteswap.h>

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
typedef float   float32_t;
typedef double  float64_t;


#ifdef __LITTLE_ENDIAN_BITFIELD

//
// --- BE to LE ---
//

static inline int64_t __be64_to_cpu(int64_t x)
{
    return bswap_64(x);
}

static inline int32_t __be32_to_cpu(int32_t x)
{
    return bswap_32(x);
}

static inline int16_t __be16_to_cpu(int16_t x)
{
    return bswap_16(x);
}

static inline float64_t __be64_float_to_cpu(float64_t x)
{
    uint64_t volatile tmp;

    tmp = bswap_64(*((uint64_t *)&x));
    return *((float64_t *)&tmp);
}

static inline float32_t __be32_float_to_cpu(float32_t x)
{
    uint32_t volatile tmp;

    tmp =  bswap_32(*((uint32_t *)&x));
    return *((float32_t *)&tmp);
}

//
// --- LE to LE ---
//

static inline int64_t __le64_to_cpu(int64_t x)
{
    return x;
}

static inline int32_t __le32_to_cpu(int32_t x)
{
    return x;
}

static inline int16_t __le16_to_cpu(int16_t x)
{
    return x;
}

static inline float64_t  __le64_float_to_cpu(float64_t x)
{
    return x;
}

static inline float32_t  __le32_float_to_cpu(float32_t x)
{
    return x;
}

#else // ! __LITTLE_ENDIAN_BITFIELD

//
// --- BE to BE ---
//

static inline int64_t __be64_to_cpu(int64_t x)
{
    return x;
}

static inline int32_t __be32_to_cpu(int32_t x)
{
    return x;
}

static inline int16_t __be16_to_cpu(int16_t x)
{
    return x;
}

static inline float64_t  __be64_float_to_cpu(float64_t x)
{
    return x;
}

static inline float32_t  __be32_float_to_cpu(float32_t x)
{
    return x;
}

//
// --- LE to BE ---
//

static inline int64_t __le64_to_cpu(int64_t x)
{
    return bswap_64(x);
}

static inline int32_t __le32_to_cpu(int32_t x)
{
    return bswap_32(x);
}

static inline int16_t __le16_to_cpu(int16_t x)
{
    return bswap_16(x);
}

static inline float64_t __le64_float_to_cpu(float64_t x)
{
    uint64_t volatile tmp;

    tmp = bswap_64(*((uint64_t *)&x));
    return *((float64_t *)&tmp);
}

static inline float32_t __le32_float_to_cpu(float32_t x)
{
    uint32_t volatile tmp;

    tmp =  bswap_32(*((uint32_t *)&x));
    return *((float32_t *)&tmp);
}

#endif // ! __LITTLE_ENDIAN_BITFIELD

static inline float64_t __cpu_to_be64_float(float64_t x)
{
    return __be64_float_to_cpu(x);
}

static inline float32_t __cpu_to_be32_float(float32_t x)
{
    return __be32_float_to_cpu(x);
}

static inline int32_t __cpu_to_be32(int32_t x)
{
    return __be32_to_cpu(x);
}

static inline int32_t __cpu_to_le32(int32_t x)
{
    return __le32_to_cpu(x);
}

static inline int16_t __cpu_to_be16(int16_t x)
{
    return __be16_to_cpu(x);
}

static inline int16_t __cpu_to_le16(int16_t x)
{
    return __le16_to_cpu(x);
}

#endif // __KERNEL__

#endif // __TIMS_BYTEORDER_H__
