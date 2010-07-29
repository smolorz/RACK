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
 *      Oliver Wulf <oliver.wulf@web.de>
 *      Sebastian Smolorz <smolorz@rts.uni-hannover.de>
 *
 */
#ifndef __TIMS_API_H__
#define __TIMS_API_H__

#include <main/tims/tims.h>

//
// api defines
//

#define TIMS_INFINITE               (0)
#define TIMS_NONBLOCK               ((int64_t)-1)

//
// includes for xenomai, xenomai/kernel and linux
//

#if defined (__XENO__) || defined (__KERNEL__)

#include <rtdm/rtdm.h>
#include <main/tims/tims_rtdm.h>

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

#else // !__XENO__ && !__KERNEL__

#include <sys/uio.h>
#include <errno.h>

#endif // __XENO__ || __KERNEL__

//
// api functions
//

#ifdef __cplusplus
extern "C" {
#endif

int tims_socket(void);

int tims_close(int fd);

int tims_bind(int fd, uint32_t address);

int tims_set_timeout(int fd, int64_t timeout_ns);

ssize_t tims_sendmsg(int fd, tims_msg_head *p_head, struct iovec *vec,
                     unsigned char veclen, int timsflags);

int tims_recvmsg_timed(int fd, tims_msg_head *p_head, void *p_data,
                       ssize_t maxdatalen, int64_t timeout_ns, int timsflags);

// creates a mailbox
int tims_mbx_create(uint32_t address, int messageSlots, ssize_t messageSize,
                    void *buffer, ssize_t buffer_size);

// deletes a mailbox
int tims_mbx_remove(int fd);

// delete all messages
int tims_mbx_clean(int fd, int addr);

// waiting a specific time for a new message
// -> returns a pointer to a tims_msg_head if successful
// -> locks the message
int tims_peek_timed(int fd, tims_msg_head **pp_head, int64_t timeout_ns);

// unlock the message, whitch is locked before
int tims_peek_end(int fd);

#ifdef __cplusplus
}
#endif

#endif // __TIMS_API_H__
