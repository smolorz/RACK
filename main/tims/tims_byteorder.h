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

#ifdef __KERNEL__

#include <asm/byteorder.h>

#else  /* !__KERNEL__ */

#include <endian.h>
#include <byteswap.h>

#if BYTE_ORDER == LITTLE_ENDIAN

#undef  __BIG_ENDIAN_BITFIELD
#define __LITTLE_ENDIAN_BITFIELD

#else // __BYTE_ORDER == __LITTLE_ENDIAN

#define  __BIG_ENDIAN_BITFIELD
#undef   __LITTLE_ENDIAN_BITFIELD

#endif // __BYTE_ORDER == __LITTLE_ENDIAN

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

static inline double __be64_float_to_cpu(double x)
{
    union {
        uint64_t  raw;
        double flt;
    } u;

    u.flt = x;
    u.raw = bswap_64(u.raw);
    return u.flt;
}

static inline float __be32_float_to_cpu(float x)
{
    union {
        uint32_t  raw;
        float flt;
    } u;

    u.flt = x;
    u.raw = bswap_32(u.raw);
    return u.flt;
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

static inline double  __le64_float_to_cpu(double x)
{
    return x;
}

static inline float  __le32_float_to_cpu(float x)
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

static inline double  __be64_float_to_cpu(double x)
{
    return x;
}

static inline float  __be32_float_to_cpu(float x)
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

static inline double __le64_float_to_cpu(double x)
{
    union {
        uint64_t  raw;
        double flt;
    } u;

    u.flt = x;
    u.raw = bswap_64(u.raw);
    return u.flt;
}

static inline float __le32_float_to_cpu(float x)
{
    union {
        uint32_t  raw;
        float flt;
    } u;

    u.flt = x;
    u.raw = bswap_32(u.raw);
    return u.flt;
}

#endif // ! __LITTLE_ENDIAN_BITFIELD

static inline double __cpu_to_be64_float(double x)
{
    return __be64_float_to_cpu(x);
}

static inline float __cpu_to_be32_float(float x)
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

#endif // !__KERNEL__

#endif // __TIMS_BYTEORDER_H__
