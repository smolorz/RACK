/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Oliver Wulf <oliver.wulf@web.de> - linux implementation based on Socket-CAN API
 *
 */

#include <main/can_port.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

//
// c function wrappers
//

int close_can_dev(int filedes)
{
    return close(filedes);
}

//
// Constructor and destructor
//

CanPort::CanPort()
{
    fd = -1;
}

CanPort::~CanPort()
{
    if (fd != -1)
        close();
}

//
// PortFunctions
//

int CanPort::open(int dev, can_filter_t *filter_list, int nr_filters, RackModule *module)
{
    int                 ret;
    struct ifreq        ifr;
    struct sockaddr_can scan;

    if (fd != -1) // file is open
        return -EBUSY;

    // Prepare CAN socket and controller
    fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (fd < 0)
        return fd;

    // get interface index
    sprintf(ifr.ifr_name, "can%d", dev);
    ret = ioctl(fd, SIOCGIFINDEX, &ifr);
    if (ret)
        goto exit_error;

    if (nr_filters > 0)
    {
        ret = setsockopt(fd, SOL_CAN_RAW, CAN_RAW_FILTER, filter_list,
                         nr_filters * sizeof(can_filter_t));
        if (ret)
            goto exit_error;
    }

    // Bind socket to default CAN IDs
    scan.can_family  = AF_CAN;
    scan.can_ifindex = ifr.ifr_ifindex;

    ret = bind(fd, (struct sockaddr *)&scan, sizeof(scan));
    if (ret)
        goto exit_error;

    this->module = module;
    return 0;

exit_error:
    if (fd != -1)
        close();

    return ret;
}

int CanPort::close(void)
{
    int i = 5;
    int ret;

    do
    {
        ret = close_can_dev(fd);
        if (!ret) // exit on success
        {
            fd = -1;
            return 0;
        }
        else if (ret == -EAGAIN) // try it again (max 5 times)
            sleep(1); // wait 1s
    }
    while (ret != -EAGAIN && i--);

    return ret;
}

int CanPort::setTxTimeout(int64_t timeout)
{
    struct timeval tv;

    timeout += 1000;        // add 1 usec to make sure that the result is > 0 after rounding

    // convert from nsec to sec and usec
    tv.tv_sec = timeout / 1000000000ll;
    tv.tv_usec = (timeout % 1000000000ll) / 1000;

    return setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

int CanPort::setRxTimeout(int64_t timeout)
{
    struct timeval tv;

    timeout += 1000;        // add 1 usec to make sure that the result is > 0 after rounding

    // convert from nsec to sec and usec
    tv.tv_sec = timeout / 1000000000ll;
    tv.tv_usec = (timeout % 1000000000ll) / 1000;

    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

int CanPort::getTimestamps()
{
    return 0;
}

int CanPort::send(can_frame_t* frame)
{
    int ret;

    ret = write(fd, frame, sizeof(can_frame_t));
    if (ret < 0)
        return ret;

    return 0;
}

int CanPort::recv(can_frame_t *recv_frame, rack_time_t *timestamp)
{
    int ret;

    ret = read(fd, recv_frame, sizeof(can_frame_t));
    if (ret < 0)
        return ret;

    if (timestamp)
    {
        *timestamp = module->rackTime.get();
    }

    return 0;
}
