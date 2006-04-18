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
#ifndef __RACK_TIME_H__
#define __RACK_TIME_H__

#define RACK_TIME_MAX 0x7fffffff

#include <inttypes.h>

#include <native/timer.h>

// time in ms
typedef uint32_t RACK_TIME;

#ifdef __cplusplus
extern "C" {
#endif

static inline RACK_TIME ns_to_rack_time(uint64_t ntime) {
  return (uint32_t)(ntime / 1000000) ;
}

static inline uint64_t rack_time_to_ns(RACK_TIME rtime) {
  return (uint64_t)(rtime * 1000000) ;
}

static inline RACK_TIME get_rack_time(void) {
  return (uint32_t)(rt_timer_read() / 1000000) ;
}

static inline uint64_t get_time_ns(void) {
  return (uint64_t)(rt_timer_read()) ;
}

#ifdef __cplusplus
}
#endif

#endif // __RACK_TIME_H__
