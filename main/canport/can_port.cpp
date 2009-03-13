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

  /*!
 * @ingroup driverapi
 * @defgroup rtcan CAN Port API
 *
 * This is the CAN Port interface of RACK provided to application programs
 * in userspace.
 * @{
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

/**
 * @brief Open a CAN device
 *
 * This function opens a new socket, gets the interface index and
 * binds the socket to the CAN IDs defined in the struct @a sockaddr_can.
 *
 * @param dev Number of the CAN device (e.g. 2 if you will open rtcan2)
 * @param filter_list Pointer to filter array (see RTDM CAN profile)
 * @param nr_filters Number of filter entries
 * @param module Pointer the RACK module. This pointer is needed to access
 *               the global timestamp.
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT)
 *
 * Rescheduling: possible.
 */
int CanPort::open(int dev, can_filter_t *filter_list, int nr_filters, RackModule *module)
{
    int                 ret;
    struct ifreq        ifr;
    struct sockaddr_can scan;

    if (fd != -1) // file is open
        return -EBUSY;

    // Prepare CAN socket and controller
    fd = rt_dev_socket(PF_CAN, SOCK_RAW, 0);
    if (fd < 0)
        return fd;

    // get interface index
    sprintf(ifr.ifr_name, "rtcan%d", dev);
    ret = rt_dev_ioctl(fd, SIOCGIFINDEX, &ifr);
    if (ret)
        goto exit_error;


    if (nr_filters > 0) {
        ret = rt_dev_setsockopt(fd, SOL_CAN_RAW, CAN_RAW_FILTER, filter_list,
                                nr_filters * sizeof(can_filter_t));
        if (ret)
            goto exit_error;
    }

    // Bind socket to default CAN IDs
    scan.can_family  = AF_CAN;
    scan.can_ifindex = ifr.ifr_ifindex;

    ret = rt_dev_bind(fd, (struct sockaddr *)&scan, sizeof(scan));
    if (ret)
        goto exit_error;

    this->module = module;
    return 0;

exit_error:
    if (fd != -1)
        close();

    return ret;
}

/**
 * @brief Close a CAN device
 *
 * This function closes a CAN device which was opened with @a open().
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT)
 *
 * Rescheduling: possible.
 */
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
            sleep(1); // wait 1s
    }
    while (ret != -EAGAIN && i--);

    return ret;
}

/**
 * @brief Set the send timeout of a CAN device
 *
 * @param timeout Send timeout in nanoseconds
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: none.
 */
int CanPort::setTxTimeout(int64_t timeout)
{
    return rt_dev_ioctl(fd, RTCAN_RTIOC_SND_TIMEOUT, &timeout);
}

/**
 * @brief Set the receive timeout of a CAN device
 *
 * @param timeout Receive timeout in nanoseconds
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: none.
 */
int CanPort::setRxTimeout(int64_t timeout)
{
    return rt_dev_ioctl(fd, RTCAN_RTIOC_RCV_TIMEOUT, &timeout);
}

/**
 * @brief Enable timestamp support
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: none.
 */
int CanPort::getTimestamps()
{
    return rt_dev_ioctl(fd, RTCAN_RTIOC_TAKE_TIMESTAMP, RTCAN_TAKE_TIMESTAMPS);
}

/**
 * @brief Send a CAN message
 *
 * @param frame Pointer to a CAN frame
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: none.
 */
int CanPort::send(can_frame_t* frame)
{
    int ret;

    ret = rt_dev_send(fd, frame, sizeof(can_frame_t), 0);
    if (ret < 0)
        return ret;

    return 0;
}

/**
 * @brief Receive a CAN message
 *
 * The @a recv() function receives a CAN message mit a given timeout set
 * with @a setRxTimeout().
 *
 * If timestamp support is enabled the pointer to the timestamp buffer have
 * to be set. The format of the returned timestamp is @a rack_time_t, thus
 * it is a global time value.
 *
 * @param recv_frame Pointer to a CAN frame
 * @param timestamp Pointer to a timestamp variable
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: possible.
 */
int CanPort::recv(can_frame_t *recv_frame, rack_time_t *timestamp)
{
    int ret;
    uint64_t timestamp_ns;

    struct iovec  iov =
    {
        iov_base : recv_frame,
        iov_len  : sizeof(can_frame_t)
    };

    struct msghdr msg =
    {
        msg_name       : NULL,
        msg_namelen    : 0,

        msg_iov        : &iov,
        msg_iovlen     : 1,
    };

    if (timestamp != NULL)
    {
        msg.msg_control    = &timestamp_ns;
        msg.msg_controllen = sizeof(uint64_t);
    }
    else
    {
        msg.msg_control    = NULL;
        msg.msg_controllen = 0;
    }

    ret = rt_dev_recvmsg(fd, &msg, 0);
    if (ret < 0)
        return ret;

    if (timestamp != NULL)
        *timestamp = module->rackTime.fromNano(timestamp_ns);

    return 0;
}

/*@}*/
