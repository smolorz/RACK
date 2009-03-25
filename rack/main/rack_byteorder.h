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
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#ifndef __RACK_DEFINES_H__
#define __RACK_DEFINES_H__

#include <main/tims/tims_byteorder.h>

/* supports :
 *
int64_t swap64(int32_t x)
int32_t swap32(int32_t x)
int16_t swap16(int16_t x)

// little endian //

int16_t __le16_to_cpu(int16_t x)
int32_t __le32_to_cpu(int32_t x)
int64_t __le64_to_cpu(int64_t x)

int16_t __cpu_to_le16(int16_t x)
int32_t __cpu_to_le32(int32_t x)
int64_t __cpu_to_le64(int64_t x)

float32_t   __le32_float_to_cpu(float32_t x)
float64_t   __le64_float_to_cpu(float64_t x)

// big endian //

int16_t __be16_to_cpu(int16_t x)
int32_t __be32_to_cpu(int32_t x)
int64_t __be64_to_cpu(int64_t x)

int16_t __cpu_to_be16(int16_t x)
int32_t __cpu_to_be32(int32_t x)
int64_t __cpu_to_be64(int64_t x)

float32_t   __be32_float_to_cpu(float32_t x)
float64_t   __be64_float_to_cpu(float64_t x)

*/

#endif // __RACK_DEFINES_H__
