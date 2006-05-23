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

#define RACK_TIME_MAX           0x7fffffff
#define FACTOR                  1000000
#define TIME_REFERENCE_DEV      "TDMA0"

#include <main/rack_rtmac.h>
#include <native/timer.h>
#include <inttypes.h>


// time in ms
typedef uint32_t RACK_TIME;

class RackTime {
    private:
        int32_t tdma_fd;

    public:
        int64_t offset;
        char    global;

        RackTime()
        {
            tdma_fd = -1;
            global  = 0;
            offset  = 0;
        }

        ~RackTime()
        {
            cleanup();
        }

        void cleanup()
        {
            if (tdma_fd)
                rt_dev_close(tdma_fd);

            tdma_fd = -1;
            global = 0;
        }

        int init()
        {
            int ret;
            int64_t offset;

            tdma_fd = rt_dev_open(TIME_REFERENCE_DEV, O_RDONLY);
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


        // if you get the time in ns (e.g. by a driver) there is not an offset added
        RACK_TIME fromNano(uint64_t ntime)
        {
            int64_t offset = 0;

            getOffset(&offset);
            return (uint32_t)((ntime + offset) / FACTOR);
        }

        uint64_t toNano(RACK_TIME rtime)
        {
            return (uint64_t)(rtime * FACTOR) ;
        }

        RACK_TIME get(void)
        {
            int64_t offset;

            getOffset(&offset);
            return (uint32_t)((rt_timer_read() + offset) / FACTOR);
        }

        int getNano(uint64_t *time_ns)
        {
            int ret;
            int64_t offset;

            ret = getOffset(&offset);
            if (ret)
                return ret;

            return rt_timer_read() + offset;
        }

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

#endif // __RACK_TIME_H__
