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
#ifndef __TIMS_ROUTER_H__
#define __TIMS_ROUTER_H__

/* this header file is used by tims_client, tims_router_tcp
   and tims kernel module
*/

#define PIPE_TIMS_TO_CLIENT                     6
#define PIPE_CLIENT_TO_TIMS                     7

//######################################################################
//# tims_router_mbx_msg
//######################################################################

typedef struct {
    tims_msg_head head;
    uint32_t      mbx;
} __attribute__((packed)) tims_router_mbx_msg;

//######################################################################
//# tims_router_config_msg
//######################################################################

typedef struct {
    uint32_t     mbx;
    uint32_t     ip;
} __attribute__((packed)) tims_router_mbx_route;

typedef struct {
    tims_msg_head           head;
    uint32_t                num;
    tims_router_mbx_route   mbx_route[0];
} __attribute__((packed)) tims_router_config_msg;

#define MAX_RTNET_ROUTE_NUM         512

//######################################################################
//# tims_router_config_msg parsing function
//######################################################################

static inline tims_router_mbx_msg* tims_router_parse_mbx_msg(tims_msg_head* p)
{
    tims_router_mbx_msg *returnP = (tims_router_mbx_msg*)p;

    if (returnP->head.flags & MESSAGE_FLAG_BODY_ORDER_LE)  // body is little endian
    {
        returnP->mbx = __le32_to_cpu(returnP->mbx);
    }
    else // body is big endian
    {
        returnP->mbx = __be32_to_cpu(returnP->mbx);
    }

      tims_set_body_byteorder(p);
      return returnP;
}

#endif // __TIMS_ROUTER_H_
