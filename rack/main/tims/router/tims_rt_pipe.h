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
#ifndef __TIMS_RT_PIPE_H__
#define __TIMS_RT_PIPE_H__

#include <main/tims/tims.h>
#include <main/tims/tims_router.h>

#define PIPE_TIMS_TO_CLIENT                     6
#define PIPE_CLIENT_TO_TIMS                     7

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

#endif // __TIMS_RT_PIPE_H_
