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
 *      Oliver Wulf <oliver.wulf@web.de>
 *
 */
#ifndef __RACK_MAILBOX_H__
#define __RACK_MAILBOX_H__

#include <main/tims/tims.h>
#include <main/tims/tims_api.h>
#include <main/rack_mutex.h>

/**
 * This is the mailbox interface of RACK provided to application programs
 * in userspace.
 *
 * @ingroup main_common
 */
class RackMessage
{
protected:
    tims_msg_head   head;
public:
    uint32_t        datalen;
    void*           p_data;

public:
    RackMessage()
    {
        clear();
    }

    ~RackMessage()
    {}

    void clear(void)
    {
        head.flags    = 0;
        head.type     = -1;
        head.priority = 0;
        head.seq_nr   = 0;
        head.dest     = 0;
        head.src      = 0;

        datalen  = 0;
        p_data   = NULL;
    }

    tims_msg_head* getHead(void)
    {
        return &head;
    }

    uint8_t getFlags(void)
    {
        return head.flags;
    }

    int8_t getType(void)
    {
        return head.type;
    }

    uint8_t getPriority(void)
    {
        return head.priority;
    }

    uint8_t getSeqNr(void)
    {
        return head.seq_nr;
    }

    uint32_t getSrc(void)
    {
        return head.src;
    }

    uint32_t getDest(void)
    {
        return head.dest;
    }

    /** Set byteorder of received mailbox data */
    void setDataByteorder(void)
    {
        tims_set_body_byteorder(&head);
    }

    int isDataByteorderLe(void)
    {
        return (head.flags & TIMS_BODY_BYTEORDER_LE);
    }

    int64_t data64ToCpu(int64_t x)
    {
        if(head.flags & TIMS_BODY_BYTEORDER_LE)
        {
            return __le64_to_cpu(x);
        }
        else
        {
            return __be64_to_cpu(x);
        }
    }

    int32_t data32ToCpu(int32_t x)
    {
        if(head.flags & TIMS_BODY_BYTEORDER_LE)
        {
            return __le32_to_cpu(x);
        }
        else
        {
            return __be32_to_cpu(x);
        }
    }

    int16_t data16ToCpu(int16_t x)
    {
        if(head.flags & TIMS_BODY_BYTEORDER_LE)
        {
            return __le16_to_cpu(x);
        }
        else
        {
            return __be16_to_cpu(x);
        }
    }

    double data64FloatToCpu(double x)
    {
        if(head.flags & TIMS_BODY_BYTEORDER_LE)
        {
            return __le64_float_to_cpu(x);
        }
        else
        {
            return __be64_float_to_cpu(x);
        }
    }

    float data32FloatToCpu(float x)
    {
        if(head.flags & TIMS_BODY_BYTEORDER_LE)
        {
            return __le32_float_to_cpu(x);
        }
        else
        {
            return __be32_float_to_cpu(x);
        }
    }
};

/**
 *
 * @ingroup main_common
 */
class RackDataMessage : public RackMessage
{
public:
    RackDataMessage()
    {}

    ~RackDataMessage()
    {}
};

/**
 *
 * @ingroup main_common
 */
class RackMailbox
{
    private:
        int             fd;
        uint32_t        addr;
        uint8_t         sendPrio;

        RackMutex       sendMtx;
        RackMutex       recvMtx;

    public:
        RackMailbox();

        /** Get length of message overhead */
        static uint32_t getMsgOverhead(void)  { return TIMS_HEADLEN; }

        /** Get address of the mailbox */
        uint32_t        getAdr(void)          { return addr; }

        /** Get mailbox priority */
        uint8_t         getPriority(void)     { return sendPrio; }

        /** Get mailbox file descriptor */
        int             getFd(void)           { return fd; }

        //
        // create, destroy and clean
        //

        int     create(uint32_t address, int messageSlots,
                       ssize_t maxDatalen, void *buffer,
                       ssize_t bufferSize, int8_t sendPriority);

        int     remove(void);

        int     clean(void);

        //
        // send
        //

        int     sendMsg(int8_t type, uint32_t dest, uint8_t seqNr);

        int     sendMsgReply(int8_t type, RackMessage *msgInfo);

        int     sendDataMsg(tims_msg_head *p_head, int dataPointers, void* data1, uint32_t datalen1, ...);

        int     sendDataMsg(int8_t type, uint32_t dest, uint8_t seqNr, int dataPointers, void *data1, uint32_t datalen1, ...);

        int     sendDataMsgReply(int8_t type, RackMessage *msgInfo, int dataPointers, void* data1, uint32_t datalen1, ...);

        //
        // peek
        //

        int     peekEnd(void);

        int     peekTimed(uint64_t timeout_ns, RackMessage *msgInfo);

        int     peek(RackMessage *msgInfo);

        int     peekIf(RackMessage *msgInfo);

        //
        // receive
        //

        int     recvMsgTimed(uint64_t timeout_ns, RackMessage *msgInfo);

        int     recvMsg(RackMessage *msgInfo);

        int     recvMsgIf(RackMessage *msgInfo);

        int     recvDataMsgTimed(uint64_t timeout_ns, void *p_data, uint32_t maxDatalen, RackMessage *msgInfo);

        int     recvDataMsg(void *p_data, uint32_t maxDatalen, RackMessage *msgInfo);

        int     recvDataMsgIf(void *p_data, uint32_t maxDatalen, RackMessage *msgInfo);
};

#endif // __RACK_MAILBOX_H__
