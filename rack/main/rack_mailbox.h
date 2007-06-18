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

/*!
 * @ingroup rackservices
 * @defgroup mailbox Rack Mailbox
 *
 * This is the mailbox interface of RACK provided to application programs
 * in userspace.
 *
 *@{*/

#include <main/tims/tims.h>

//######################################################################
//# message_info
//######################################################################

class RackMailbox;

/*  TiMS message head
    uint8_t       flags;     // 1 Byte: flags
    int8_t        type;      // 1 Byte: Message Type
    uint8_t       priority;  // 1 Byte: Priority
    uint8_t       seq_nr;    // 1 Byte: Sequence Number
    uint32_t      dest;      // 4 Byte: Destination ID
    uint32_t      src;       // 4 Byte: Source ID
    uint32_t      msglen;    // 4 Byte: length of complete message
    uint8_t       data[0];   // 0 Byte: following data
*/

/** Message information */
typedef struct {
    /** data flags */
    uint8_t         flags;

    /** message type */
    int8_t          type;

    /** message priority */
    uint8_t         priority;

    /** sequence number */
    uint8_t         seq_nr;

    /** destination address */
    uint32_t        dest;

    /** source address */
    uint32_t        src;

    /** data length (bytes)*/
    uint32_t        datalen;

    /** data pointer */
    void*           p_data;

} __attribute__((packed)) message_info;

/** Clear message info*/
static inline void clearMsgInfo(message_info* msgInfo)
{
    msgInfo->flags    = 0;
    msgInfo->type     = -1;
    msgInfo->priority = 0;
    msgInfo->seq_nr   = 0;
    msgInfo->dest     = 0;
    msgInfo->src      = 0;
    msgInfo->datalen  = 0;
    msgInfo->p_data   = NULL;
}

/** Check if data byteorder is little endian */
static inline int isDataByteorderLe(message_info *msgInfo)
{
    return (msgInfo->flags & TIMS_BODY_BYTEORDER_LE);
}

/** Set byteorder of received mailbox data */
static inline void setDataByteorder(message_info *msgInfo)
{
    tims_set_body_byteorder((tims_msg_head*)msgInfo);
}

//######################################################################
//# RackMailbox
//######################################################################

class RackMailbox {

    private:
        int             fd;
        uint32_t        adr;
        uint8_t         sendPrio;

        void            fillMessageRecvInfo(message_info *msgInfo, void *p_data);
        void            fillMessagePeekInfo(message_info *msgInfo, tims_msg_head* p_peek_head);

    public:
        RackMailbox();

        /** Get length of message overhead */
        static uint32_t getMsgOverhead(void)  { return TIMS_HEADLEN; }

        /** Get address of the mailbox */
        uint32_t        getAdr(void)          { return adr; }

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

        int     sendMsgReply(int8_t type, message_info *msgInfo);

        int     sendDataMsg(tims_msg_head *p_head, int dataPointers, void* data1, uint32_t datalen1, ...);

        int     sendDataMsg(int8_t type, uint32_t dest, uint8_t seqNr,
                            int dataPointers, void *data1, uint32_t datalen1, ...);

        int     sendDataMsgReply(int8_t type, message_info *msgInfo,
                                 int dataPointers, void* data1, uint32_t datalen1, ...);

        //
        // peek
        //

        int     peekEnd(void);

        int     peekTimed(uint64_t timeout_ns, message_info *msgInfo);

        int     peek(message_info *msgInfo);

        int     peekIf(message_info *msgInfo);

        //
        // receive
        //

        int     recvMsgTimed(uint64_t timeout_ns, message_info *msgInfo);

        int     recvMsg(message_info *msgInfo);

        int     recvMsgIf(message_info *msgInfo);

        int     recvDataMsgTimed(uint64_t timeout_ns, void *p_data, uint32_t maxDatalen, message_info *msgInfo);

        int     recvDataMsg(void *p_data, uint32_t maxDatalen, message_info *msgInfo);

        int     recvDataMsgIf(void *p_data, uint32_t maxDatalen, message_info *msgInfo);
};

/*@}*/

#endif // __RACK_MAILBOX_H__
