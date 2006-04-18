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
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#include <main/can_port.h>

#include <iostream>
#include <errno.h>

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

int CanPort::open(int dev, sockaddr_can* scan, int scan_size)
{
    int             ret;
    struct ifreq    ifr;

    if (fd != -1) // file is open
    {
        return -EBUSY;
    }

    // Prepare CAN socket and controller
    fd = rt_dev_socket(PF_CAN, SOCK_RAW, 0);
    if (fd < 0)
    {
        return fd;
    }

    // get interface index
    sprintf(ifr.ifr_name, "rtcan%d", dev);
    ret = rt_dev_ioctl(fd, RTCAN_RTIOC_GET_IFINDEX, &ifr);
    if (ret)
    {
        goto exit_error;
    }

    // Bind socket to default CAN IDs
    scan->can_family  = AF_CAN;
    scan->can_ifindex = ifr.ifr_ifindex;

    ret = rt_dev_bind(fd, (struct sockaddr *)scan, scan_size);
    if (ret)
    {
        goto exit_error;
    }

    return 0;

exit_error:
    if (fd != -1)
        close();

    return ret;
}

// non realtime context !!!
int CanPort::close(void)
{
    int i = 5;
    int ret;

    do
    {
        ret = rt_dev_close(fd);
        if (!ret) // exit on success
        {
            fd = -1;
            return 0;
        }
        else if (ret == -EAGAIN) // try it again (max 5 times)
        {
            sleep(1); // wait 1s
        }
    }
    while (ret != -EAGAIN && i--);

    return ret;
}

int CanPort::setTxTimeout(int64_t timeout)
{
    return rt_dev_ioctl(fd, RTCAN_RTIOC_SND_TIMEOUT, &timeout);
}

int CanPort::setRxTimeout(int64_t timeout)
{
    return rt_dev_ioctl(fd, RTCAN_RTIOC_RCV_TIMEOUT, &timeout);
}

int CanPort::getTimestamps()
{
    return rt_dev_ioctl(fd, RTCAN_RTIOC_TAKE_TIMESTAMP, RTCAN_TAKE_TIMESTAMPS);
}

int CanPort::send(rtcan_frame_t* frame)
{
    int ret;

    ret = rt_dev_send(fd, frame, sizeof(rtcan_frame_t), 0);
    if (ret != sizeof(rtcan_frame_t))
    {
        return ret;
    }
    return 0;
}

int CanPort::recv(rtcan_frame_t *recv_frame, uint64_t *timestamp)
{
    int ret;

    struct iovec  iov =
    {
        iov_base : recv_frame,
        iov_len  : sizeof(rtcan_frame_t)
    };

	struct msghdr msg =
	{
        msg_name       : NULL,
        msg_namelen    : 0,

	    msg_iov        : &iov,
        msg_iovlen     : 1,

        msg_control    : (void *)timestamp,
        msg_controllen : timestamp ? sizeof(uint64_t) : 0
	};

	ret = rt_dev_recvmsg(fd, &msg, 0);
    if (ret != sizeof(rtcan_frame_t))
    {
        return ret;
    }
    return 0;
}
