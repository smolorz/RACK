/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2009 University of Hannover
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
 *      Marko Reimer <reimer@rts.uni-hannover.de>
 *      Jan Kiszka <kiszka@rts.uni-hannover.de>
 *      Oliver Wulf <oliver.wulf@web.de>
 *      Sebastian Smolorz <smolorz@rts.uni-hannover.de>
 */
#ifndef __TIMS_H__
#define __TIMS_H__

#include <main/tims/tims_types.h>
#include <main/tims/tims_byteorder.h>

//
//  TIMS message types
//

#define TIMS_MSG_OK                             0
#define TIMS_MSG_ERROR                         -1
#define TIMS_MSG_TIMEOUT                       -2
#define TIMS_MSG_NOT_AVAILABLE                 -3

//
// TIMS message head
//

typedef struct
{
    uint8_t     flags;     // 1 Byte: flags
    int8_t      type;      // 1 Byte: Message Type
    uint8_t     priority;  // 1 Byte: Priority
    uint8_t     seq_nr;    // 1 Byte: Sequence Number
    uint32_t    dest;      // 4 Byte: Destination ID
    uint32_t    src;       // 4 Byte: Source ID
    uint32_t    msglen;    // 4 Byte: length of complete message
    uint8_t     data[0];   // 0 Byte: following data
                           //---> 16 Byte
} __attribute__((packed)) tims_msg_head;

#define TIMS_HEADLEN        sizeof(tims_msg_head)

/* TiMS flags (first byte of the head) */
#define TIMS_HEAD_BYTEORDER_LE  0x01
#define TIMS_BODY_BYTEORDER_LE  0x02

//
// common static inline functions
//

#ifdef __cplusplus
extern "C" {
#endif

static inline void tims_set_head_byteorder(tims_msg_head* p)
{
  #if defined __BIG_ENDIAN_BITFIELD
      p->flags &= ~TIMS_HEAD_BYTEORDER_LE;
  #elif defined __LITTLE_ENDIAN_BITFIELD
      p->flags |= TIMS_HEAD_BYTEORDER_LE;
  #else
      #error There is no byteorder defined. Fix <main/tims/tims_byteorder.h>.
  #endif
}

static inline void tims_set_body_byteorder(tims_msg_head* p)
{
    #if defined __BIG_ENDIAN_BITFIELD
        p->flags &= ~TIMS_BODY_BYTEORDER_LE;
    #elif defined __LITTLE_ENDIAN_BITFIELD
        p->flags |= TIMS_BODY_BYTEORDER_LE;
    #else
        #error There is no byteorder defined. Fix <main/tims/tims_byteorder.h>.
    #endif
}

static inline void tims_set_byteorder(tims_msg_head* p)
{
    tims_set_head_byteorder(p);
    tims_set_body_byteorder(p);
}

static inline void tims_parse_head_byteorder(tims_msg_head* p)
{
    if (p->flags & TIMS_HEAD_BYTEORDER_LE)  /* head is little endian */
    {
        p->src     = __le32_to_cpu(p->src);
        p->dest    = __le32_to_cpu(p->dest);
        p->msglen  = __le32_to_cpu(p->msglen);
    }
    else                                    /* head is big endian */
    {
        p->src     = __be32_to_cpu(p->src);
        p->dest    = __be32_to_cpu(p->dest);
        p->msglen  = __be32_to_cpu(p->msglen);
    }
    tims_set_head_byteorder(p);
}

static inline void tims_fill_head(tims_msg_head *p_head, int8_t type,
                                  uint32_t dest, uint32_t src,
                                  uint8_t priority, uint8_t seq_nr, uint8_t flags,
                                  uint32_t msglen)
{
    p_head->flags    = flags;
    p_head->type     = type;
    p_head->priority = priority;
    p_head->seq_nr   = seq_nr;
    p_head->dest     = dest;
    p_head->src      = src;
    p_head->msglen   = msglen;

    tims_set_byteorder(p_head);
}

#ifdef __cplusplus
}
#endif

#endif // __TIMS_H__
