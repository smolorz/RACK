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
 *      Sebastian Smolorz <smolorz@rts.uni-hannover.de>
 */
#include <main/tims/driver/tims_rtnet.h>
#include <main/tims/driver/tims_debug.h>

#define RTNET_INIT_BIT_RTNET_SOCKET          0
#define RTNET_INIT_BIT_RTNET_MBXROUTE        1

//
// external functions / values
//

// --- tims_driver ---
extern tims_ctx*        tims_ctx_get(uint32_t mbxaddr);
extern int              tims_ctx_put(tims_ctx *ctx);
extern tims_mbx_slot*   tims_get_write_slot(tims_mbx *p_mbx, __s8 prio_new);
extern tims_mbx_slot*   tims_get_cont_write_slot(tims_mbx *p_mbx,
                                                 uint32_t src_addr);
extern void             tims_put_write_slot(tims_mbx *p_mbx, tims_mbx_slot *slot);
extern void             tims_put_write_slot_error(tims_mbx *p_mbx, tims_mbx_slot *slot);

// --- tims_debug ---
extern int dbglevel;

//
// Module parameter
//

static int rtnet_buffers = 200;

module_param(rtnet_buffers, int, 0400);
MODULE_PARM_DESC(rtnet_buffers, "number of RTnet buffers for incoming "
                                "and outgoing packets");

//
// data structures
//
tims_rtnet_extension  rtnet;

/* rtnet_limit_msg - The purpose of this function is to manipulate the iovecs
 * of the message to be send over RTnet so that a maximum length is kept.
 * Returns the size of the last iovec buffer to be sent as final part. */
static inline unsigned int rtnet_limit_msg(struct msghdr *msg, int len)
{
    struct iovec *iov = msg->msg_iov;
    int iovlen = 1;

    while (len) {
        if (len > iov->iov_len) {
            len -= iov->iov_len;
            iov++;
            iovlen++;
        } else {
            iov->iov_len = len;
            len = 0;
            msg->msg_iovlen = iovlen;
        }
    }
    return iov->iov_len;
}

