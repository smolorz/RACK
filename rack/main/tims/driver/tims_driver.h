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
#ifndef __TIMS_DRIVER_H__
#define __TIMS_DRIVER_H__

#include <linux/bitops.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <rtdm/rtdm_driver.h>
#include <main/tims/tims.h>
#include <main/tims/router/tims_msg_router.h>

//
// map info
//
typedef struct tims_map_info
{
    unsigned long virtual;
    unsigned char mapped;
} timsMapInfo_t;

//
// mailbox slot state
//
typedef struct slot_state
{
    int free;
    int write;
    int peek;
    int read;
} slot_state_t;

//
// mailbox slot
//
typedef struct tims_mbx_slot
{
    timsMsgHead*            p_head;         // message pointers
    unsigned long           p_head_map;     // mapped virtual kernel address
    struct list_head        mbx_list;       // to hold it in free|read|writelist

    int                     map_idx;        // start index of page table
    struct tims_mailbox*    p_mbx;          // pointer to mailbox
} timsMbxSlot;

//
// mailbox
//
typedef struct tims_mailbox
{
    timsMbxSlot*            slot;           // mailbox slots
    rtdm_sem_t              readSem;        // semaphore for the mbx reader
                                            // (waiting for the next message)

    rtdm_lock_t             list_lock;      // lock for mbx_lists
    struct list_head        free_list;      // list of all free mailbox slots
    struct list_head        write_list;     // list of all mailbox write slots
    struct list_head        read_list;      // list of all mailbox read slots
    timsMbxSlot*            p_peek;         // save peek slot

    int64_t                 timeout_ns;     // timeout for peek/receive
    unsigned long           flags;          // mailbox state
    unsigned int            address;        // address of this mailbox

    // config values
    unsigned int            slot_count;     // 0  => FIFO Queueing
                                            // > 0 => Priority Queueing
    slot_state_t            slot_state;     // tims slot state
    unsigned int            msg_size;       // bytes per message
    void*                   buffer;         // NULL: kernel-located
    unsigned long           buffer_pages;   // number of buffer pages
    struct page**           pp_pages;       // pointer to page list
    timsMapInfo_t*          p_mapInfo;      // virtual addresses of page list
    size_t                  buffer_size;
} timsMbx;

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
typedef struct tims_context
{
    struct list_head        ctx_list;       // list of all registered contextes
    int                     protocol;       // tims protocol
    struct tims_sockaddr    sock;           // socket address
    timsMbx*                p_mbx;          // mailbox
    int                     use_counter;    // context use counter
    unsigned long           flags;          // context flags
} timsCtx;

//
// help functions
//
static inline int get_max_pages(unsigned long buffersize)
{
    return (((buffersize + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
            >> PAGE_SHIFT) + 1;
}

#endif // __TIMS_DRIVER_H__
