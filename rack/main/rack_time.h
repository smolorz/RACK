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

#ifndef __RACK_TIME_H__
#define __RACK_TIME_H__

 /*!
 * @ingroup rackos
 * @defgroup time Rack Time
 * @{
 */

/** Maximum RACK time value */
#define RACK_TIME_MAX           0x7fffffff

/** RACK time factor (1 ms) */
#define RACK_TIME_FACTOR          1000000

/** RTnet time reference device*/
#define RACK_TIME_REF_DEV      "TDMA0"

#include <main/rack_rtmac.h>
#include <native/timer.h>
#include <inttypes.h>


/** RACK time (32 Bit) */
typedef uint32_t rack_time_t;

class RackTime {
    private:

/** TDMA file decriptor */
        int32_t tdma_fd;

    public:

/** Global time offset */
        int64_t offset;

/** Global time flag */
        char    global;

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
            tdma_fd = -1;
            global  = 0;
            offset  = 0;
        }

/**
 * @brief RackTime destructor.
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT)
 *
 * Rescheduling: never.
 */
        ~RackTime()
        {
            cleanup();
        }

/**
 * @brief Cleanup the RackTime class. If the RTnet TDMA device has been opened
 * the device is closed.
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (RT, non-RT)
 *
 * Rescheduling: never.
 */
        void cleanup()
        {
            if (tdma_fd)
                rt_dev_close(tdma_fd);

            tdma_fd = -1;
            global = 0;
        }

/**
 * @brief Initializing the RackTime class. The function tries to open the
 * RTnet TDMA device. On success the global offset is fetched and the
 * global flag is set.
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
        int init()
        {
            int ret;
            int64_t offset;

            tdma_fd = rt_dev_open(RACK_TIME_REF_DEV, O_RDONLY);
            if (tdma_fd > -1)
            {
                global = 1;

                ret = getOffset(&offset);
                if (ret)
                {
                    cleanup();
                    return ret;
                }
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
            int64_t offset = 0;

            getOffset(&offset);
            return (rack_time_t)((ntime + offset) / RACK_TIME_FACTOR);
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
 * @brief Gets the current RACK time. If a global time offset is given the
 * offset is added.
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
            int64_t offset;

            getOffset(&offset);
            return (uint32_t)((rt_timer_read() + offset) / RACK_TIME_FACTOR);
        }

/**
 * @brief Gets the current time in nanoseconds. If a global time offset is
 * given the offset is added.
 *
 * @param[in,out] time_ns Pointer to the nanoseconds value
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
        int getNano(uint64_t *time_ns)
        {
            int ret;
            int64_t offset;

            ret = getOffset(&offset);
            if (ret)
                return ret;

            *time_ns = rt_timer_read() + offset;
            return 0;
        }

/**
 * @brief Gets the global offset in nanoseconds.
 *
 * @param[in,out] offset Pointer to the offset value
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
        int getOffset(int64_t *offset)
        {
            int ret;

            if (global)
            {
                ret = rt_dev_ioctl(tdma_fd, RTMAC_RTIOC_TIMEOFFSET, offset);
                if (ret)
                    return ret;
            }
            else
            {
                *offset = 0;
            }

            return 0;
        }
};

/** @} */

#endif // __RACK_TIME_H__