/* rtnet_sendmsg - Called by the TiMS driver to send a message over RTnet */
int rtnet_sendmsg(rtdm_user_info_t *user_info, const struct msghdr *msg)
{
    int                 i;
    int                 ret;
    tims_msg_head       *p_head = msg->msg_iov[0].iov_base;

    uint32_t            msglen = p_head->msglen;
    tims_msg_head       head_tmp;
    struct iovec        iov[msg->msg_iovlen + 1];
    unsigned int        remain;

    struct msghdr       rtnet_msg;
    struct sockaddr_in  dest_addr;


    for (i = 0; i < rtnet.mbxRouteNum; i++) {
        if (rtnet.mbxRoute[i].mbx == p_head->dest) {
            dest_addr.sin_family          = AF_INET;
            dest_addr.sin_port            = htons(TIMS_MSG_ROUTER_PORT);
            dest_addr.sin_addr.s_addr     = rtnet.mbxRoute[i].ip;

            rtnet_msg.msg_name            = &dest_addr;
            rtnet_msg.msg_namelen         = sizeof(dest_addr);
            rtnet_msg.msg_control         = NULL;
            rtnet_msg.msg_controllen      = 0;
            rtnet_msg.msg_flags           = 0;

            /* Check whether the length of the message exceeds the size of an
             * UDP packet. */
            if (msglen > TIMS_RTNET_MAX_MSG_SIZE) {
                /* We have to split the message up. */
                memcpy(&head_tmp, p_head, TIMS_HEADLEN);
                memcpy(&iov[1], msg->msg_iov, sizeof(struct iovec) * 
                                                    msg->msg_iovlen);
                /* The head of the first message part is not modified aside
                 * from one bit in the flags field. */
                p_head->flags |= TIMS_RTNET_SPLIT_START;

                rtnet_msg.msg_iov = &iov[1];
                rtnet_msg.msg_iovlen = msg->msg_iovlen;
                /* The iovlens have to be cut so that they reflect exactly a
                 * length of a UDP packet minus the IP and UDP header len. */
                remain = rtnet_limit_msg(&rtnet_msg, TIMS_RTNET_MAX_MSG_SIZE);

                /* Send the initial part of the message. */
                ret = rtdm_sendmsg(rtnet.fd, &rtnet_msg, 0);
                if (ret < TIMS_RTNET_MAX_MSG_SIZE) {
                    tims_error("[RTnet]: %x -> %x: Can't forward first part "
                               "of splitted message, type %i, msglen %i, "
                               "msg part len %i. rt_socket_sendto(), code = "
                               "%d\n", p_head->src, p_head->dest,
                              p_head->type, p_head->msglen,
                              TIMS_RTNET_MAX_MSG_SIZE, ret);
                    return ret;
                }

                tims_dbginfo("%x -> %x, send first part of splitted message "
                              "(%d bytes in whole) over RTnet\n", p_head->src,
                              p_head->dest, p_head->msglen);

                /* Prepare head_tmp which is prepended to every consecutive
                 * part of the splitted message. */
                head_tmp.flags |= TIMS_RTNET_SPLIT;
                head_tmp.msglen = TIMS_RTNET_MAX_MSG_SIZE;
                head_tmp.seq_nr = 1;
                rtnet_msg.msg_iov = iov;
                /* This is important as we prepend an additional iovec at the
                 * front of the array. */
                rtnet_msg.msg_iovlen++;

                msglen -= TIMS_RTNET_MAX_MSG_SIZE;

                do {
                    iov[0].iov_base = &head_tmp;
                    iov[0].iov_len = TIMS_HEADLEN;

                    if (msglen <= TIMS_RTNET_MAX_MSG_SIZE - TIMS_HEADLEN) {
                        /* Final part of the splitted message. */
                        head_tmp.msglen= msglen + TIMS_HEADLEN;
                        msglen = 0;
                        head_tmp.flags |= TIMS_RTNET_SPLIT_STOP;
                    } else
                        msglen = msglen - TIMS_RTNET_MAX_MSG_SIZE +
                                                            TIMS_HEADLEN;

                    /* Again, limit the message part to be sent. But before,
                     * fix the iov_len belonging to the last buffer sent. */
                    iov[rtnet_msg.msg_iovlen - 1].iov_len =
                            msg->msg_iov[rtnet_msg.msg_iovlen - 2].iov_len -
                                                                    remain;
                    remain = rtnet_limit_msg(&rtnet_msg, head_tmp.msglen);

                    /* Send a consecutive part of the message. */
                    ret = rtdm_sendmsg(rtnet.fd, &rtnet_msg, 0);
                    if (ret < head_tmp.msglen) {
                        tims_error("[RTnet]: %x -> %x: Can't forward part %i "
                            "of splitted message, type %i, msglen %i, msg "
                            "part len %i. rt_socket_sendto(), code = %d\n",
                              head_tmp.src, head_tmp.dest, head_tmp.seq_nr,
                              head_tmp.type, p_head->msglen, head_tmp.msglen,
                              ret);
                        return ret;
                    }
                    tims_dbginfo("%x -> %x, send consecutive part of "
                                 "splitted message (%d bytes) "
                                 "over RTnet\n", p_head->src, p_head->dest,
                                  head_tmp.msglen);

                    head_tmp.seq_nr++;

                } while (msglen);

                /* Adjust return value to the length of the whole message. */
                ret = p_head->msglen;

            } else {
                /* The message fits into one RTnet UDP packet. */
                rtnet_msg.msg_iov = msg->msg_iov;
                rtnet_msg.msg_iovlen = msg->msg_iovlen;

                ret = rtdm_sendmsg(rtnet.fd, &rtnet_msg, 0);
                if (ret < p_head->msglen) {
                    tims_error("[RTnet]: %x -> %x: Can't forward message, "
                               "type %i, msglen %i. "
                               "rt_socket_sendto(), code = %d\n",
                              p_head->src, p_head->dest, p_head->type,
                              p_head->msglen, ret);

                    return ret;
                }
            }

            tims_dbginfo("%x -> %x, send global message (%d bytes) over RTnet"
                         "\n", p_head->src, p_head->dest, p_head->msglen);
            return ret;
        }
    }
    return -ENODEV;    // destination address not found in table
}

