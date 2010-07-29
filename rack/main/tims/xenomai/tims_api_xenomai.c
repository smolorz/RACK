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

#include <main/tims/tims.h>
#include <main/tims/tims_rtdm.h>
#include <main/tims/tims_api.h>

#ifdef __cplusplus
extern "C" {
#endif

int tims_socket(void)
{
    return rt_dev_socket(PF_TIMS, SOCK_RAW, 0);
}

int tims_close(int fd)
{
    return rt_dev_close(fd);
}

int tims_bind(int fd, uint32_t address)
{
    tims_sockaddr tsock = {
        sa_family:  PF_TIMS,
        id:         address
    };

    struct _rtdm_setsockaddr_args args = {
        addr:       (struct sockaddr *)&tsock,
        addrlen:    sizeof(tims_sockaddr)
    };

    return rt_dev_ioctl(fd, _RTIOC_BIND, &args);
}

int tims_set_timeout(int fd, int64_t timeout_ns)
{
    return rt_dev_ioctl(fd, TIMS_RTIOC_TIMEOUT, &timeout_ns);
}

ssize_t tims_sendmsg(int fd, tims_msg_head *p_head, struct iovec *vec,
                     unsigned char veclen, int timsflags)
{
    unsigned char vec_nr = veclen + 1; // add head
    int i = 0;
    struct iovec iov[vec_nr];
    struct msghdr msg = {NULL, 0, iov , vec_nr, NULL, 0, 0};

    // add messagehead to vector list
    iov[0].iov_base = (void *)p_head;
    iov[0].iov_len  = TIMS_HEADLEN;

    // copy other vectors
    for (i=0; i < veclen; i++)
    {
        iov[i+1].iov_base = vec[i].iov_base;
        iov[i+1].iov_len  = vec[i].iov_len;
    }

    return rt_dev_sendmsg(fd, &msg, timsflags);
}

int tims_recvmsg_timed(int fd, tims_msg_head *p_head, void *p_data,
                       ssize_t maxdatalen, int64_t timeout_ns, int timsflags)
{
    struct iovec iov[2];
    // sending timeout in msghdr.msg_control
    struct msghdr msg = { NULL, 0, iov, 2, &timeout_ns, 0, 0 };

    // add messagehead to vector list
    iov[0].iov_base = (void *)p_head;
    iov[0].iov_len  = (ssize_t)TIMS_HEADLEN;

    // copy data pointer
    iov[1].iov_base = p_data;
    iov[1].iov_len  = maxdatalen;

    return rt_dev_recvmsg(fd, &msg, timsflags);
}

// creates a mailbox
int tims_mbx_create(uint32_t address, int messageSlots, ssize_t messageSize,
                    void *buffer, ssize_t buffer_size)
{
    int ret = 0;
    int fd  = -1;

    tims_mbx_cfg tmc = {
        msg_size:       messageSize,
        slot_count:     messageSlots,
        buffer:         buffer,
        buffer_size:    buffer_size
    };

    // small checks
    if (buffer &&
        messageSlots > 0 &&
        buffer_size < (ssize_t)(messageSlots * messageSize) )
    {
        return -EINVAL;
    }

    if ( buffer &&
        !messageSlots &&
        buffer_size < (ssize_t)TIMS_MIN_FIFOMBX_SIZE )
    {
        return -EINVAL;
    }

    // init socket
    ret = tims_socket(); // get rtdm descriptor
    if (ret < 0)
        return ret;

    fd = ret;

    // bind
    ret = tims_bind(fd, address);
    if (ret)
    {
        tims_close(fd);
        fd = -1;
        return ret;
    }

    // create mailbox
    ret = rt_dev_ioctl(fd, TIMS_RTIOC_MBXCFG, &tmc);
    if (ret)
    {
        tims_close(fd);
        fd = -1;
        return ret;
    }

    return fd;
}

// deletes a mailbox
int tims_mbx_remove(int fd)
{
    return rt_dev_close(fd);
}


// delete all messages
int tims_mbx_clean(int fd, int addr)
{
    return rt_dev_ioctl(fd, TIMS_RTIOC_MBXCLEAN, 0);
}

// waiting a specific time for a new message
// -> returns a pointer to a tims_msg_head if successful
// -> locks the message
int tims_peek_timed(int fd, tims_msg_head **pp_head, int64_t timeout_ns)
{
    // set timeout
    int ret = tims_set_timeout(fd, timeout_ns);
    if (ret)
        return ret;

    return rt_dev_ioctl(fd, TIMS_RTIOC_RECVBEGIN, pp_head);
}

// unlock the message, whitch is locked before
int tims_peek_end(int fd)
{
    return rt_dev_ioctl(fd, TIMS_RTIOC_RECVEND, NULL);
}

#ifdef __cplusplus
}
#endif
