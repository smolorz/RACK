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

/*!
 * \ingroup rackos
 * \defgroup mutex Rack Mutex
 *
 * Mutex services.
 *
 * A mutex is a MUTual EXclusion object, and is useful for protecting
 * shared data structures from concurrent modifications, and
 * implementing critical sections and monitors.
 *
 * A mutex has two possible states: unlocked (not owned by any task),
 * and locked (owned by one task). A mutex can never be owned by two
 * different tasks simultaneously. A task attempting to lock a mutex
 * that is already locked by another task is blocked until the latter
 * unlocks the mutex first.
 *
 * RACK (Xenomai) mutex services enforce a priority inheritance protocol in
 * order to solve priority inversions.
 *
 *@{*/

/** Infinite timeout */
#ifndef RACK_INFINITE
#define RACK_INFINITE    TM_INFINITE
#endif

/** Non blocking */
#ifndef RACK_NONBLOCK
#define RACK_NONBLOCK    TM_NONBLOCK
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

/**
 * @brief Create a mutex.
 *
 * Create a mutual exclusion object that allows multiple tasks to
 * synchronize access to a shared resource. A mutex is left in an
 * unlocked state after creation.
 *
 * @return 0 is returned upon success. Otherwise:
 *
 * - -ENOMEM is returned if the system fails to get enough dynamic
 * memory from the global real-time heap in order to register the
 * mutex.
 *
 * - -EEXIST is returned if the @a name is already in use by some
 * registered object.
 *
 * - -EPERM is returned if this service was called from an
 * asynchronous context.
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task
 *
 * Rescheduling: possible.
 */
        int create(void)
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

/**
 * @brief Delete a mutex.
 *
 * Destroy a mutex and release all the tasks currently pending on it.
 * A mutex exists in the system since RackNutex::create() has been
 * called to create it, so this service must be called in order to
 * destroy it afterwards.
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
                return -EINVAL;

            ret = rt_mutex_delete(&mutex);
            if (ret)
                return ret;

            init = 0;
            return 0;
        }

/**
 * @brief Acquire a mutex.
 *
 * Attempt to lock a mutex. The calling task is blocked until the
 * mutex is available, in which case it is locked again before this
 * service returns. Mutexes have an ownership property, which means
 * that their current owner is tracked. Xenomai mutexes are implicitely
 * recursive and implement the priority inheritance protocol.
 *
 * Since a nested locking count is maintained for the current owner,
 * RackMutex::lock() and RackMutex::unlock() must be used in pairs.
 *
 * Tasks pend on mutexes by priority order.
 *
 * @param timeout The number of nanoseconds to wait for the mutex to
 * be available to the calling task. Passing RACK_INFINITE
 * causes the caller to block indefinitely until the mutex is
 * available. Passing RACK_NONBLOCK causes the service to return
 * immediately without waiting if the mutex is still locked by another
 * task.
 *
 * @return 0 is returned upon success. Otherwise:
 *
 * - -EINVAL is returned if the mutex is not a mutex descriptor
 * (internal error).
 *
 * - -EIDRM is returned if the mutex is a deleted mutex descriptor,
 * including if the deletion occurred while the caller was sleeping on
 * it (internal error).
 *
 * - -EWOULDBLOCK is returned if @a timeout is equal to RACK_NONBLOCK
 * and the mutex is not immediately available.
 *
 * - -EINTR is returned if RackTask::unblock() has been called for the
 * waiting task before the mutex has become available.
 *
 * - -ETIMEDOUT is returned if the mutex cannot be made available to
 * the calling task within the specified amount of time.
 *
 * - -EPERM is returned if this service was called from a context
 * which cannot be given the ownership of the mutex (e.g. interrupt,
 * non-realtime or scheduler locked).
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (switches to primary mode)
 *
 * Rescheduling: always unless the request is immediately satisfied or
 * @a timeout specifies a non-blocking operation.  If the caller is
 * blocked, the current owner's priority might be temporarily raised
 * as a consequence of the priority inheritance protocol.
 */
        int lock(int64_t timeout)
        {
            return rt_mutex_lock(&mutex, timeout);
        }

/**
 * @brief Unlock mutex.
 *
 * Release a mutex. If the mutex is pended, the first waiting task (by
 * priority order) is immediately unblocked and transfered the
 * ownership of the mutex; otherwise, the mutex is left in an unlocked
 * state.
 *
 * @return 0 is returned upon success. Otherwise:
 *
 * - -EINVAL is returned if the mutex is not a mutex descriptor
 * (internal error).
 *
 * - -EIDRM is returned if the mutex is a deleted mutex descriptor
 * (internal error).
 *
 * - -EPERM is returned if the mutex is not owned by the current task,
 * or more generally if this service was called from a context which
 * cannot own any mutex (e.g. interrupt, or non-realtime context).
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (switches to primary mode)
 *
 * Rescheduling: possible.
 */
        int unlock(void)
        {
            return rt_mutex_unlock(&mutex);
        }

};

/*@}*/

#endif // __RACK_MUTEX_H__