static int rtnet_recv_message(tims_mbx* p_recv_mbx, tims_msg_head *head)
{
    tims_mbx_slot*  recv_slot       = NULL;
#if 0
    tims_msg_head*  head_part       = NULL;
#endif
    /* We need an additional iovec for heads of consecutive packets of
      splitted messages, therefore + 1. */
    struct iovec    iov[get_max_pages(head->msglen > TIMS_RTNET_MAX_MSG_SIZE ?
                              TIMS_RTNET_MAX_MSG_SIZE + 1: head->msglen + 1)];
    tims_msg_head   consec_head;
    struct msghdr   recv_msg;
    unsigned long   p_recv_map      = 0;
//    void*           p_recv          = NULL;
    unsigned long   recv_page       = 0;
    unsigned long   recv_bytes      = head->msglen;
    unsigned long   recv_bytes_usr;
    int             ret;
    int             free_in_page    = 0;
    int             akt_copy_size   = 0;


    /* Check if the packet is a consecutive part of a splitted message. */
    if (head->flags & TIMS_RTNET_SPLIT) {
        /* A splitted msg continues. We have to put it into an already
         * existing write slot. */
        recv_slot = tims_get_cont_write_slot(p_recv_mbx, head->src);
        if (!recv_slot) {
            tims_error("[RTnet]: Received a consecutive part of a splitted "
                       "msg without having its beginning! %08x -> %08x \n",
                        head->src, p_recv_mbx->address);
            rtdm_recv(rtnet.fd, head, 0, 0); // clean up
            return -EBADSLT;
        }
        /* Check the sequence number. */
        if (head->seq_nr != ++recv_slot->seq_nr) {
            tims_error("[RTnet]: Received a consecutive part of a splitted "
                       "msg with wrong sequence number %d, should be %d "
                       "(%08x -> %08x). Removing incomplete message.\n",
                        head->seq_nr, recv_slot->seq_nr, head->src,
                        p_recv_mbx->address);
            ret = -ENOMSG;
            goto recvmsg_error_clean;
        }

    } else {
        /* A new message. We have to ensure that there is no stale incomplete
         * message in the write list of this mbx from that src address. */
        recv_slot = tims_get_cont_write_slot(p_recv_mbx, head->src);
        if (recv_slot) {
            tims_warn("[RTnet]: Received a new msg from %08x to %08x while "
                      "waiting for the end of a splitted msg; now removing "
                      "it.\n", head->src, p_recv_mbx->address);
            /* Remove write slot with incomplete message. */
            tims_put_write_slot_error(p_recv_mbx, recv_slot);
        }

        // get local mailbox slot
        recv_slot = tims_get_write_slot(p_recv_mbx, head->priority);
        if (!recv_slot) {
            tims_error("[RTnet]: No free write slot in mailbox %08x \n",
                        p_recv_mbx->address);
            rtdm_recv(rtnet.fd, head, 0, 0); // clean up
            return -ENOSPC;
        }

        /* If a splitted msg starts adjust recv_bytes. If not bail out if
         * the msglen is greater than TIMS_RTNET_MAX_MSG_SIZE. */
        if (head->flags & TIMS_RTNET_SPLIT_START) {
            recv_bytes = TIMS_RTNET_MAX_MSG_SIZE;
            recv_slot->seq_nr = 0;
        } else if (unlikely(recv_bytes > TIMS_RTNET_MAX_MSG_SIZE)) {
            tims_error("[RTnet]: Msg for mbx %08x has more than %d bytes, "
                        "too large for RTnet!\n", p_recv_mbx->address,
                        TIMS_RTNET_MAX_MSG_SIZE);
            ret = -EFBIG;
            goto recvmsg_error_clean;
        }
    }

    //
    // create msg vector
    //
    memset(&recv_msg, 0, sizeof(struct msghdr));

    if (test_bit(TIMS_MBX_BIT_USRSPCBUFFER, &p_recv_mbx->flags)) {
        // receive mailbox is in userpace
        if (head->flags & TIMS_RTNET_SPLIT) {
            iov[0].iov_base = &consec_head;
            iov[0].iov_len = TIMS_HEADLEN;
            p_recv_map  = recv_slot->next_part_map;
            recv_page   = recv_slot->next_map_idx;
            recv_bytes_usr = recv_bytes - TIMS_HEADLEN;
            recv_msg.msg_iovlen = 1;
        } else {
            p_recv_map = recv_slot->p_head_map;
//          p_recv = recv_slot->p_head;
            recv_page = recv_slot->map_idx;
            recv_bytes_usr = recv_bytes;
            recv_msg.msg_iovlen = 0;
            recv_slot->next_part_map = p_recv_map;
        }
        recv_msg.msg_iov = iov;

        while (recv_bytes_usr) {
            free_in_page  = (p_recv_map & PAGE_MASK) + PAGE_SIZE - p_recv_map;
            akt_copy_size = free_in_page > recv_bytes_usr ?
                            recv_bytes_usr : free_in_page;

            iov[recv_msg.msg_iovlen].iov_base = (void *)p_recv_map;
            iov[recv_msg.msg_iovlen].iov_len  = akt_copy_size;

            recv_msg.msg_iovlen++;

            free_in_page -= akt_copy_size;
            recv_bytes_usr -= akt_copy_size;
            recv_slot->next_part_map += akt_copy_size;

            if (!free_in_page)
            {
                recv_page++;
                if (recv_slot->p_mbx->p_mapInfo[recv_page].mapped)
                {
                    p_recv_map = recv_slot->p_mbx->p_mapInfo[recv_page].virtual;
                }
                else
                {
                    ret = -ENOMEM;
                    goto recvmsg_error_clean;
                }
                recv_slot->next_part_map = p_recv_map;
            }
        }
        recv_slot->next_map_idx = recv_page;
    }
    else // receive mailbox is in kernelspace
    {
        if (head->flags & TIMS_RTNET_SPLIT) {
            iov[0].iov_base = &consec_head;
            iov[0].iov_len = TIMS_HEADLEN;
            iov[1].iov_base = recv_slot->next_part;
            iov[1].iov_len = recv_bytes - TIMS_HEADLEN;
            recv_msg.msg_iovlen = 2;
            recv_slot->next_part += iov[1].iov_len;
        } else {
            iov[0].iov_base = recv_slot->p_head;
            iov[0].iov_len = recv_bytes;
            recv_msg.msg_iovlen = 1;
            recv_slot->next_part = (void *)(recv_slot->p_head) + recv_bytes;
        }
        recv_msg.msg_iov = iov;
    }

    // receive complete message
    ret = rtdm_recvmsg(rtnet.fd, &recv_msg, 0);
    if (ret != recv_bytes)
    {
        tims_error("[RTnet]: Corrupt message received, code = %d\n", ret);
        goto recvmsg_error;
    }

    tims_dbginfo("[RTnet]: %x -> %x, received message (%d bytes)\n",
                  head->src, head->dest, head->msglen);

    /* The following part needs a major rework and is incomplete anyway. As a
     * temporary workaround we disable it as there should only be a theoretical
     * need for it.
     */
#if 0
    // parse message head
    if (unlikely(iov[0].iov_len < sizeof(tims_msg_head))) // head on two pages
    {
        // data sizes in head : 1, 1, 1, 1,  4, 4, 4

        free_in_page = ((unsigned long)iov[0].iov_base & PAGE_MASK) +
                        PAGE_SIZE - (unsigned long)iov[0].iov_base;

        head_part = (tims_msg_head*)iov[0].iov_base;
// TODO -> parsing
    }
    else
    {
        tims_parse_head_byteorder((tims_msg_head*)iov[0].iov_base);
    }
#endif

    switch (head->flags & TIMS_RTNET_SPLIT_MASK) {
    case TIMS_RTNET_SPLIT_START:
        /* Remove this bit in the head residing in the slot. */
        if (test_bit(TIMS_MBX_BIT_USRSPCBUFFER, &p_recv_mbx->flags))
            ((tims_msg_head *)(recv_slot->p_head_map))->flags &=
                                                    ~TIMS_RTNET_SPLIT_START;
        else
            recv_slot->p_head->flags &= ~TIMS_RTNET_SPLIT_START;
        break;
    case TIMS_RTNET_SPLIT:
        break;
    default:    /* also in case of TIMS_RTNET_SPLIT_STOP */
        tims_put_write_slot(p_recv_mbx, recv_slot);
        break;
    }

    return 0;

recvmsg_error_clean:
    rtdm_recv(rtnet.fd, head, 0, 0); // clean up

recvmsg_error:
    tims_put_write_slot_error(p_recv_mbx, recv_slot);
    tims_error("[RTnet]: %x -> %x, Can't send message, code = %d\n",
               head->src, head->dest, ret);
    return ret;
}

