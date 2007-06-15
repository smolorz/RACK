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

#include <stdarg.h>
#include <string.h> // memset

#include <main/rack_mailbox.h>

// init bits
#define     INIT_BIT_TIMS_MBX_CREATED       0

//
// internal functions
//

void RackMailbox::fillMessageRecvInfo(message_info *msgInfo, void *p_data)
{
    msgInfo->p_data    = p_data;
    msgInfo->datalen  -= TIMS_HEADLEN;
    msgInfo->usedMbx   = this;
}

void RackMailbox::fillMessagePeekInfo(message_info *msgInfo)
{
    msgInfo->flags    = p_peek_head->flags;
    msgInfo->type     = p_peek_head->type;
    msgInfo->priority = p_peek_head->priority;
    msgInfo->seq_nr   = p_peek_head->seq_nr;
    msgInfo->dest     = p_peek_head->dest;
    msgInfo->src      = p_peek_head->src;
    msgInfo->datalen  = p_peek_head->msglen - TIMS_HEADLEN;
    msgInfo->p_data   = &p_peek_head->data;
    msgInfo->usedMbx  = this;
}

int RackMailbox::mbxOK(void)
{
    return mbxBits.testBit(INIT_BIT_TIMS_MBX_CREATED);
}

 /*!
 * @ingroup mailbox
 *
 *@{*/

int RackMailbox::setDataByteorder(message_info *msgInfo)
{
    tims_set_body_byteorder((tims_msg_head*)msgInfo);

    return 0;
}

//
// create, destroy and clean
//

/**
 * @brief Mailbox constructor
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT)
 *
 * Rescheduling: never.
 */
RackMailbox::RackMailbox()
{
    fildes      = -1;
    adr         = 0;
    send_prio   = 0;
    p_peek_head = NULL;
    mbxBits.clearAllBits();
}

/**
 * @brief Create a mailbox
 *
 * This function creates a mailbox to send and receive messages.
 * Using this function a unique mailbox address have to be given.
 * The class Module privides an additional function createMailbox() to
 * set this mailbox address automatically.
 *
 * @param address Mailbox address of the mailbox.
 * @param messageSlots Number of message slots. If this value is 0 a FIFO
 *                     mailbox is created (but not supported yet).
 * @param messageDataSize Number of maximum data bytes in one slot.
 * @param buffer Pointer to an own created mailbox buffer. If the pointer
 *               is NULL Rack creates the needed buffer with the given
 *               @a buffer_size.
 * @param buffer_size Size of the mailbox buffer (in bytes). This value is
 *                    only needed if the buffer pointer is not NULL.
 * @param sendPriority Message priority of all sent messages.
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (switches to primary mode)
 *
 * Rescheduling: possible.
 */
int RackMailbox::create(uint32_t address, int messageSlots,
                        ssize_t messageDataSize, void *buffer,
                        ssize_t buffer_size, int8_t sendPriority)
{
    int fd = 0;
    ssize_t msglen = messageDataSize + TIMS_HEADLEN;
    fd = tims_mbx_create(address, messageSlots, msglen,
                         buffer, buffer_size);
    if (fd < 0)
        return fd;

    fildes        = fd;
    adr           = address;
    send_prio     = sendPriority;
    max_data_len  = messageDataSize;

    mbxBits.setBit(INIT_BIT_TIMS_MBX_CREATED);
    return 0;
}

/**
 * @brief Delete a mailbox
 *
 * RackMailbox::remove(void) deletes this mailbox which was created
 * with RackMailbox::create().
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task
 *
 * Rescheduling: never
 */
int RackMailbox::remove(void)
{
    int ret;

    if (mbxBits.testAndClearBit(INIT_BIT_TIMS_MBX_CREATED))
    {
        ret = tims_mbx_remove(fildes);
        if (ret)
            return ret;

        fildes = -1;
        adr    = 0;

        return 0;
    }
    return -ENODEV;
}

/**
 * @brief Clean a mailbox
 *
 * This function removes all messages in this mailbox.
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task
 *
 * Rescheduling: never
 */
int RackMailbox::clean(void)
{
    if (!mbxOK())
        return -ENODEV;

    return tims_mbx_clean(fildes);
}

//
// send
//

/**
 * @brief Send a message (without data)
 *
 * This function sends a message without any data.
 *
 * @param type Message type
 * @param dest Destination mailbox address
 * @param seq_nr Sequence number
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: possible
 */
