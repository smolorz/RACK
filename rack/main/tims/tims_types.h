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
 *      Oliver Wulf <oliver.wulf@web.de>
 *
 */
#ifndef _TIMS_TYPES_H_
#define _TIMS_TYPES_H_

// define int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t
// define float (32bit), double (64bit)
// define size_t, ssize_t, uid_t, gid_t, ...

#ifdef __KERNEL__

#include <linux/types.h>

#else  /* !__KERNEL__ */

#include <inttypes.h>
#include <unistd.h>

#endif /* !__KERNEL__ */

#endif // __TIMS_TYPES_H_