static void rtnet_recv_callback(struct rtdm_dev_context *context, void* arg)
{
    tims_msg_head   head;
    tims_ctx*       ctx;
    int             ret;
    tims_mbx*       p_recv_mbx  = NULL;

    // receive message head (only peek)
    ret = rtdm_recv(rtnet.fd, &head, sizeof(tims_msg_head), MSG_PEEK);
    if (ret < (int)sizeof(tims_msg_head))
    {
        tims_error("[RTnet]: Corrupt package received (rtdm_recv(): %d)\n",
                   ret);

        rtdm_recv(rtnet.fd, &head, 0, 0); /* clean up */
        return;
    }

    tims_parse_head_byteorder(&head);

    tims_dbgdetail("[RTnet] %x -> %x, type %d, msglen %d \n",
                   head.src, head.dest, head.type, head.msglen);

    // get mailbox context
    ctx = tims_ctx_get(head.dest);
    if (!ctx)
    {
        tims_error("[RTnet]: %x -> %x, Can't deliver msg, type %d, msglen %d. "
                   "Mailbox not available.\n", head.src, head.dest, head.type,
                   head.msglen);

        rtdm_recv(rtnet.fd, &head, 0, 0); // clean up
        return;
    }

    p_recv_mbx = ctx->p_mbx;

    // return, if buffer is too small
    if (head.msglen > p_recv_mbx->msg_size)
    {
        tims_error("[RTnet]: %x -> %x, Msg is too big for mbx (%d -> %d)\n",
                   head.src, head.dest, head.msglen, p_recv_mbx->msg_size);

        rtdm_recv(rtnet.fd, &head, 0, 0); // clean up
        tims_ctx_put(ctx);
        return;
    }

    // receive complete message
    ret = rtnet_recv_message(p_recv_mbx, &head);
    if (ret)
    {
        tims_error("[RTnet]: %x -> %x, Can't receive message, code = %d) \n",
                   head.src, head.dest, ret);
    }

    tims_ctx_put(ctx);
    return;
}

