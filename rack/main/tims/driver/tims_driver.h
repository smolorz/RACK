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
 *      Joerg Langenberg  <joerg.langenberg@gmx.net>
 *      Sebastian Smolorz <Sebastian.Smolorz@stud.uni-hannover.de>
 *      Jan Kiszka <kiszka@rts.uni-hannover.de>
 *
 */
#ifndef __TIMS_DRIVER_H__
#define __TIMS_DRIVER_H__

#include <linux/bitops.h>
#include <linux/list.h>
#include <linux/mm.h>

#include <rtdm/rtdm_driver.h>
#include <main/tims/tims.h>
#include <main/tims/router/tims_router.h>

//
// init flags
//
#define TIMS_INIT_BIT_RECV_TASK             0
#define TIMS_INIT_BIT_PIPE_TO_CLIENT        1
#define TIMS_INIT_BIT_PIPE_FROM_CLIENT      2
#define TIMS_INIT_BIT_REGISTERED            3
#define TIMS_INIT_BIT_MBX_CACHE             4
#define TIMS_INIT_BIT_RTNET                 5
#define TIMS_INIT_BIT_SYNC_DEV              6
#define TIMS_INIT_BIT_SYNC_TASK             7

//
// map info
//
typedef struct
{
    unsigned long virtual;
    unsigned char mapped;
} tims_map_info;

//
// mailbox slot state
//
typedef struct
{
    int free;
    int write;
    int peek;
    int read;
} tims_slot_state;

//
// mailbox slot
//
typedef struct
{
    tims_msg_head*          p_head;         // message pointers
    unsigned long           p_head_map;     // mapped virtual kernel address
    struct list_head        mbx_list;       // to hold it in free|read|writelist

    int                     map_idx;        // start index of page table
    struct tims_mbx*        p_mbx;          // pointer to mailbox
} tims_mbx_slot;

//
// mailbox
//
typedef struct tims_mbx
{
    tims_mbx_slot*          slot;           // mailbox slots
    rtdm_sem_t              readSem;        // semaphore for the mbx reader
                                            // (waiting for the next message)

    rtdm_lock_t             list_lock;      // lock for mbx_lists
    struct list_head        free_list;      // list of all free mailbox slots
    struct list_head        write_list;     // list of all mailbox write slots
    struct list_head        read_list;      // list of all mailbox read slots
    tims_mbx_slot*          p_peek;         // save peek slot

    int64_t                 timeout_ns;     // timeout for peek/receive
    unsigned long           flags;          // mailbox state
    unsigned int            address;        // address of this mailbox

    // config values
    unsigned int            slot_count;     // 0  => FIFO Queueing
                                            // > 0 => Priority Queueing
    tims_slot_state         slot_state;     // tims slot state
    unsigned int            msg_size;       // bytes per message
    void*                   buffer;         // NULL: kernel-located
    unsigned long           buffer_pages;   // number of buffer pages
    struct page**           pp_pages;       // pointer to page list
    tims_map_info*          p_mapInfo;      // virtual addresses of page list
    size_t                  buffer_size;
} tims_mbx;

//
// mailbox flags
//
#define TIMS_MBX_BIT_EXTERNBUFFER           0
#define TIMS_MBX_BIT_SLOT                   1
#define TIMS_MBX_BIT_FIFO                   2
#define TIMS_MBX_BIT_READERWAIT             3
#define TIMS_MBX_BIT_USRSPCBUFFER           4

//
// tims context
//
typedef struct
{
    struct list_head        ctx_list;       // list of all registered contextes
    int                     protocol;       // tims protocol
    tims_sockaddr           sock;           // socket address
    tims_mbx*               p_mbx;          // mailbox
    int                     use_counter;    // context use counter
    unsigned long           flags;          // context flags
} tims_ctx;

//
// common variables
//
extern unsigned long init_flags;

//
// helper functions
//
static inline int get_max_pages(unsigned long buffersize)
{
    return (((buffersize + PAGE_SIZE - 1) & PAGE_MASK)
            >> PAGE_SHIFT) + 1;
}

#endif // __TIMS_DRIVER_H__
