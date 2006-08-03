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
#ifndef __TIMS_ENDIAN_H__
#define __TIMS_ENDIAN_H__

//
// endian checks
//

#define MESSAGE_FLAG_HEAD_ORDER_LE  0x01
#define MESSAGE_FLAG_BODY_ORDER_LE  0x02

#ifdef __cplusplus
extern "C" {
#endif

static inline int tims_cpu_is_le(void)
{
  #if defined __BIG_ENDIAN_BITFIELD
      return 0;
  #elif defined __LITTLE_ENDIAN_BITFIELD
      return 1;
  #else
      #error There is no byte-order defined. Fix <asm/byteorder.h>.
  #endif
}

static inline void tims_set_head_byteorder(tims_msg_head* p)
{
  #if defined __BIG_ENDIAN_BITFIELD
      p->flags &= ~MESSAGE_FLAG_HEAD_ORDER_LE;
  #elif defined __LITTLE_ENDIAN_BITFIELD
      p->flags |= MESSAGE_FLAG_HEAD_ORDER_LE;
  #else
      #error There is no byte-order defined. Fix <asm/byteorder.h>.
  #endif
}

static inline void tims_set_body_byteorder(tims_msg_head* p)
{
  #if defined __BIG_ENDIAN_BITFIELD
      p->flags &= ~MESSAGE_FLAG_BODY_ORDER_LE;
  #elif defined __LITTLE_ENDIAN_BITFIELD
      p->flags |= MESSAGE_FLAG_BODY_ORDER_LE;
  #else
      #error There is no byte-order defined. Fix <asm/byteorder.h>.
  #endif
}

static inline void tims_set_byteorder(tims_msg_head* p)
{
  if (!p) {
    return;
  }

  tims_set_head_byteorder(p);

  if (p->msglen > TIMS_HEADLEN) {
    tims_set_body_byteorder(p);
  }
}

static inline void tims_parse_head_byteorder(tims_msg_head* p)
{
  if (p->flags & MESSAGE_FLAG_HEAD_ORDER_LE) { /* head is little endian */
    p->src     = __le32_to_cpu(p->src);
    p->dest    = __le32_to_cpu(p->dest);
    p->msglen  = __le32_to_cpu(p->msglen);
  } else {                                     /* head is big endian */
    p->src     = __be32_to_cpu(p->src);
    p->dest    = __be32_to_cpu(p->dest);
    p->msglen  = __be32_to_cpu(p->msglen);
  }

  tims_set_head_byteorder(p);
}

#ifdef __cplusplus
}
#endif

#endif // __TIMS_ENDIAN_H__
