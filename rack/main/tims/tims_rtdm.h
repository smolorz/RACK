/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2009 University of Hannover
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
 *      Marko Reimer <reimer@rts.uni-hannover.de>
 *      Jan Kiszka <kiszka@rts.uni-hannover.de>
 *      Oliver Wulf <oliver.wulf@web.de>
 *      Sebastian Smolorz <smolorz@rts.uni-hannover.de>
 */
#ifndef __TIMS_RTDM_H__
#define __TIMS_RTDM_H__

#include <main/tims/tims.h>

#include <rtdm/rtdm.h>

#ifdef __KERNEL__

#include <linux/ioctl.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/errno.h>

#else  /* !__KERNEL__ */

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>

#endif /* !__KERNEL__ */

//
// TIMS rtdm socket api
//

// socket protocol family
#ifndef PF_TIMS
#define PF_TIMS                     27
#endif

typedef struct
{
  sa_family_t sa_family;
  uint32_t    id;
} tims_sockaddr;

#define RTDM_SUBCLASS_TIMS          0

// min fifo mailbox size
#define TIMS_MIN_FIFOMBX_SIZE       TIMS_HEADLEN
#define TIMS_STD_FIFOMBX_SIZE       ( 3 * TIMS_HEADLEN )

//
// ioctl requests and structs
//

typedef struct
{
    uint32_t    msg_size;        // max size per message
    uint32_t    slot_count;      // 0  => FIFO Queuing
                                  // >0 => Priority Queuing
    void        *buffer;         // NULL: kernel-located
    uint32_t    buffer_size;
} tims_mbx_cfg;

typedef struct
{
    uint64_t     utc_timestamp;
    uint64_t     rec_timestamp;
} tims_clock_setvalue;

#define RTIOC_TYPE_TIMS             RTDM_CLASS_NETWORK

#define TIMS_RTIOC_MBXCFG          _IOW(RTIOC_TYPE_TIMS, 0x00,        \
                                        tims_mbx_cfg)

#define TIMS_RTIOC_MBXCLEAN        _IOW(RTIOC_TYPE_TIMS, 0x01,        \
                                        int)

#define TIMS_RTIOC_RECVBEGIN       _IOW(RTIOC_TYPE_TIMS, 0x02,        \
                                        tims_msg_head)

#define TIMS_RTIOC_RECVEND         _IOW(RTIOC_TYPE_TIMS, 0x03,        \
                                        int)

#define TIMS_RTIOC_TIMEOUT         _IOW(RTIOC_TYPE_TIMS, 0x04,        \
                                        nanosecs_rel_t)

#define TIMS_RTIOC_GETCLOCKOFFSET  _IOW(RTIOC_TYPE_TIMS, 0x05,        \
                                        nanosecs_rel_t)

#define TIMS_RTIOC_GETTIME         _IOW(RTIOC_TYPE_TIMS, 0x06,        \
                                        nanosecs_abs_t)

#define TIMS_RTIOC_SETTIME         _IOW(RTIOC_TYPE_TIMS, 0x07,        \
                                        tims_clock_setvalue)

#endif // __TIMS_RTDM_H__
