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

#include <main/rack_task.h>

#include <errno.h>
#include <unistd.h>
#include <stdio.h>

RackTask::RackTask()
{
}

RackTask::~RackTask()
{
}

int RackTask::create(const char *name, int stksize, int prio, int mode)
{
    // function not needed in linux implementation
    return 0;
}

int RackTask::destroy(void)
{
    // function not needed in linux implementation
    return 0;
}

int RackTask::start(void (*fun)(void *cookie), void *cookie)
{
    return pthread_create(&task, NULL, (void *(*)(void *))fun, cookie);
}

int RackTask::join(void)
{
    //pthread_cancel
    return pthread_join(task, NULL);
}

int RackTask::setMode(int clrmask, int setmask, int *mode_r)
{
    // function not needed in linux implementation
    return 0;
}

int RackTask::sleep(uint64_t delay)
{
    // convert delay in nano seconds to usleep in micro seconds
    return usleep(delay / (uint64_t)1000);
}

int RackTask::sleepUntil(int64_t date)
{
    printf("Function RackTask::sleepUnitl() is not jet implemented\n");
    return -1;
}

int RackTask::enableRealtimeMode()
{
    // function not needed in linux implementation
    return 0;
}

int RackTask::disableRealtimeMode()
{
    // function not needed in linux implementation
    return 0;
}
