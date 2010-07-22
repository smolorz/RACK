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

#include <sys/time.h>
#include <stdio.h>

RackTime::RackTime()
{
}

int RackTime::init(int tims_fd)
{
    return 0;
}

rack_time_t RackTime::fromNano(uint64_t ntime)
{
    return (rack_time_t)(ntime / RACK_TIME_FACTOR);
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
    return 0;
}

uint64_t RackTime::getNano(void)
{
    struct timeval time;
    uint64_t nanoTime;

    gettimeofday(&time, NULL);

    nanoTime = (uint64_t) time.tv_sec * 1000000000llu + (uint64_t) time.tv_usec * 1000llu;

    return nanoTime;
}

int64_t RackTime::getOffset(void)
{
    return 0;
}
