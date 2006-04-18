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
#ifndef __RACK_TASK_H__
#define __RACK_TASK_H__

#include <inttypes.h>

#include <native/task.h>

#ifndef RACK_INFINITE
#define RACK_INFINITE	   TM_INFINITE
#endif

#ifndef RACK_NONBLOCK
#define RACK_NONBLOCK	   TM_NONBLOCK
#endif

typedef RT_TASK RACK_TASK;

class RackTask
{
    private:
        int init;
        RT_TASK task;

    public:
        RackTask()
        {
            init = 0;
        }

        ~RackTask()
        {
            if (init)
                destroy();
        }

        // non realtime context
        int create(int stksize, int prio, int mode)
        {
            int ret;

            if (init)
                return -EBUSY;

            ret = rt_task_create(&task, NULL, stksize, prio, mode);
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

            ret = rt_task_delete(&task);
            if (ret)
            {
                return ret;
            }

            init = 0;
            return 0;
        }

        // non realtime context
        int start(void (*fun)(void *cookie), void *cookie)
        {
            return rt_task_start(&task, fun, cookie);
        }

        // non realtime context
        int join(void)
        {
            return rt_task_join(&task);
        }

        // realtime context
        static int setMode(int clrmask, int setmask, int *mode_r)
        {
            return rt_task_set_mode(clrmask, setmask, mode_r);
        }

        // realtime context
        static int sleep(uint64_t delay)
        {
            return rt_task_sleep((RTIME)delay);
        }

        // realtime context
        static int sleepUntil(int64_t date)
        {
            return rt_task_sleep_until(date);
        }

        // set task into primary mode and enable the SIGXCPU signal to
        // detect an unexpected switch to secondary mode
		static int enableRealtimeMode()
		{
			return setMode(0, T_PRIMARY | T_WARNSW, NULL);
		}

        // set task into secondary mode and disable the SIGXCPU signal
		static int disableRealtimeMode()
		{
			return setMode(T_PRIMARY | T_WARNSW, 0, NULL);
		}

};

/* not supported

int suspend(RACK_TASK *task);

int resume(RACK_TASK *task);

int yield(void);

int set_periodic(RACK_TASK *task, int64_t idate, int64_t period);

int wait_period(void);

int set_priority(RACK_TASK *task, int prio);

int unblock(RACK_TASK *task);

int inquire(RACK_TASK *task, RT_TASK_INFO *info);

int catch(void (*handler)(rt_sigset_t));

int notify(RACK_TASK *task, rt_sigset_t signals);

*/

#endif
