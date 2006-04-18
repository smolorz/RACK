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
#ifndef __RACK_MUTEX_H__
#define __RACK_MUTEX_H__

#include <native/mutex.h>

#ifndef RACK_INFINITE
#define RACK_INFINITE	TM_INFINITE
#endif

#ifndef RACK_NONBLOCK
#define RACK_NONBLOCK	TM_NONBLOCK
#endif

class RackMutex
{
    private:
        int init;
        RT_MUTEX  mutex;

    public:
        RackMutex()
        {
            init = 0;
        }

        ~RackMutex()
        {
            if  (init)
                destroy();
        }

        // non realtime context
        int create(void)
        {
            int ret;

            if (init)
                return -EBUSY;

            ret = rt_mutex_create(&mutex, NULL);
            if (ret)
            {
                return ret;
            }

            init = 1;
            return 0;
        }

        // non realtime context
        int destroy(void)
        {
            int ret;

            if (!init)
                return -ENOSYS;

            ret = rt_mutex_delete(&mutex);
            if (ret)
            {
                return ret;
            }

            init = 0;
            return 0;
        }

        // realtime context
        int lock(int64_t timeout)
        {
            return rt_mutex_lock(&mutex, timeout);
        }

        // realtime context
        int unlock(void)
        {
            return rt_mutex_unlock(&mutex);
        }

};

#endif // __RACK_MUTEX_H__
