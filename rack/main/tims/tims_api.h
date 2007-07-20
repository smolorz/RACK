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
 *      Oliver Wulf <oliver.wulf@web.de>
 *
 */
#ifndef __TIMS_API_H__
#define __TIMS_API_H__

#define TIMS_INFINITE               (0)
#define TIMS_NONBLOCK               ((int64_t)-1)

#ifdef __cplusplus
extern "C" {
#endif

static inline void tims_set_head_byteorder(tims_msg_head* p)
{
  #if defined __BIG_ENDIAN_BITFIELD
      p->flags &= ~TIMS_HEAD_BYTEORDER_LE;
  #elif defined __LITTLE_ENDIAN_BITFIELD
      p->flags |= TIMS_HEAD_BYTEORDER_LE;
  #else
      #error There is no byteorder defined. Fix <main/tims/tims_byteorder.h>.
  #endif
}

static inline void tims_set_body_byteorder(tims_msg_head* p)
{
    #if defined __BIG_ENDIAN_BITFIELD
        p->flags &= ~TIMS_BODY_BYTEORDER_LE;
    #elif defined __LITTLE_ENDIAN_BITFIELD
        p->flags |= TIMS_BODY_BYTEORDER_LE;
    #else
        #error There is no byteorder defined. Fix <main/tims/tims_byteorder.h>.
    #endif
}

static inline void tims_set_byteorder(tims_msg_head* p)
{
    tims_set_head_byteorder(p);
    tims_set_body_byteorder(p);
}

static inline void tims_parse_head_byteorder(tims_msg_head* p)
{
    if (p->flags & TIMS_HEAD_BYTEORDER_LE)  /* head is little endian */
    {
        p->src     = __le32_to_cpu(p->src);
        p->dest    = __le32_to_cpu(p->dest);
        p->msglen  = __le32_to_cpu(p->msglen);
    }
    else                                    /* head is big endian */
    {
        p->src     = __be32_to_cpu(p->src);
        p->dest    = __be32_to_cpu(p->dest);
        p->msglen  = __be32_to_cpu(p->msglen);
    }
    tims_set_head_byteorder(p);
}

static inline void tims_fill_head(tims_msg_head *p_head, int8_t type,
                                  uint32_t dest, uint32_t src,
                                  uint8_t priority, uint8_t seq_nr, uint8_t flags,
                                  uint32_t msglen)
{
    p_head->flags    = flags;
    p_head->type     = type;
    p_head->priority = priority;
    p_head->seq_nr   = seq_nr;
    p_head->dest     = dest;
    p_head->src      = src;
    p_head->msglen   = msglen;

    tims_set_byteorder(p_head);
}

#ifdef __cplusplus
}
#endif

//
// tims rtdm api
//

#if defined (__XENO__) || defined (__KERNEL__)

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

#define TIMS_ALLOW_KERNEL_TASKS                0

//
// TIMS socket
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

//
// functions
//

#ifdef __cplusplus
extern "C" {
#endif

static inline int tims_socket(void)
{
    return rt_dev_socket(PF_TIMS, SOCK_RAW, 0);
}

static inline int tims_close(int fd)
{
    return rt_dev_close(fd);
}

static inline int tims_bind(int fd, uint32_t address)
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

static inline int tims_set_timeout(int fd, int64_t timeout_ns)
{
    return rt_dev_ioctl(fd, TIMS_RTIOC_TIMEOUT, &timeout_ns);
}

static inline ssize_t tims_sendmsg(int fd, tims_msg_head *p_head,
                                   struct iovec *vec, unsigned char veclen,
                                   int timsflags)
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

static inline int tims_recvmsg_timed(int fd, tims_msg_head *p_head, void *p_data,
                                     ssize_t maxdatalen, int64_t timeout_ns,
                                     int timsflags)
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
static inline int tims_mbx_create(uint32_t address, int messageSlots,
                                  ssize_t messageSize, void *buffer,
                                  ssize_t buffer_size)
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
static inline int tims_mbx_remove(int fd)
{
    return rt_dev_close(fd);
}


// delete all messages
static inline int tims_mbx_clean(int fd)
{
    return rt_dev_ioctl(fd, TIMS_RTIOC_MBXCLEAN, 0);
}

// waiting a specific time for a new message
// -> returns a pointer to a tims_msg_head if successful
// -> locks the message
static inline int tims_peek_timed(int fd, tims_msg_head **pp_head,
                                  int64_t timeout_ns)
{
    // set timeout
    int ret = tims_set_timeout(fd, timeout_ns);
    if (ret)
        return ret;

    return rt_dev_ioctl(fd, TIMS_RTIOC_RECVBEGIN, pp_head);
}

// unlock the message, whitch is locked before
static inline int tims_peek_end(int fd)
{
    return rt_dev_ioctl(fd, TIMS_RTIOC_RECVEND, NULL);
}

#ifdef __cplusplus
}
#endif

