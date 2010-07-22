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

#include <errno.h>
#include <stdio.h>

RackMutex::RackMutex()
{
}

RackMutex::~RackMutex()
{
    destroy();
}

int RackMutex::create(void)
{
    int ret;

    ret = sem_init(&sem, 0, 1);

    if(ret)
    {
        return errno;
    }
    else
    {
        return 0;
    }
}

int RackMutex::destroy(void)
{

    int ret;

    ret = sem_destroy(&sem);

    if(ret)
    {
        return errno;
    }
    else
    {
        return 0;
    }
}

int RackMutex::lock(int64_t timeout)
{
    return sem_wait(&sem);
}

int RackMutex::lock(void)
{
    return lock(RACK_INFINITE);
}

int RackMutex::unlock(void)
{
    return sem_post(&sem);
}
