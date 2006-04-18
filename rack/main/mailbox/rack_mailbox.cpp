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
#include <stdarg.h>
#include <string.h> // memset

#include <main/rack_mailbox.h>

// init bits
#define     INIT_BIT_TIMS_MBX_CREATED       0

//
// internal functions
//

void         RackMailbox::fillMessageRecvInfo(MessageInfo *msgInfo,
                                              void *p_data)
{
  msgInfo->p_data         = p_data;
  msgInfo->datalen        -= TIMS_HEADLEN;
  msgInfo->usedMbx        = this;
}

void         RackMailbox::fillMessagePeekInfo(MessageInfo *msgInfo)
{
  msgInfo->flags          = p_peek_head->flags;
  msgInfo->type           = p_peek_head->type;
  msgInfo->priority       = p_peek_head->priority;
  msgInfo->seq_nr         = p_peek_head->seq_nr;
  msgInfo->dest           = p_peek_head->dest;
  msgInfo->src            = p_peek_head->src;
  msgInfo->datalen        = p_peek_head->msglen - TIMS_HEADLEN;
  msgInfo->p_data         = &p_peek_head->data;
  msgInfo->usedMbx        = this;
}

int          RackMailbox::setDataByteorder(MessageInfo *msgInfo)
{
  if (!msgInfo || !msgInfo->p_data) {
    // no data
    return 0;
  }

  // check peek pointer (change flags in mailbox slot, tims does it)
  if (p_peek_head &&
      p_peek_head->data == msgInfo->p_data) {
    tims_set_body_byteorder(p_peek_head);
    return 0;
  }

  // message received (change flags in message info struct)
  if (tims_cpu_is_le()) {
    msgInfo->flags |= MSGINFO_DATA_LE;
  } else {
    msgInfo->flags &= ~MSGINFO_DATA_LE;
  }

  return 0;
}

int          RackMailbox::mbxOK(void)
{
    return mbxBits.testBit(INIT_BIT_TIMS_MBX_CREATED);
}

//
// create, destroy and clean
//

RackMailbox::RackMailbox()
{
    fildes      = -1;
    adr         = 0;
    send_prio   = 0;
    p_peek_head = NULL;
    mbxBits.clearAllBits();
}

int     RackMailbox::create(uint32_t address, int messageSlots,
                            ssize_t messageDataSize, void *buffer,
                            ssize_t buffer_size, int8_t sendPriority)
{
    int fd = 0;
    ssize_t msglen = messageDataSize + TIMS_HEADLEN;
    fd = tims_mbx_create(address, messageSlots, msglen,
                        buffer, buffer_size);
    if (fd < 0)
    {
        return fd;
    }

    fildes        = fd;
    adr           = address;
    send_prio     = sendPriority;
    max_data_len  = messageDataSize;

    mbxBits.setBit(INIT_BIT_TIMS_MBX_CREATED);
    return 0;
}

int     RackMailbox::remove(void)
{
    int ret;

    if (mbxBits.testAndClearBit(INIT_BIT_TIMS_MBX_CREATED))
    {
        ret = tims_mbx_remove(fildes);
        if (ret)
        {
            return ret;
        }
        fildes = -1;
        adr    = 0;

        return 0;
    }
    return -ENODEV;
}

int     RackMailbox::clean(void)
{
    if (!mbxOK())
        return -ENODEV;

    return tims_mbx_clean(fildes);
}

//
// send
//

int     RackMailbox::sendMsg(int8_t type, uint32_t dest, uint8_t seq_nr)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    ret = (int)tims_sendmsg_0(fildes, type, dest, adr, send_prio, seq_nr, 0, 0);
    if (ret != TIMS_HEADLEN)
    {
        return ret;
    }
    return 0;
}

int     RackMailbox::sendMsgReply(int8_t type, MessageInfo* msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    if (!msgInfo)
        return -EINVAL;

    ret = (int)tims_sendmsg_0(fildes, type, msgInfo->src, msgInfo->dest,
                         msgInfo->priority, msgInfo->seq_nr, 0, 0);
    if (ret != TIMS_HEADLEN)
    {
        return ret;
    }
    return 0;
}

int     RackMailbox::sendDataMsg(int8_t type, uint32_t dest,
                                 uint8_t seq_nr, int dataPointers,
                                 void* data1, uint32_t datalen1, ...)
{
    int i = 1;
    uint32_t msglen;
    int32_t  ret;
    timsMsgHead head;
    struct iovec iov[dataPointers];

    if (!mbxOK())
        return -ENODEV;

    iov[0].iov_base = data1;
    iov[0].iov_len  = datalen1;
    msglen = datalen1 + TIMS_HEADLEN;

    va_list ap;
    va_start(ap, datalen1);

    while ( i < dataPointers)
    {
        iov[i].iov_base = va_arg(ap, void*);
        iov[i].iov_len  = va_arg(ap, unsigned int);
        msglen += iov[i].iov_len;
        i++;
    }
    va_end(ap);

    tims_fillhead(&head, type, dest, adr, send_prio, seq_nr, 0, msglen);

    ret = tims_sendmsg(fildes, &head, iov, dataPointers, 0);
    if (ret < 0)    // error
        return ret;

    if ((uint32_t)ret != msglen)
       return -EFAULT;

    return 0;
}

