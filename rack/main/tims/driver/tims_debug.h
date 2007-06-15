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
 *      Sebastian Smolorz <Sebastian.Smolorz@stud.uni-hannover.de>
 *
 */
 #ifndef __TIMS_DEBUG_H__
 #define __TIMS_DEBUG_H__

#include "tims_driver.h"

#define TIMS_LEVEL_PRINT          0
#define TIMS_LEVEL_ERROR          1
#define TIMS_LEVEL_INFO           2
#define TIMS_LEVEL_WARN           3
#define TIMS_LEVEL_DBG_INFO       4
#define TIMS_LEVEL_DBG_DETAIL     5

#define TIMS_LEVEL_MAX            TIMS_LEVEL_DBG_DETAIL


#define tims_print(fmt, ... )    \
                                do {                                         \
                                  if (dbglevel >= TIMS_LEVEL_PRINT)          \
                                    rtdm_printk("TIMS: <0x%p>: "fmt,         \
                                                current, ##__VA_ARGS__);     \
                                } while(0)

#define tims_print_0(fmt, ... ) \
                                do {                                         \
                                  if (dbglevel >= TIMS_LEVEL_PRINT)          \
                                    rtdm_printk(fmt, ##__VA_ARGS__);         \
                                } while(0)

#define tims_error(fmt, ... )   \
                                do {                                         \
                                  if (dbglevel >= TIMS_LEVEL_ERROR)          \
                                    rtdm_printk("TIMS: <0x%p>: ERROR: "fmt,  \
                                                current, ##__VA_ARGS__);     \
                                } while(0)

#define tims_error_0(fmt, ... ) \
                                do {                                         \
                                  if (dbglevel >= TIMS_LEVEL_ERROR)          \
                                    rtdm_printk(fmt, ##__VA_ARGS__);         \
                                } while(0)

#define tims_info(fmt, ... )    \
                                do {                                         \
                                  if (dbglevel >= TIMS_LEVEL_INFO)           \
                                    rtdm_printk("TIMS: <0x%p>: INFO:  "fmt,  \
                                                current, ##__VA_ARGS__);     \
                                } while(0)

#define tims_info_0(fmt, ... )  \
                                do {                                         \
                                  if (dbglevel >= TIMS_LEVEL_INFO)           \
                                    rtdm_printk(fmt, ##__VA_ARGS__);         \
                                } while(0)

#define tims_warn(fmt, ... )    \
                                do {                                         \
                                  if (dbglevel >= TIMS_LEVEL_WARN)           \
                                    rtdm_printk("TIMS: <0x%p>: WARN:  "fmt,  \
                                                current, ##__VA_ARGS__);     \
                                } while(0)

#define tims_warn_0(fmt, ... )  \
                                do {                                         \
                                  if (dbglevel >= TIMS_LEVEL_WARN)           \
                                    rtdm_printk(fmt, ##__VA_ARGS__);         \
                                } while(0)

#define tims_dbginfo(fmt, ... ) \
                                do {                                         \
                                  if (dbglevel >= TIMS_LEVEL_DBG_INFO)       \
                                    rtdm_printk("TIMS: <0x%p>: DBG_I: "fmt,  \
                                                current, ##__VA_ARGS__);     \
                                } while(0)

#define tims_dbginfo_0(fmt, ... ) \
                                do {                                         \
                                  if (dbglevel >= TIMS_LEVEL_DBG_INFO)       \
                                    rtdm_printk(fmt, ##__VA_ARGS__);         \
                                } while(0)

#define tims_dbgdetail(fmt, ... ) \
                                do {                                         \
                                  if (dbglevel >= TIMS_LEVEL_DBG_DETAIL)     \
                                    rtdm_printk("TIMS: <0x%p>: DBG_D: "fmt,  \
                                                current, ##__VA_ARGS__);     \
                                } while(0)

#define tims_dbgdetail_0(fmt, ... ) \
                                do {                                         \
                                  if (dbglevel >= TIMS_LEVEL_DBG_DETAIL)     \
                                    rtdm_printk(fmt, ##__VA_ARGS__);         \
                                } while(0)

extern int dbglevel;

 #endif // __TIMS_DEBUG_H__
