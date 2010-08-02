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

#include <main/rack_time.h>
#include <main/tims/tims_rtdm.h>

#include <native/timer.h>

RackTime::RackTime()
{
    tims_fd = -1;
}

int RackTime::init(int tims_fd)
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

rack_time_t RackTime::fromNano(uint64_t ntime)
{
    return (rack_time_t)((ntime + getOffset()) / RACK_TIME_FACTOR);
}

uint64_t RackTime::toNano(rack_time_t rtime)
{
    return (uint64_t)(rtime * RACK_TIME_FACTOR) ;
}

rack_time_t RackTime::get(void)
{
    return (rack_time_t)(getNano() / RACK_TIME_FACTOR);
}

int RackTime::set(uint64_t utcTimestamp, uint64_t recTimestamp)
{
    tims_clock_setvalue setValue = {utcTimestamp, recTimestamp};

    if (tims_fd < 0 ||
                rt_dev_ioctl(tims_fd, TIMS_RTIOC_SETTIME, &setValue) < 0)
        return -EINVAL;
    else
        return 0;
}

uint64_t RackTime::getNano(void)
{
    uint64_t ntime;

    if (tims_fd < 0 ||
        rt_dev_ioctl(tims_fd, TIMS_RTIOC_GETTIME, &ntime) < 0)
        return rt_timer_read();
    else
        return ntime;
}

int64_t RackTime::getOffset(void)
{
    int64_t offset;

    if (tims_fd < 0 ||
        rt_dev_ioctl(tims_fd, TIMS_RTIOC_GETCLOCKOFFSET, &offset) < 0)
        offset = 0;

    return offset;
}
