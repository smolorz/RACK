/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2007 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Authors
 *      Marko Reimer <reimer@rts.uni-hannover.de>
 *      Jan Kiszka <kiszka@rts.uni-hannover.de>
 *
 */
#ifndef __TIMS_CLOCK_H__
#define __TIMS_CLOCK_H__

#include <rtdm/rtdm_driver.h>

extern struct rtdm_dev_context *sync_dev_ctx;

static inline void tims_clock_open(void)
{
    if (sync_dev_ctx)
        rtdm_context_lock(sync_dev_ctx);
}

void tims_clock_close(void)
{
    if (sync_dev_ctx)
        rtdm_context_unlock(sync_dev_ctx);
}

int tims_clock_ioctl(rtdm_user_info_t *user_info, unsigned int request,
                     void *arg);
int tims_clock_init(void);
void tims_clock_cleanup(void);

#endif /* !__TIMS_CLOCK_H__ */

