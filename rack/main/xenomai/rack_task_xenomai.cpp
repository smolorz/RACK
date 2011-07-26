/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2011 University of Hannover
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
 *      Sebastian Smolorz <sebastian.smolorz@gmx.de>
 *
 */

#include <main/rack_task.h>

RackTask::RackTask()
{
    init = 0;
}

RackTask::~RackTask()
{
    if (init)
        destroy();
}

int RackTask::create(const char *name, int stksize, int prio, int mode)
{
    int ret;

    if (init)
        return -EBUSY;

    ret = rt_task_create(&task, name, stksize, prio, mode);
    if (ret)
        return ret;

    init = 1;
    return 0;
}

int RackTask::destroy(void)
{
    int ret;

    if (!init)
        return -ENOSYS;

    ret = rt_task_delete(&task);
    if (ret)
        return ret;

    init = 0;
    return 0;
}

int RackTask::start(void (*fun)(void *cookie), void *cookie)
{
    return rt_task_start(&task, fun, cookie);
}

int RackTask::join(void)
{
    //rt_task_unblock()
    return rt_task_join(&task);
}

int RackTask::setMode(int clrmask, int setmask, int *mode_r)
{
    return rt_task_set_mode(clrmask, setmask, mode_r);
}

int RackTask::sleep(uint64_t delay)
{
    return rt_task_sleep((RTIME)delay);
}

int RackTask::sleepUntil(int64_t date)
{
    return rt_task_sleep_until(date);
}

int RackTask::enableRealtimeMode()
{
    return setMode(0, T_WARNSW, NULL);
}

int RackTask::disableRealtimeMode()
{
    return setMode(T_WARNSW, 0, NULL);
}