int RackMailbox::sendMsg(int8_t type, uint32_t dest, uint8_t seq_nr)
{
    tims_msg_head head;
    int ret;

    if (!mbxOK())
        return -ENODEV;

    tims_fill_head(&head, type, dest, adr, send_prio, seq_nr, 0, TIMS_HEADLEN);

    ret = (int)tims_sendmsg(fildes, &head, NULL, 0, 0);

    if (ret != TIMS_HEADLEN)
        return ret;

    return 0;
}

/**
 * @brief Send a reply message (without data)
 *
 * This function sends a reply message without any data.
 *
 * @param type Message type
 * @param msgInfo Message info of the previously received message
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: possible
 */
int RackMailbox::sendMsgReply(int8_t type, message_info* msgInfo)
{
    tims_msg_head head;
    int ret;

    if (!mbxOK())
        return -ENODEV;

    if (!msgInfo)
        return -EINVAL;

    tims_fill_head(&head, type, msgInfo->src, adr, msgInfo->priority, msgInfo->seq_nr, 0, TIMS_HEADLEN);

    ret = (int)tims_sendmsg(fildes, &head, NULL, 0, 0);

    if (ret != TIMS_HEADLEN)
        return ret;

    return 0;
}

/**
 * @brief Send a data message
 *
 * This function sends a data message.
 *
 * @param type Message type
 * @param dest Destination mailbox address
 * @param seq_nr Sequence number
 * @param dataPointers Number of data pointers to data buffers which
 *                     have to send.
 * @param data1 Pointer to first data buffer
 * @param datalen1 Length of first data buffer
 * @param ... Next Buffer pointer with next buffer size, ...
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: possible
 */
int RackMailbox::sendDataMsg(int8_t type, uint32_t dest, uint8_t seq_nr,
                             int dataPointers, void* data1, uint32_t datalen1,
                             ...)
{
    int i = 1;
    uint32_t msglen;
    int32_t  ret;
    tims_msg_head head;
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

    tims_fill_head(&head, type, dest, adr, send_prio, seq_nr, 0, msglen);

    ret = tims_sendmsg(fildes, &head, iov, dataPointers, 0);

    if (ret < 0)    // error
        return ret;

    if ((uint32_t)ret != msglen)
       return -EFAULT;

    return 0;
}

/**
 * @brief Send a data message (using tims_msg_head)
 *
 * This function sends a data message (using tims_msg_head).
 *
 * @param p_head Pointer to a filled struct tims_msg_head.
 * @param dataPointers Number of data pointers to data buffers which
 *                     have to send.
 * @param data1 Pointer to first data buffer
 * @param datalen1 Length of first data buffer
 * @param ... Next Buffer pointer with next buffer size, ...
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: possible
 */
int RackMailbox::sendDataMsg(tims_msg_head *p_head, int dataPointers, void* data1,
                             uint32_t datalen1, ...)
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

/**
 * @brief Send a reply data message
 *
 * This function sends a reply message muit attached data.
 *
 * @param type Message type
 * @param msgInfo Message info of the previously received message
 * @param dataPointers Number of data pointers to data buffers which
 *                     have to send.
 * @param data1 Pointer to first data buffer
 * @param datalen1 Length of first data buffer
 * @param ... Next Buffer pointer with next buffer size, ...
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: possible
 */
