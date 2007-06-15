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

//
// rtnet functions
//
int rtnet_sendmsg(rtdm_user_info_t *user_info, const struct msghdr *msg)
{
    int                 i;
    int                 ret;
    tims_msg_head*      p_head = msg->msg_iov[0].iov_base;
    struct msghdr       rtnet_msg;
    struct sockaddr_in  dest_addr;

    for (i = 0; i < rtnet.mbxRouteNum; i++)
    {
        if (rtnet.mbxRoute[i].mbx == p_head->dest)
        {
            dest_addr.sin_family          = AF_INET;
            dest_addr.sin_port            = htons(TIMS_MSG_ROUTER_PORT);
            dest_addr.sin_addr.s_addr     = rtnet.mbxRoute[i].ip;

            rtnet_msg.msg_name            = &dest_addr;
            rtnet_msg.msg_namelen         = sizeof(dest_addr);
            rtnet_msg.msg_iov             = msg->msg_iov;
            rtnet_msg.msg_iovlen          = msg->msg_iovlen;
            rtnet_msg.msg_control         = NULL;
            rtnet_msg.msg_controllen      = 0;
            rtnet_msg.msg_flags           = 0;

            ret = rtdm_sendmsg(rtnet.fd, &rtnet_msg, 0);
            if (ret < rtnet_msg.msg_namelen + p_head->msglen)
            {
                tims_error("[RTnet]: %x -> %x: Can't forward message, "
                           "type %i, msglen %i. "
                           "rt_socket_sendto(), code = %d\n",
                           p_head->src, p_head->dest, p_head->type,
                           p_head->msglen, ret);

                return ret;
            }

            tims_dbginfo("%x -> %x, send global message (%d bytes) over RTnet\n",
                         p_head->src, p_head->dest, p_head->msglen);
            return 0;
        }
    }
    return -ENODEV;    // destination address not found in table
}

static int rtnet_recv_message(tims_mbx* p_recv_mbx, tims_msg_head *head)
{
    tims_mbx_slot*  recv_slot       = NULL;
    tims_msg_head*  head_part       = NULL;
    struct iovec    iov[get_max_pages(head->msglen)];
    struct msghdr   recv_msg;
    unsigned long   p_recv_map      = 0;
    void*           p_recv          = NULL;
    unsigned long   recv_page       = 0;
    unsigned long   recv_bytes      = head->msglen;
    int             ret;
    int             free_in_page    = 0;
    int             akt_copy_size   = 0;

    // get local mailbox slot
    recv_slot = tims_get_write_slot(p_recv_mbx, head->priority);
    if (!recv_slot)
    {
        tims_error("[RTnet]: No free write slot in mailbox %08x \n",
                   p_recv_mbx->address);
        rtdm_recv(rtnet.fd, head, 0, 0); // clean up
        return -ENOSPC;
    }

    //
    // create msg vector
    //
    memset(&recv_msg, 0, sizeof(struct msghdr));

    if (test_bit(TIMS_MBX_BIT_USRSPCBUFFER, &p_recv_mbx->flags))
    {
        // receive mailbox is in userpace

        p_recv_map  = recv_slot->p_head_map;
        p_recv      = recv_slot->p_head;
        recv_page   = recv_slot->map_idx;

        recv_msg.msg_iov    = iov;
        recv_msg.msg_iovlen = 0;

        while (recv_bytes)
        {
            free_in_page  = (p_recv_map & PAGE_MASK) + PAGE_SIZE - p_recv_map;
            akt_copy_size = free_in_page > recv_bytes ?
                            recv_bytes : free_in_page;

            iov[recv_msg.msg_iovlen].iov_base = (void*)p_recv_map;
            iov[recv_msg.msg_iovlen].iov_len  = akt_copy_size;

            recv_msg.msg_iovlen++;

            free_in_page -= akt_copy_size;

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
            }
        }
    }
    else // receive mailbox is in kernelspace
    {
        struct iovec iov[1];

        iov[0].iov_base     = recv_slot->p_head;
        iov[0].iov_len      = recv_bytes;

        recv_msg.msg_iov    = iov;
        recv_msg.msg_iovlen = 1;
    }

    // receive complete message
    ret = rtdm_recvmsg(rtnet.fd, &recv_msg, 0);
    if (ret != head->msglen)
    {
        tims_error("[RTnet]: Corrupt message received, code = %d) \n", ret);
        goto recvmsg_error;
    }

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



    tims_put_write_slot(p_recv_mbx, recv_slot);
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
                   &entry, sizeof(tims_router_mbx_route));

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

