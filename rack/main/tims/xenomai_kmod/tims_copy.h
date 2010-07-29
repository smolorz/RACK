/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg  <joerg.langenberg@gmx.net>
 *
 */
#ifndef __TIMS_COPY_H__
#define __TIMS_COPY_H__

#include "tims_driver.h"

int copy_msg_into_slot(rtdm_user_info_t *user_info, tims_mbx_slot *slot,
                       const struct msghdr *msg, unsigned long mbxFlags);

int copy_msg_out_slot(rtdm_user_info_t *user_info, tims_mbx_slot *slot,
                      const struct msghdr *msg, unsigned long mbxFlags);

#endif // __TIMS_COPY_H__