#else // !__XENO__ && !__KERNEL__

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <main/tims/tims_router.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline ssize_t tims_sendmsg(int fd, tims_msg_head *p_head,
                                   struct iovec *vec, unsigned char veclen,
                                   int timsflags)
{
    int ret, i;

    //printf("Tims: %8x --(%4d)--> %8x, send msg (%u bytes)\n", p_head->src, p_head->type, p_head->dest, p_head->msglen);

    ret = send(fd, p_head, TIMS_HEADLEN, 0);
    if(ret < (int)TIMS_HEADLEN)
    {
        printf("Tims: %8x --(%4d)--> %8x, (%u bytes), "
                   "send ERROR, (%s)\n", p_head->src, p_head->type, p_head->dest, p_head->msglen, strerror(errno));
        return ret;
    }

    for(i = 0; i < veclen; i++)
    {
        ret = send(fd, vec[i].iov_base, vec[i].iov_len, 0);
        if(ret < (int)vec[i].iov_len)
        {
            printf("Tims: %8x --(%4d)--> %8x, (%u bytes), "
                       "send ERROR, (%s)\n", p_head->src, p_head->type, p_head->dest, p_head->msglen, strerror(errno));
            return ret;
        }
    }

    return p_head->msglen;
}

static inline int tims_recvmsg_timed(int fd, tims_msg_head *p_head, void *p_data,
                                     ssize_t maxdatalen, int64_t timeout_ns,
                                     int timsflags)
{
    int             ret;
    unsigned int    len;
    tims_msg_head   replyMsg;

    fd_set set;
    struct timeval timeout;

    FD_ZERO (&set);
    FD_SET (fd, &set);

    if(timeout_ns == TIMS_NONBLOCK)
    {
        timeout.tv_sec = 0;
        timeout.tv_usec = 1;
    }
    else
    {
        timeout.tv_sec = (int)(timeout_ns / (int64_t)1000000000);
        timeout.tv_sec++;
        timeout.tv_usec = 0;
    }

    do
    {
        len = 0;
        // receive head
        while (len < TIMS_HEADLEN)
        {
            if(timeout_ns != TIMS_INFINITE)
            {
                ret = select (FD_SETSIZE, &set, NULL, NULL, &timeout);
                if(ret == 0)
                {
                    return -EWOULDBLOCK;
                }
                else if(ret < 0)
                {
                    return ret;
                }
            }

            ret = recv(fd, (char*)p_head + len, TIMS_HEADLEN - len, 0);
            if (ret < 0)
            {
                close(fd);
                printf("Tims: recv head ERROR, (%s)\n", strerror(errno));
                return ret;
            }

            if (ret == 0)
            {
                close(fd);
                printf("Tims: recv head ERROR, socket closed\n");
                return -1;
            }
            len += ret;
        }

        tims_parse_head_byteorder(p_head);

        if (p_head->msglen > maxdatalen + TIMS_HEADLEN)
        {
            close(fd);
            printf("Tims: %8x --(%4d)--> %8x, recv ERROR, message "
                       "(%u bytes) is too big for buffer (%u bytes)\n", p_head->src, p_head->type, p_head->dest,
                       p_head->msglen, maxdatalen + TIMS_HEADLEN);
            return -1;
        }

        if (p_head->msglen > TIMS_HEADLEN)
        {
            len = 0;

            while (len < p_head->msglen - TIMS_HEADLEN)
            {
                ret = recv(fd, (char*)p_data + len, p_head->msglen - TIMS_HEADLEN - len, 0);
                if (ret < 0)
                {
                    close(fd);
                    printf("Tims: %8x --(%4d)--> %8x, recv body "
                               "ERROR, (%s)\n", p_head->src, p_head->type, p_head->dest,
                               strerror(errno));
                    return -1;
                }

                if (ret == 0)
                {
                    close(fd);
                    printf("Tims: %8x --(%4d)--> %8x, recv body "
                               "ERROR, socket closed\n", p_head->src, p_head->type, p_head->dest);
                    return -1;
                }
                len += ret;
            }
        }
        //printf("Tims: %8x --(%4d)--> %8x, recv msg (%u bytes)\n", p_head->src, p_head->type, p_head->dest, p_head->msglen);

        if(p_head->type == TIMS_MSG_ROUTER_GET_STATUS)
        {
            // send reply to router watchdog
            tims_fill_head(&replyMsg, TIMS_MSG_OK, 0, 0, 0, 0, 0, sizeof(replyMsg));

            ret = tims_sendmsg(fd, &replyMsg, NULL, 0, 0);
            if (ret < (int)sizeof(replyMsg))
            {
                printf("Tims ERROR: Can't send watchdog reply message (code %i)\n", ret);
                close(fd);
                return ret;
            }
        }
    }
    while(p_head->type == TIMS_MSG_ROUTER_GET_STATUS);

    return p_head->msglen;
}

