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
 *      Oliver Wulf <oliver.wulf@web.de>
 *
 */

#include <main/rack_mutex.h>

RackMutex::RackMutex()
{
    init = 0;
}

RackMutex::~RackMutex()
{
    if  (init)
        destroy();
}

int RackMutex::create(void)
{
    int ret;

    if (init)
        return -EEXIST;

    ret = rt_mutex_create(&mutex, NULL);
    if (ret)
        return ret;

    init = 1;
    return 0;
}

int RackMutex::destroy(void)
{
    int ret;

    if (!init)
        return -EINVAL;

    ret = rt_mutex_delete(&mutex);
    if (ret)
        return ret;

    init = 0;
    return 0;
}

int RackMutex::lock(int64_t timeout)
{
#if XENO_VERSION_CODE < XENO_VERSION(2,4,90)
    return rt_mutex_lock(&mutex, timeout);
#else
    return rt_mutex_acquire(&mutex, timeout);
#endif
}

int RackMutex::lock(void)
{
    return lock(RACK_INFINITE);
}

int RackMutex::unlock(void)
{
#if XENO_VERSION_CODE < XENO_VERSION(2,4,90)
    return rt_mutex_unlock(&mutex);
#else
    return rt_mutex_release(&mutex);
#endif
}
