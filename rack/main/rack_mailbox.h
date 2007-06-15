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

#include <string.h>
#include <main/tims/tims.h>
#include <main/defines/rack_bitops.h> // init bits

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

/** used mailbox */
    RackMailbox*    usedMbx;

} __attribute__((packed)) message_info;

/** Clear a message info*/
static inline void clearMsgInfo(message_info* msgInfo)
{
    memset(msgInfo, 0, sizeof(message_info));
}

//######################################################################
//# RackMailbox
//######################################################################

class RackMailbox {

    private:
        RackBits        mbxBits;
        int             fildes;
        uint32_t        adr;

        tims_msg_head*  p_peek_head;

        int8_t          send_prio;
        uint32_t        max_data_len;

        int             mbxOK(void);
        void            fillMessageRecvInfo(message_info *msgInfo, void *p_data);
        void            fillMessagePeekInfo(message_info *msgInfo);

    public:
        RackMailbox();

/** Get length of message overhead */
        static uint32_t getMsgOverhead(void)  { return TIMS_HEADLEN; }

/** Get address of the mailbox */
        uint32_t        getAdr(void)          { return adr; }

/** Get maximum data length supported by the mailbox */
        uint32_t        maxDataLen(void)      { return max_data_len; }

/** Get mailbox priority */
        int8_t          getPriority(void)     { return send_prio; }

/** Get mailbox file descriptor */
        int             getFildes(void)       { return fildes; }

/** Set byteorder of received mailbox data */
        int             setDataByteorder(message_info *msgInfo);


//
// create, destroy and clean
//

        int     create(uint32_t address, int messageSlots,
                       ssize_t messageDataSize, void *buffer,
                       ssize_t buffer_size, int8_t sendPriority);

        int     remove(void);

        int     clean(void);

//
// send
//

        int     sendMsg(int8_t type, uint32_t dest, uint8_t seq_nr);

        int     sendMsgReply(int8_t type, message_info *msgInfo);

        int     sendDataMsg(tims_msg_head *p_head, int dataPointers, void* data1,
                            uint32_t datalen1, ...);

        int     sendDataMsg(int8_t type, uint32_t dest, uint8_t seq_nr,
                            int dataPointers, void *data1,
                            uint32_t datalen1, ...);

        int     sendDataMsgReply(int8_t type, message_info *msgInfo,
                                 int dataPointers,
                                 void* data1, uint32_t datalen1, ...);

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

        int     recvDataMsgTimed(uint64_t timeout_ns, void *p_data,
                                 uint32_t maxdatalen, message_info *msgInfo);

        int     recvDataMsg(void *p_data, uint32_t maxdatalen,
                            message_info *msgInfo);

        int     recvDataMsgIf(void *p_data, uint32_t maxdatalen,
                              message_info *msgInfo);
};

/*@}*/

#endif // __RACK_MAILBOX_H__