static inline int tims_mbx_create(uint32_t address, int messageSlots,
                                  ssize_t messageSize, void *buffer,
                                  ssize_t buffer_size)
{
    int fd, ret;

    tims_router_mbx_msg mbxInitMsg;
    struct iovec        iov[1];
    tims_msg_head       msg;

    char                ip[16];
    int                 port;
    struct sockaddr_in  tcpAddr;

    strncpy(ip, "127.0.0.1", 16);
    port = 2000;

    // init router IP
    if ((tcpAddr.sin_addr.s_addr = inet_addr(ip)) == INADDR_NONE)
    {
        printf("Tims ERROR: Ip %s not valid\n", ip);
        return -1;
    }

    tcpAddr.sin_port   = htons((unsigned short)port);
    tcpAddr.sin_family = AF_INET;
    bzero(&(tcpAddr.sin_zero), 8);

    // open socket
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        printf("Tims ERROR: Can't create socket, (%s)\n",
                   strerror(errno));
        return errno;
    }

    // connect
    ret = connect(fd, (struct sockaddr *)&tcpAddr, sizeof(tcpAddr));
    if (ret)
    {
        printf("Tims ERROR: Can't connect to Router, (%s)\n",
                   strerror(errno));
       close(fd);
        return ret;
    }

    // disable watchdog
    tims_fill_head(&msg, TIMS_MSG_ROUTER_DISABLE_WATCHDOG, 0, 0, 0, 0, 0, sizeof(msg));

    ret = tims_sendmsg(fd, &msg, NULL, 0, 0);
    if (ret < 0)
    {
        printf("Tims ERROR: Can't send diable watchdog message to router (code %i)\n", ret);
        close(fd);
        return ret;
    }

    // init mbx
    tims_fill_head((tims_msg_head*)&mbxInitMsg, TIMS_MSG_ROUTER_MBX_INIT_WITH_REPLY, 0, 0, 0, 0, 0, sizeof(mbxInitMsg));
    mbxInitMsg.mbx = address;

    iov[0].iov_base = mbxInitMsg.head.data;
    iov[0].iov_len  = sizeof(mbxInitMsg) - TIMS_HEADLEN;

    ret = tims_sendmsg(fd, (tims_msg_head*)&mbxInitMsg, iov, 1, 0);
    if (ret < 0)
    {
        printf("Tims ERROR: Can't send mbx init message to router (code %i)\n", ret);
        close(fd);
        return ret;
    }

    ret = tims_recvmsg_timed(fd, &msg, NULL, 0, 0, 0);
    if(ret < 0)
    {
        printf("Tims ERROR: Can't read mbx init reply (code %i)\n", ret);
        close(fd);
        return ret;
    }
    if(msg.type != TIMS_MSG_OK)
    {
        printf("Tims ERROR: Can't ini mbx(%x)\n", address);
        close(fd);
        return -1;
    }

    //printf("Tims: Connected to TimsRouterTcp (mbx %x)\n", address);

    return fd;
}

static inline int tims_mbx_remove(int fd)
{
    close(fd);

    printf("Tims: Socket closed\n");

    return 0;
}

static inline int tims_mbx_clean(int fd)
{
    return -1;//rt_dev_ioctl(fd, TIMS_RTIOC_MBXCLEAN, 0);
}

static inline int tims_peek_timed(int fd, tims_msg_head **pp_head,
                                  int64_t timeout_ns)
{
    printf("tims_peek_timed not implemented\n");
    return -1;
    /*// set timeout
    int ret = tims_set_timeout(fd, timeout_ns);
    if (ret)
        return ret;

    return rt_dev_ioctl(fd, TIMS_RTIOC_RECVBEGIN, pp_head);*/
}

static inline int tims_peek_end(int fd)
{
    printf("tims_peek_end not implemented\n");
    return -1;//rt_dev_ioctl(fd, TIMS_RTIOC_RECVEND, NULL);
}

#ifdef __cplusplus
}
#endif

#endif // __XENO__ || __KERNEL__

#endif // __TIMS_API_H__