int RackMailbox::sendDataMsgReply(int8_t type, message_info* msgInfo,
                                  int dataPointers, void* data1,
                                  uint32_t datalen1, ...)
{
    int             i = 1;
    uint32_t        msglen;
    int32_t         ret;
    tims_msg_head   head;
    struct iovec    iov[dataPointers];

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

    tims_fill_head(&head, type, msgInfo->src, adr, msgInfo->priority, msgInfo->seq_nr, 0, msglen);

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

/**
 * @brief Receive message (without copying data - infinite blocking)
 *
 * This function receives the newest message with the highest priority
 * without copying data. RACK gets only the pointer to the message and
 * fills in the message information data structure.
 * Using the @a peek() function the receiver can manipulate the message data
 * in the mailbox directly.
 *
 * This function can only be called if the mailbox is created in userspace.
 * Userspace tasks can't access kernel mailboxes using @a peek().
 *
 * If no message is inside the mailbox @a peek() is blocking until
 * a message is received.
 *
 * After the return of this function the mailbox slot is locked until the
 * caller executes the @a peek_end() function.
 *
 * The @a peek() function can be called once a time. The next @a peek() call
 * have to occur after the mailbox is unlocked again.
 *
 * @param msgInfo Pointer to a @a message_info
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (RT)
 *
 * Rescheduling: possible
 */
 int RackMailbox::peek(message_info *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    if (p_peek_head)
        return -EBUSY;

    ret = tims_peek_timed(fildes, &p_peek_head, TIMS_INFINITE);
    if (ret)
    {
        p_peek_head = NULL;
        return ret;
    }
    fillMessagePeekInfo(msgInfo);
    return 0;
}

/**
 * @brief Receive message with given timeout
 * (without copying data - timeout blocking)
 *
 * This function receives the newest message with the highest priority
 * without copying data. RACK gets only the pointer to the message and
 * fills in the message information data structure.
 *
 * This function can only be called if the mailbox is created in userspace.
 * Userspace tasks can't access kernel mailboxes using @a peek().
 *
 * If no message is inside the mailbox @a peekTimed() is blocking until
 * a message is received or the timeout is expired.
 *
 * After the return of this function the mailbox slot is locked until the
 * caller executes the @a peek_end() function.
 *
 * The @a peek() function can be called once a time. The next @a peek() call
 * have to occur after the mailbox is unlocked again.
 *
 * @param timeout_ns Receive timeout in nanoceconds
 * @param msgInfo Pointer to a @a message_info
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (RT)
 *
 * Rescheduling: possible
 */
int RackMailbox::peekTimed(uint64_t timeout_ns, message_info *msgInfo)
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

/**
 * @brief Receive message if a message is available
 * (without copying data - non-blocking)
 *
 * This function receives the newest message with the highest priority
 * without copying data. RACK gets only the pointer to the message and
 * fills in the message information data structure.
 *
 * This function can only be called if the mailbox is created in userspace.
 * Userspace tasks can't access kernel mailboxes using @a peek().
 *
 * If no message is inside the mailbox @a peekIf() returns immediately
 * with the returncode -EWOULDBLOCK.
 *
 * After the return of this function the mailbox slot is locked until the
 * caller executes the @a peek_end() function.
 *
 * The @a peek() function can be called once a time. The next @a peek() call
 * have to occur after the mailbox is unlocked again.
 *
 * @param msgInfo Pointer to a @a message_info
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (RT)
 *
 * Rescheduling: none
 */
int RackMailbox::peekIf(message_info *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    if (p_peek_head)
        return -EBUSY;

    ret = tims_peek_timed(fildes, &p_peek_head, TIMS_NONBLOCK);
    if (ret)
    {
        p_peek_head = NULL;
        return ret;
    }
    fillMessagePeekInfo(msgInfo);
    return 0;
}

/**
 * @brief Unlocking the mailbox after receiving a message.
 *
 * This function unlocks the mailbox after a message is received with @a peek(),
 * @a peek_timed() or @a peek_if().
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (RT)
 *
 * Rescheduling: none
 */
int RackMailbox::peekEnd(void)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    if (!p_peek_head)
        return -EFAULT;

    ret = tims_peek_end(fildes);
    if (ret)
        return ret;

    p_peek_head = NULL;
    return 0;
}

//
// receive
//

/**
 * @brief Receive message (no data - infinite blocking)
 *
 * This function receives the newest message with the highest priority.
 * The received message (without data) is written into the message info
 * data structure @a message_info.
 *
 * This function can be called if the mailbox is created in userspace or in
 * kernelspace.
 *
 * If no message is inside the mailbox @a recvMsg() is blocking until
 * a message is received.
 *
 * @param msgInfo Pointer to a @a message_info
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (RT)
 *
 * Rescheduling: possible
 */
int RackMailbox::recvMsg(message_info *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    ret = tims_recvmsg_timed(fildes, (tims_msg_head *)msgInfo, NULL, 0, TIMS_INFINITE, 0);
    if (ret)
        return ret;

    fillMessageRecvInfo(msgInfo, NULL);
    return 0;
}

/**
 * @brief Receive message (no data - timeout blocking)
 *
 * This function receives the newest message with the highest priority.
 * The received message (without data) is written into the message info
 * data structure @a message_info.
 *
 * This function can be called if the mailbox is created in userspace or in
 * kernelspace.
 *
 * If no message is inside the mailbox @a recvMsgTimed() is blocking until
 * a message is received or the timeout is expired.
 *
 * @param timeout_ns Receive timeout in nanoceconds
 * @param msgInfo Pointer to a @a message_info
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (RT)
 *
 * Rescheduling: possible
 */
