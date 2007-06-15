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
 *      Oliver Wulf <oliver.wulf@web.de>
 *
 */
#ifndef __TIMS_ROUTER_H__
#define __TIMS_ROUTER_H__

#include <main/tims/tims.h>

//
// sending message types ( > 0 )
//

// TimsClient -> TcpRouter
#define TIMS_MSG_ROUTER_LOGIN                  10
#define TIMS_MSG_ROUTER_MBX_INIT               11 // ohne Quittung
#define TIMS_MSG_ROUTER_MBX_DELETE             12 // ohne Quittung
#define TIMS_MSG_ROUTER_MBX_INIT_WITH_REPLY    13 // Sendet TIMS_MSG_OK oder TIMS_MSG_ERROR zurueck
#define TIMS_MSG_ROUTER_MBX_DELETE_WITH_REPLY  14 // Sendet TIMS_MSG_OK oder TIMS_MSG_ERROR zurueck
#define TIMS_MSG_ROUTER_MBX_PURGE              15

// TimsDriver -> TimsRtPipe
#define TIMS_MSG_ROUTER_GET_CONFIG             16

// TcpRouter -> TimsClient
#define TIMS_MSG_ROUTER_GET_STATUS             17 // Client -> TCP Router TIMS_MSG_OK

//
//  return / reply message types ( <=0 )
//

// TimsRtPipe -> TimsDriver
#define TIMS_MSG_ROUTER_CONFIG                -10
#define TIMS_MSG_ROUTER_CONNECTED             -11

//######################################################################
//# tims_router_mbx_msg
//######################################################################

typedef struct
{
    tims_msg_head   head;
    uint32_t        mbx;
} __attribute__((packed)) tims_router_mbx_msg;

//######################################################################
//# tims_router_mbx_msg parsing function
//######################################################################

static inline tims_router_mbx_msg* tims_router_parse_mbx_msg(tims_msg_head* p)
{
    tims_router_mbx_msg *returnP = (tims_router_mbx_msg*)p;

    if (returnP->head.flags & TIMS_BODY_BYTEORDER_LE)  // body is little endian
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
