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

#include <main/tims/tims_api.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

ssize_t tims_sendmsg(int fd, tims_msg_head *p_head, struct iovec *vec,
                     unsigned char veclen, int timsflags)
{
    int ret, i;

    //printf("Tims: %8x --(%4d)--> %8x, send msg (%u bytes)\n", (unsigned int)p_head->src, p_head->type, (unsigned int)p_head->dest, (unsigned int)p_head->msglen);

    ret = send(fd, p_head, TIMS_HEADLEN, 0);
    if(ret < (int)TIMS_HEADLEN)
    {
        printf("Tims: %8x --(%4d)--> %8x, (%u bytes), "
                   "send ERROR, (%s)\n", (unsigned int)p_head->src, p_head->type, (unsigned int)p_head->dest, (unsigned int)p_head->msglen, strerror(errno));
        return ret;
    }

    for(i = 0; i < veclen; i++)
    {
        ret = send(fd, vec[i].iov_base, vec[i].iov_len, 0);
        if(ret < (int)vec[i].iov_len)
        {
            printf("Tims: %8x --(%4d)--> %8x, (%u bytes), "
                       "send ERROR, (%s)\n", (unsigned int)p_head->src, p_head->type, (unsigned int)p_head->dest, (unsigned int)p_head->msglen, strerror(errno));
            return ret;
        }
    }

    return p_head->msglen;
}

int tims_recvmsg_timed(int fd, tims_msg_head *p_head, void *p_data,
                       ssize_t maxdatalen, int64_t timeout_ns, int timsflags)
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
                       "(%u bytes) is too big for buffer (%u bytes)\n", (unsigned int)p_head->src, p_head->type, (unsigned int)p_head->dest,
                       (unsigned int)p_head->msglen, (unsigned int)(maxdatalen + TIMS_HEADLEN));
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
                               "ERROR, (%s)\n", (unsigned int)p_head->src, p_head->type, (unsigned int)p_head->dest,
                               strerror(errno));
                    return -1;
                }

                if (ret == 0)
                {
                    close(fd);
                    printf("Tims: %8x --(%4d)--> %8x, recv body "
                               "ERROR, socket closed\n", (unsigned int)p_head->src, p_head->type, (unsigned int)p_head->dest);
                    return -1;
                }
                len += ret;
            }
        }
        //printf("Tims: %8x --(%4d)--> %8x, recv msg (%u bytes)\n", (unsigned int)p_head->src, p_head->type, (unsigned int)p_head->dest, (unsigned int)p_head->msglen);

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

int tims_mbx_create(uint32_t address, int messageSlots, ssize_t messageSize,
                    void *buffer, ssize_t buffer_size)
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

    // minimize transmission latency
    int flags = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flags, sizeof(flags));

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
        printf("Tims ERROR: Can't ini mbx(%x)\n", (unsigned int)address);
        close(fd);
        return -1;
    }

    //printf("Tims: Connected to TimsRouterTcp (mbx %x)\n", (unsigned int)address);

    return fd;
}

int tims_mbx_remove(int fd)
{
    close(fd);

    //printf("Tims: Socket closed\n");

    return 0;
}

int tims_mbx_clean(int fd, int addr)
{
    char buffer[128];
    int ret;
    
    do
    {
        // read an dump the data from this socket
        ret = recv(fd, buffer, 128, MSG_DONTWAIT);
    }
    while(ret > 0);
    
    return 0;
}

int tims_peek_timed(int fd, tims_msg_head **pp_head, int64_t timeout_ns)
{
    printf("tims_peek_timed not implemented\n");
    return -1;
    /*// set timeout
    int ret = tims_set_timeout(fd, timeout_ns);
    if (ret)
        return ret;

    return rt_dev_ioctl(fd, TIMS_RTIOC_RECVBEGIN, pp_head);*/
}

int tims_peek_end(int fd)
{
    printf("tims_peek_end not implemented\n");
    return -1;//rt_dev_ioctl(fd, TIMS_RTIOC_RECVEND, NULL);
}

#ifdef __cplusplus
}
#endif
