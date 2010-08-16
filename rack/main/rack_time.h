/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2009 University of Hannover
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
 *      Oliver Wulf <oliver.wulf@web.de>
 *      Sebastian Smolorz <smolorz@rts.uni-hannover.de>
 *
 */

#ifndef __RACK_TIME_H__
#define __RACK_TIME_H__

#include <inttypes.h>

/**
 * Maximum RACK time value
 * @ingroup rack_os_abstraction
 */
#define RACK_TIME_MAX           0x7fffffff

/**
 * RACK time factor (1 ms)
 * @ingroup rack_os_abstraction
 */
#define RACK_TIME_FACTOR          1000000llu

/** RACK time (32 Bit)
 * @ingroup rack_os_abstraction
 */
typedef uint32_t rack_time_t;

/**
 * @ingroup main_os_abstraction
 */
class RackTime
{
#if defined (__XENO__)

private:
    /** File decriptor to communicate with TIMS */
    int tims_fd;

#else // !__XENO__

private:

#endif // __XENO__

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
    RackTime();

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
    int init(int tims_fd);

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
    rack_time_t fromNano(uint64_t ntime);

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
    uint64_t toNano(rack_time_t rtime);

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
    rack_time_t get(void);

    int set(uint64_t utcTimestamp, uint64_t recTimestamp);

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
    uint64_t getNano(void);

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
    int64_t getOffset(void);

};

#endif // __RACK_TIME_H__