int rtnet_read_config(tims_router_config_msg *configMsg)
{
    struct ifreq                  ifr[8];
    struct ifconf                 ifc;
    int                           ips;
    int                           j;
    int                           localIp;
    tims_router_mbx_route*        entry;
    unsigned int                  i;
    tims_msg_head*                p_head;
    unsigned int                  configSize;

    if (rtnet.mbxRouteNum) // routing table was filled
        return -EBUSY;

    p_head = &configMsg->head;

    if (p_head->flags & TIMS_BODY_BYTEORDER_LE)
        configMsg->num = __le32_to_cpu(configMsg->num);
    else
        configMsg->num = __be32_to_cpu(configMsg->num);

    if (!configMsg->num) // no route is inside the config message
    {
        tims_error("[RTnet]: No route in config file\n");
        return -ENODEV;
    }

    configSize = configMsg->num * sizeof(tims_router_mbx_route);

    if ((p_head->msglen - sizeof(tims_router_config_msg)) < configSize )
    {
        tims_error("[RTnet]: Corrupt configuration msg in pipeFromClient, "
                   "configSize too great\n");
        return -EINVAL;
    }

    ifc.ifc_len = sizeof(ifr);
    ifc.ifc_req = ifr;
    if (rtdm_ioctl(rtnet.fd, SIOCGIFCONF, &ifc) < 0)
        ips = 0;
    else
        ips = ifc.ifc_len / sizeof(struct ifreq);

    for (i=0; i<configMsg->num; i++)
    {
        entry = &configMsg->mbx_route[i];

        if (configMsg->head.flags & TIMS_BODY_BYTEORDER_LE)
        {
            entry->mbx = __le32_to_cpu(entry->mbx);
            entry->ip  = __le32_to_cpu(entry->ip);
        }
        else
        {
            entry->mbx = __be32_to_cpu(entry->mbx);
            entry->ip  = __be32_to_cpu(entry->ip);
        }

        localIp = 0;
        for (j = 0; j < ips; j++)
        {
            if (entry->ip ==
                ((struct sockaddr_in*)&ifr[j].ifr_addr)->sin_addr.s_addr)
            {
                localIp = 1;
            }
        }

        if (!localIp)
        {
            memcpy(&rtnet.mbxRoute[rtnet.mbxRouteNum],
                   entry, sizeof(tims_router_mbx_route));

            tims_info("[RTnet]: add route [%02d]: mbx: %08x - ip: %08x \n",
                      rtnet.mbxRouteNum, entry->mbx, entry->ip);

            rtnet.mbxRouteNum++;
        }
    }
    return 0;
}

