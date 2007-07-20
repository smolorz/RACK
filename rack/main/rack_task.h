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
#ifndef __RACK_TASK_H__
#define __RACK_TASK_H__

#if defined (__XENO__) || defined (__KERNEL__)

 /*!
 * @ingroup rackos
 * @defgroup task Rack Task
 * @{
 */

#include <native/task.h>

#ifndef RACK_INFINITE
#define RACK_INFINITE       TM_INFINITE
#endif

#ifndef RACK_NONBLOCK
#define RACK_NONBLOCK       TM_NONBLOCK
#endif

#define RACK_TASK_FPU       T_FPU
#define RACK_TASK_JOINABLE  T_JOINABLE

class RackTask
{
    private:
        int init;
        RT_TASK task;

    public:

/**
 * @brief RACK task constructor.
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT)
 *
 * Rescheduling: never.
 */
        RackTask()
        {
            init = 0;
        }

/**
 * @brief RACK task destructor.
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT)
 *
 * Rescheduling: never.
 */
        ~RackTask()
        {
            if (init)
                destroy();
        }

/**
 * @brief Create a new RACK task.
 *
 * @param[in] name Task name.
 *
 * @param[in] stksize The size of the stack (in bytes) for the new task.
 * If zero is passed, a reasonable pre-defined size will be substituted.
 *
 * @param[in] prio The base priority of the new task. This value must range
 * from [1 .. 99] (inclusive) where 1 is the lowest effective priority.
 *
 * @param[in] mode The task creation mode. The following flags can be OR'ed
 * into this bitmask, each of them affecting the new task:
 *
 * - T_FPU allows the task to use the FPU whenever available on the
 * platform. This flag is forced for user-space tasks.
 *
 * - T_JOINABLE allows another task to wait on the termination of the
 * new task. This implies that RackTask.join() is actually called for this
 * task to clean up any user-space located resources after its termination.
 *
 * Passing T_FPU|T_JOINABLE in the @a mode parameter thus creates a task
 * with FPU support enabled and which will be joinable.
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task
 *
 * Rescheduling: possible.
 */
        int create(const char *name, int stksize, int prio, int mode)
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

/**
 * @brief Delete a RACK task.
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task
 *
 * Rescheduling: possible.
 */
        int destroy(void)
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

/**
  * @brief Start a RACK task.
 *
 * Start a (newly) created task, scheduling it for the first time.
 *
 * @param fun The address of the task's body routine. In other
 * words, it is the task entry point.
 *
 * @param cookie A user-defined opaque cookie the real-time kernel
 * will pass to the emerging task as the sole argument of its entry
 * point.
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task
 *
 * Rescheduling: possible.
 */
        int start(void (*fun)(void *cookie), void *cookie)
        {
            return rt_task_start(&task, fun, cookie);
        }

/**
 * @brief Wait on the termination of a real-time task.
 *
 * This user-space only service blocks the caller in non-real-time context
 * until @a task has terminated. Note that the specified task must have
 * been created with the T_JOINABLE mode flag set.
 *
 * @return 0 is returned upon success. Otherwise:
 *
 * - -EINVAL is returned if the task was not created with T_JOINABLE set or
 * some other task is already waiting on the termination.
 *
 * - -EDEADLK is returned if @a task refers to the caller.
 *
 * This service can be called from:
 *
 * - User-space task.
 *
 * Rescheduling: always unless the task was already terminated.
 */

/*@}*/
        int join(void)
        {
            //rt_task_unblock()
            return rt_task_join(&task);
        }

/**
 * @brief Change task mode bits.
 *
 * Each Xenomai task has a set of internal bits determining various
 * operating conditions. RackTask::setMode() takes a bitmask of mode
 * bits to clear for disabling the corresponding modes, and another
 * one to set for enabling them. The mode bits which were previously
 * in effect can be returned upon request.
 *
 * The following supported bits can be part of the bitmask:
 *
 * - When set, T_WARNSW causes the SIGXCPU signal to be sent to the
 * current user-space task whenever it switches to the secondary
 * mode. This feature is useful to detect unwanted migrations to the
 * Linux domain.
 *
 * - T_PRIMARY can be passed to switch the current user-space task to
 * primary mode (setmask |= T_PRIMARY), or secondary mode (clrmask |=
 * T_PRIMARY). Upon return from rt_task_set_mode(), the user-space
 * task will run into the specified domain.
 *
 * @param clrmask A bitmask of mode bits to clear for the current
 * task, before @a setmask is applied. 0 is an acceptable value which
 * leads to a no-op.
 *
 * @param setmask A bitmask of mode bits to set for the current
 * task. 0 is an acceptable value which leads to a no-op.
 *
 * @param mode_r If non-NULL, @a mode_r must be a pointer to a memory
 * location which will be written upon success with the previous set
 * of active mode bits. If NULL, the previous set of active mode bits
 * will not be returned.
 *
 * @return 0 is returned upon success, or:
 *
 * - -EINVAL if either @a setmask or @a clrmask specifies invalid
 * bits. T_PRIMARY is invalid for kernel-based tasks.
 *
 * - -EPERM is returned if this service was not called from a
 * real-time task context.
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task
 *
 * Rescheduling: possible
 */
        static int setMode(int clrmask, int setmask, int *mode_r)
        {
            return rt_task_set_mode(clrmask, setmask, mode_r);
        }

/**
 * @brief Delay the calling task (relative).
 *
 * Delay the execution of the calling task by given nanoseconds.
 *
 * @param delay of nanoseconds to wait before resuming the task.
 * Passing zero causes the task to return immediately with no delay.
 *
 * @return 0 is returned upon success, otherwise:
 *
 * - -EINTR is returned if this sleeping task is waked up before the
 * sleep time has elapsed.
 *
 * - -EWOULDBLOCK is returned if the system timer is inactive.
 *
 * - -EPERM is returned if this service was called from a context
 * which cannot sleep (e.g. interrupt, non-realtime or scheduler
 * locked).
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (switches to primary mode)
 *
 * Rescheduling: always unless a null delay is given.
 */
        static int sleep(uint64_t delay)
        {
            return rt_task_sleep((RTIME)delay);
        }

/**
 * @brief Delay the calling task (absolute).
 *
 * Delay the execution of the calling task until a given date is
 * reached.
 *
 * @param date The absolute date in nanoseconds to wait before resuming
 * the task. Passing an already elapsed date causes the task to return
 * immediately with no delay.
 *
 * @return 0 is returned upon success. Otherwise:
 *
 * - -EINTR is returned if this sleeping task is waked up before the
 * sleep time has elapsed.
 *
 * - -ETIMEDOUT is returned if @a date has already elapsed.
 *
 * - -EWOULDBLOCK is returned if the system timer is inactive.
 *
 * - -EPERM is returned if this service was called from a context
 * which cannot sleep (e.g. interrupt, non-realtime or scheduler
 * locked).
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (switches to primary mode)
 *
 * Rescheduling: always unless a date in the past is given.
 */
        static int sleepUntil(int64_t date)
        {
            return rt_task_sleep_until(date);
        }

/**
 * @brief Set current task into realtime mode.
 *
 * RackTask::enableRealtimeMode() enables the primary mode of the
 * current task. For that purpose the mode bits T_PRIMARY and T_WARNSW
 * are set. The T_WARNSW flag enables the SIGXCPU signal to detect an
 * unexpected switch to secondary mode.
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (switches to primary mode)
 *
 * Rescheduling: possible.
 */
        static int enableRealtimeMode()
        {
            return setMode(0, T_PRIMARY | T_WARNSW, NULL);
        }

/**
 * @brief Set current task into secondary mode.
 *
 * RackTask::disableRealtimeMode() disables the primary mode of the
 * current task. For that purpose the mode bits T_PRIMARY and T_WARNSW
 * are cleared.
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (switches to secondary mode)
 *
 * Rescheduling: possible.
 */
        static int disableRealtimeMode()
        {
            return setMode(T_PRIMARY | T_WARNSW, 0, NULL);
        }

};

/** @} */

#else // !__XENO__ && !__KERNEL__

#include <pthread.h>
#include <errno.h>

#define RACK_TASK_FPU       1
#define RACK_TASK_JOINABLE  2

class RackTask
{
    private:
        int init;
        pthread_t task;

    public:

        RackTask()
        {
        }

        ~RackTask()
        {
        }

        int create(const char *name, int stksize, int prio, int mode)
        {
            return 0;
        }

        int destroy(void)
        {
            return 0;
        }

        int start(void (*fun)(void *cookie), void *cookie)
        {
            return pthread_create(&task, NULL, (void *(*)(void *))fun, cookie);
        }

        int join(void)
        {
            //pthread_cancel
            return pthread_join(task, NULL);
        }

        static int setMode(int clrmask, int setmask, int *mode_r)
        {
            return 0;
        }

        static int sleep(uint64_t delay)
        {
            return usleep(delay / (uint64_t)1000);
        }

        static int sleepUntil(int64_t date)
        {
            return -1;//return rt_task_sleep_until(date);
        }

        static int enableRealtimeMode()
        {
            return 0;
        }

        static int disableRealtimeMode()
        {
            return 0;
        }

};

#endif // __XENO__ || __KERNEL__

#endif
