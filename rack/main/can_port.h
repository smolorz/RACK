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
 *      Jan Kiszka <jan.kiszka@web.de> - update to new RT-Socket-CAN profile
 *
 */
#ifndef __CAN_PORT_H__
#define __CAN_PORT_H__

#include <main/rack_time.h>
#include <main/rack_module.h>
#include <rtdm/rtcan.h>

//######################################################################
//# class CanPort
//######################################################################

class CanPort
{
    private:

    protected:

        int fd;
        RackModule *module;

    public:

        CanPort();
        ~CanPort();

        int open(int dev, can_filter_t *filter_list, int nr_filters, RackModule *module);
        int close(void);

        int setTxTimeout(int64_t timeout);
        int setRxTimeout(int64_t timeout);
        int getTimestamps();

        int send(can_frame_t *frame);
        int recv(can_frame_t *recv_frame, rack_time_t *timestamp);

};

#endif // __CAN_PORT_H__