void rtnet_cleanup(void)
{
    void *tmp;

    if (test_and_clear_bit(RTNET_INIT_BIT_RTNET_SOCKET, &rtnet.init_flags))
    {
        while (rtdm_close(rtnet.fd) == -EAGAIN)
        {
            tims_info("[RTnet]: socket busy - waiting...\n");
            set_current_state(TASK_INTERRUPTIBLE);
            schedule_timeout(1*HZ); /* wait a second */
        }
    }

    if (test_and_clear_bit(RTNET_INIT_BIT_RTNET_MBXROUTE, &rtnet.init_flags))
    {
        tmp = rtnet.mbxRoute;
        rtnet.mbxRoute = NULL;
        if (tmp != NULL)
            kfree(tmp);

        tims_info("[RTnet]: Buffer for struct maxRoute deleted\n");
    }
}

int rtnet_init(void)
{
    int                     ret;
    struct sockaddr_in      bindAddr;
    int                     nonblock = -1;
    tims_rtnet_callback     callback = {rtnet_recv_callback, NULL};

    rtnet.init_flags  = 0;
    rtnet.fd          = -1;
    rtnet.mbxRoute    = NULL;
    rtnet.mbxRouteNum = 0;

    // create socket
    rtnet.fd = rtdm_socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (rtnet.fd < 0)
    {
        tims_error("[RTnet]: Unable to create socket.\n");
        ret = rtnet.fd;
        goto rtnet_error;
    }
    tims_info("[RTnet]: Socket created \n");
    set_bit(RTNET_INIT_BIT_RTNET_SOCKET, &rtnet.init_flags);

    // bind to TIMS_MSG_ROUTER_PORT
    bindAddr.sin_family      = AF_INET;
    bindAddr.sin_port        = htons(TIMS_MSG_ROUTER_PORT);
    bindAddr.sin_addr.s_addr = INADDR_ANY;
    ret = rtdm_bind(rtnet.fd, (struct sockaddr *)&bindAddr, sizeof(bindAddr));
    if (ret < 0)
    {
        tims_error("[RTnet]: Unable to bind socket.\n");
        goto rtnet_error;
    }
    tims_info("[RTnet]: Socket has been bound \n");

    // allocate additional buffers
    ret = rtdm_ioctl(rtnet.fd, RTNET_RTIOC_EXTPOOL, &rtnet_buffers);
    if (ret != rtnet_buffers)
    {
        tims_error("[RTnet]: Unable to allocate additional buffers.\n");
        goto rtnet_error;
    }
    rtdm_ioctl(rtnet.fd, RTNET_RTIOC_TIMEOUT, &nonblock);
    rtdm_ioctl(rtnet.fd, RTNET_RTIOC_CALLBACK, &callback);


    // allocate mem for routing table
    rtnet.mbxRoute = kmalloc(MAX_RTNET_ROUTE_NUM, GFP_KERNEL *
                             sizeof(tims_router_mbx_route));
    if (!rtnet.mbxRoute)
    {
        tims_error("[RTnet]: Insufficient memory for routing table\n");
        return -ENOMEM;
    }
    set_bit(RTNET_INIT_BIT_RTNET_MBXROUTE, &rtnet.init_flags);
    tims_info("[RTnet]: Buffer for routing table created\n");

    return 0;

rtnet_error:
    rtnet_cleanup();
    return ret;
}

