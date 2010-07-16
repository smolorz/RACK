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
 *      Oliver Wulf <oliver.wulf@web.de> - add linux implementation based on Socket-CAN API
 *
 */
#ifndef __CAN_PORT_H__
#define __CAN_PORT_H__

#include <main/rack_time.h>
#include <main/rack_module.h>

#if defined (__XENO__) || defined (__KERNEL__)

#include <rtdm/rtcan.h>

#else // !__XENO__ && !__KERNEL__

#include <linux/can.h>
#include <linux/can/raw.h>

typedef struct can_frame can_frame_t;
typedef struct can_filter can_filter_t;

#endif // !__XENO__ && !__KERNEL__

/*!
 * @ingroup driverapi
 * @defgroup rtcan CAN Port API
 *
 * This is the CAN Port interface of RACK provided to application programs
 * in userspace.
 * @{
 */

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
        int open(int dev, can_filter_t *filter_list, int nr_filters, RackModule *module);

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
        int close(void);



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
        int setTxTimeout(int64_t timeout);

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
        int setRxTimeout(int64_t timeout);

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
        int getTimestamps();



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
        int send(can_frame_t *frame);

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
        int recv(can_frame_t *recv_frame, rack_time_t *timestamp);
};

/*@}*/

#endif // __CAN_PORT_H__