int RackMailbox::recvMsgTimed(uint64_t timeout_ns, message_info *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    ret = tims_recvmsg_timed(fildes, (tims_msg_head *)msgInfo, NULL, 0, timeout_ns, 0);
    if (ret)
        return ret;

    fillMessageRecvInfo(msgInfo, NULL);
    return 0;
}

/**
 * @brief Receive message (no data - non-blocking)
 *
 * This function receives the newest message with the highest priority.
 * The received message (without data) is written into the message info
 * data structure @a message_info.
 *
 * This function can be called if the mailbox is created in userspace or in
 * kernelspace.
 *
 * If no message is inside the mailbox @a recvMsgIf() returns immediately
 * with the returncode -EWOULDBLOCK.
 *
 * @param msgInfo Pointer to a @a message_info
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (RT)
 *
 * Rescheduling: none
 */
int     RackMailbox::recvMsgIf(message_info *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    ret = tims_recvmsg_timed(fildes, (tims_msg_head *)msgInfo, NULL, 0, TIMS_NONBLOCK, 0);
    if (ret)
        return ret;

    fillMessageRecvInfo(msgInfo, NULL);
    return 0;
}

/**
 * @brief Receive message (with data - infinite blocking)
 *
 * This function receives the newest message with the highest priority.
 * The received message head is written into the message info
 * data structure @a message_info and all data into the given data buffer.
 *
 * This function can be called if the mailbox is created in userspace or in
 * kernelspace.
 *
 * If no message is inside the mailbox @a recvDataMsg() is blocking until
 * a message is received.
 *
 * @param p_data Pointer to the receive data buffer
 * @param maxdatalen Size of the receive data buffer
 * @param msgInfo Pointer to a @a message_info
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (RT)
 *
 * Rescheduling: possible
 */
int     RackMailbox::recvDataMsg(void *p_data, uint32_t maxdatalen,
                                 message_info *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    ret = tims_recvmsg_timed(fildes, (tims_msg_head *)msgInfo, p_data, maxdatalen, TIMS_INFINITE, 0);
    if (ret)
        return ret;

    fillMessageRecvInfo(msgInfo, p_data);
    return 0;
}

/**
 * @brief Receive message (with data - timeout blocking)
 *
 * This function receives the newest message with the highest priority.
 * The received message head is written into the message info
 * data structure @a message_info and all data into the given data buffer.
 *
 * This function can be called if the mailbox is created in userspace or in
 * kernelspace.
 *
 * If no message is inside the mailbox @a recvDataMsgTimed() is blocking until
 * a message is received or the timeout is expired.
 *
 * @param timeout_ns Receive timeout in nanoceconds
 * @param p_data Pointer to the receive data buffer
 * @param maxdatalen Size of the receive data buffer
 * @param msgInfo Pointer to a @a message_info
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (RT)
 *
 * Rescheduling: possible
 */
int     RackMailbox::recvDataMsgTimed(uint64_t timeout_ns, void *p_data,
                                      uint32_t maxdatalen, message_info *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    ret = tims_recvmsg_timed(fildes, (tims_msg_head *)msgInfo, p_data, maxdatalen, timeout_ns, 0);
    if (ret)
        return ret;

    fillMessageRecvInfo(msgInfo, p_data);
    return 0;
}

/**
 * @brief Receive message (no data - non-blocking)
 *
 * This function receives the newest message with the highest priority.
 * The received message head is written into the message info
 * data structure @a message_info and all data into the given data buffer.
 *
 * This function can be called if the mailbox is created in userspace or in
 * kernelspace.
 *
 * If no message is inside the mailbox @a recvDataMsgIf() returns immediately
 * with the returncode -EWOULDBLOCK.
 *
 * @param p_data Pointer to the receive data buffer
 * @param maxdatalen Size of the receive data buffer
 * @param msgInfo Pointer to a @a message_info
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (RT)
 *
 * Rescheduling: none
 */
int     RackMailbox::recvDataMsgIf(void *p_data, uint32_t maxdatalen,
                                   message_info *msgInfo)
{
    int ret;

    if (!mbxOK())
        return -ENODEV;

    ret = tims_recvmsg_timed(fildes, (tims_msg_head *)msgInfo, p_data, maxdatalen, TIMS_NONBLOCK, 0);
    if (ret)
        return ret;

    fillMessageRecvInfo(msgInfo, p_data);
    return 0;
}

/*@}*/