int     RackMailbox::sendDataMsg(timsMsgHead *p_head, int dataPointers,
                                 void* data1, uint32_t datalen1, ...)
{
    int i = 1;
    uint32_t msglen;
    int32_t  ret;
    struct iovec iov[dataPointers];

    if (!mbxOK())
        return -ENODEV;

    iov[0].iov_base = data1;
    iov[0].iov_len  = datalen1;
    msglen = datalen1 + TIMS_HEADLEN;

    va_list ap;
    va_start(ap, datalen1);

    while ( i < dataPointers)
    {
        iov[i].iov_base = va_arg(ap, void*);
        iov[i].iov_len  = va_arg(ap, unsigned int);
        msglen += iov[i].iov_len;
        i++;
    }
    va_end(ap);

    p_head->msglen = msglen;

    ret = tims_sendmsg(fildes, p_head, iov, dataPointers, 0);
    if (ret < 0)    // error
        return ret;

    if ((uint32_t)ret != msglen)
       return -EFAULT;

    return 0;
}

int     RackMailbox::sendDataMsgReply(int8_t type, MessageInfo* msgInfo,
                                      int dataPointers,
                                      void* data1, uint32_t datalen1, ...)
{
    int         i = 1;
    uint32_t    msglen;
    int32_t     ret;
    timsMsgHead head;
    struct iovec iov[dataPointers];

    if (!mbxOK())
        return -ENODEV;

    if (!msgInfo)
        return -EINVAL;

    iov[0].iov_base = data1;
    iov[0].iov_len  = datalen1;
    msglen = datalen1 + TIMS_HEADLEN;

    va_list ap;
    va_start(ap, datalen1);

    while (i < dataPointers)
    {
        iov[i].iov_base = va_arg(ap, void*);
        iov[i].iov_len  = va_arg(ap, unsigned int);
        msglen += iov[i].iov_len;
        i++;
    }
    va_end(ap);

    tims_fillhead(&head, type, msgInfo->src, msgInfo->dest, msgInfo->priority,
                  msgInfo->seq_nr, 0, msglen);

    ret = tims_sendmsg(fildes, &head, iov, dataPointers, 0);
    if (ret < 0)    // error
        return ret;

    if ((uint32_t)ret != msglen)
       return -EFAULT;

    return 0;
}

//
// peek
//

int     RackMailbox::peekEnd(void)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    if (!p_peek_head)
        return -EFAULT;

    ret = tims_peek_end(fildes);
    if (ret)
    {
        return ret;
    }
    p_peek_head = NULL;
    return 0;
}

int     RackMailbox::peekTimed(uint64_t timeout_ns, MessageInfo *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    if (p_peek_head)
        return -EINVAL;

    ret = tims_peek_timed(fildes, &p_peek_head, timeout_ns);
    if (ret)
    {
        p_peek_head = NULL;
        return ret;
    }
    fillMessagePeekInfo(msgInfo);
    return 0;
}

int     RackMailbox::peek(MessageInfo *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    if (p_peek_head)
        return -EBUSY;

    ret = tims_peek(fildes, &p_peek_head);
    if (ret)
    {
        p_peek_head = NULL;
        return ret;
    }
    fillMessagePeekInfo(msgInfo);
    return 0;
}

int     RackMailbox::peekIf(MessageInfo *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    if (p_peek_head)
        return -EBUSY;

    ret = tims_peek_if(fildes, &p_peek_head);
    if (ret)
    {
        p_peek_head = NULL;
        return ret;
    }
    fillMessagePeekInfo(msgInfo);
    return 0;
}


//
// receive
//

int     RackMailbox::recvMsgTimed(uint64_t timeout_ns, MessageInfo *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    ret = tims_recvmsg_timed(fildes, (timsMsgHead *)msgInfo, NULL, 0, timeout_ns, 0);
    if (ret)
    {
        return ret;
    }
    fillMessageRecvInfo(msgInfo, NULL);
    return 0;
}

int     RackMailbox::recvMsg(MessageInfo *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    ret = tims_recvmsg(fildes, (timsMsgHead *)msgInfo, NULL, 0, 0);
    if (ret)
    {
        return ret;
    }

    fillMessageRecvInfo(msgInfo, NULL);
    return 0;
}

int     RackMailbox::recvMsgIf(MessageInfo *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    ret = tims_recvmsg_if(fildes, (timsMsgHead *)msgInfo, NULL, 0, 0);
    if (ret)
    {
        return ret;
    }

    fillMessageRecvInfo(msgInfo, NULL);
    return 0;
}


int     RackMailbox::recvDataMsgTimed(uint64_t timeout_ns, void *p_data,
                                      uint32_t maxdatalen, MessageInfo *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    ret = tims_recvmsg_timed(fildes, (timsMsgHead *)msgInfo, p_data, maxdatalen,
                             timeout_ns, 0);
    if (ret)
    {
        return ret;
    }
    fillMessageRecvInfo(msgInfo, p_data);
    return 0;
}

int     RackMailbox::recvDataMsg(void *p_data, uint32_t maxdatalen,
                                 MessageInfo *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    ret = tims_recvmsg(fildes, (timsMsgHead *)msgInfo, p_data, maxdatalen, 0);
    if (ret)
    {
        return ret;
    }
    fillMessageRecvInfo(msgInfo, p_data);
    return 0;
}

int     RackMailbox::recvDataMsgIf(void *p_data, uint32_t maxdatalen,
                                   MessageInfo *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    ret = tims_recvmsg_if(fildes, (timsMsgHead *)msgInfo, p_data, maxdatalen, 0);
    if (ret) {
        return ret;
    }
    fillMessageRecvInfo(msgInfo, p_data);
    return 0;
}

