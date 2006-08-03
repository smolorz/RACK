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
 *      Sebastian Smolorz <Sebastian.Smolorz@stud.uni-hannover.de>
 *
 */
#ifndef __TIMS_RTNET_H__
#define __TIMS_RTNET_H__

#include <main/tims/driver/tims_driver.h>

#include <net/ip.h>

typedef struct {
    void    (*func)(struct rtdm_dev_context *, void *);
    void    *arg;
} tims_rtnet_callback;

#define RTIOC_TYPE_NETWORK      RTDM_CLASS_NETWORK

#define RTNET_RTIOC_TIMEOUT     _IOW(RTIOC_TYPE_NETWORK, 0x11, int64_t)
#define RTNET_RTIOC_CALLBACK    _IOW(RTIOC_TYPE_NETWORK, 0x12, tims_rtnet_callback)
#define RTNET_RTIOC_EXTPOOL     _IOW(RTIOC_TYPE_NETWORK, 0x14, unsigned int)

#define TIMS_MSG_ROUTER_PORT    0x2000

typedef struct
{
    int                     fd;
    tims_router_mbx_route*  mbxRoute;
    unsigned int            mbxRouteNum;
    char                    enabled;
    unsigned long           init_flags;
} tims_rtnet_extension;

#endif // __TIMS_RTNET_H__
