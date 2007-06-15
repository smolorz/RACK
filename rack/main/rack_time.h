/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2007 University of Hannover
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
 *      Marko Reimer <reimer@rts.uni-hannover.de>
 *      Jan Kiszka <kiszka@rts.uni-hannover.de>
 *
 */

#ifndef __RACK_TIME_H__
#define __RACK_TIME_H__

#include <main/tims/tims.h>

 /*!
 * @ingroup rackos
 * @defgroup time Rack Time
 * @{
 */

/** Maximum RACK time value */
#define RACK_TIME_MAX           0x7fffffff

/** RACK time factor (1 ms) */
#define RACK_TIME_FACTOR          1000000

/** RACK time (32 Bit) */
typedef uint32_t rack_time_t;

#if defined (__XENO__) || defined (__KERNEL__)

#include <native/timer.h>

class RackTime
{
private:

    /** File decriptor to communicate with TIMS */
    int tims_fd;

public:

    /**
     * @brief RackTime constructor.
     *
     * Environments:
     *
     * This service can be called from:
     *
     * - User-space task (non-RT)
     *
     * Rescheduling: never.
     */
    RackTime()
    {
        tims_fd = -1;
    }

    /**
     * @brief Initializing the RackTime class. The function tries to open the
     * RTnet TDMA device. On success the global offset is fetched and the
     * global flag is set.
     *
     * @param[in] tims_fd File descriptor of a TIMS mailbox. Has to remain valid
     * as long as the RackTime instance is uses.
     *
     * @return 0 on success, otherwise negative error code
     *
     * Environments:
     *
     * This service can be called from:
     *
     * - User-space task (RT, non-RT)
     *
     * Rescheduling: never.
     */
    int init(int tims_fd)
    {
        int err;
        int64_t offset;

        this->tims_fd = tims_fd;
        err = rt_dev_ioctl(tims_fd, TIMS_RTIOC_GETCLOCKOFFSET, &offset);
        if (err)
        {
            tims_fd = -1;
            return err;
        }
        if (offset == 0)
        {
            /* Optimisation: we are (most probably) the time master */
            tims_fd = -1;
        }
        return 0;
    }

    /**
     * @brief Converting nanoseconds into rack_time_t. If a global time offset is
     * given the offset is added to the nanoseconds.
     *
     * @param[in] ntime Time in nanoseconds
     *
     * @return rack_time_t
     *
     * Environments:
     *
     * This service can be called from:
     *
     * - User-space task (RT, non-RT)
     *
     * Rescheduling: never.
     */
    rack_time_t fromNano(uint64_t ntime)
    {
        return (rack_time_t)((ntime + getOffset()) / RACK_TIME_FACTOR);
    }

    /**
     * @brief Converting a given rack_time_t value into nanoseconds.
     *
     * @param[in] rtime rack_time_t value
     *
     * @return Given rack_time_t in nanoseconds
     *
     * Environments:
     *
     * This service can be called from:
     *
     * - User-space task (RT, non-RT)
     *
     * Rescheduling: never.
     */
    uint64_t toNano(rack_time_t rtime)
    {
        return (uint64_t)(rtime * RACK_TIME_FACTOR) ;
    }

    /**
     * @brief Gets the current RACK time, synchonised on a reference clock if TIMS
     * is configured accordingly.
     *
     * @return Current rack_time_t
     *
     * Environments:
     *
     * This service can be called from:
     *
     * - User-space task (RT, non-RT)
     *
     * Rescheduling: never.
     */
    rack_time_t get(void)
    {
        return (rack_time_t)(getNano() / RACK_TIME_FACTOR);
    }

    /**
     * @brief Gets the current RACK time in nanoseconds, synchonised on a
     * reference clock if TIMS is configured accordingly.
     *
     * @return Current RACK time in nanoseconds
     *
     * Environments:
     *
     * This service can be called from:
     *
     * - User-space task (RT, non-RT)
     *
     * Rescheduling: never.
     */
    uint64_t getNano(void)
    {
        uint64_t ntime;

        if (tims_fd < 0 ||
            rt_dev_ioctl(tims_fd, TIMS_RTIOC_GETTIME, &ntime) < 0)
            return rt_timer_read();
        else
            return ntime;
    }

    /**
     * @brief Gets the offset to the reference clock in nanoseconds.
     *
     * @return Local clock offset (global = local + offset)
     *
     * Environments:
     *
     * This service can be called from:
     *
     * - User-space task (RT, non-RT)
     *
     * Rescheduling: never.
     */
    int64_t getOffset(void)
    {
        int64_t offset;

        if (tims_fd < 0 ||
            rt_dev_ioctl(tims_fd, TIMS_RTIOC_GETCLOCKOFFSET, &offset) < 0)
            offset = 0;

        return offset;
    }
};

#endif // __XENO__ || __KERNEL__

/** @} */

#endif // __RACK_TIME_H__
