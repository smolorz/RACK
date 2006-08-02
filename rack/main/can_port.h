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
#ifndef __CAN_PORT_H__
#define __CAN_PORT_H__

#include <main/rack_time.h>
#include <main/rack_module.h>
#include <rtcan.h>

/* Create a sockaddr_can with specific filterlen

struct meta_sockaddr_can {
    struct sockaddr_can  scan;          // you need not init this
    rtcan_filter_t       filter[...];
};

sockaddr_can mcan;

*/
//######################################################################
//# class CanPort
//######################################################################

class CanPort
{
    private:

    protected:

        int fd;
        Module *module;

    public:

        CanPort();
        ~CanPort();

        int open(int dev, sockaddr_can* scan, int scan_size, Module *module);
        int close(void);

        int setTxTimeout(int64_t timeout);
        int setRxTimeout(int64_t timeout);
        int getTimestamps();

        int send(rtcan_frame_t* frame);
        int recv(rtcan_frame_t *recv_frame, rack_time_t *timestamp);

};

#endif // __CAN_PORT_H__
