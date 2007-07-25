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
}

void RackMailbox::fillMessagePeekInfo(message_info *msgInfo, tims_msg_head* p_peek_head)
{
    msgInfo->flags    = p_peek_head->flags;
    msgInfo->type     = p_peek_head->type;
    msgInfo->priority = p_peek_head->priority;
    msgInfo->seq_nr   = p_peek_head->seq_nr;
    msgInfo->dest     = p_peek_head->dest;
    msgInfo->src      = p_peek_head->src;
    msgInfo->datalen  = p_peek_head->msglen - TIMS_HEADLEN;
    msgInfo->p_data   = &p_peek_head->data;
}

 /*!
 * @ingroup mailbox
 *
 *@{*/

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
    fd          = -1;
    adr         = 0;
    sendPrio    = 0;
}

/**
 * @brief Create a mailbox
 *
 * This function creates a mailbox to send and receive messages.
 * Using this function a unique mailbox address has to be given.
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
                        ssize_t maxDatalen, void *buffer,
                        ssize_t bufferSize, int8_t sendPriority)
{
    ssize_t maxMsglen = maxDatalen + TIMS_HEADLEN;

    fd = tims_mbx_create(address, messageSlots, maxMsglen, buffer, bufferSize);

    if (fd < 0)
    {
        adr         = 0;
        sendPrio    = 0;

        return fd;
    }

    adr         = address;
    sendPrio    = sendPriority;

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

    ret = tims_mbx_remove(fd);

    fd          = -1;
    adr         = 0;
    sendPrio    = 0;

    return ret;
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
    return tims_mbx_clean(fd);
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
int RackMailbox::sendMsg(int8_t type, uint32_t dest, uint8_t seqNr)
{
    tims_msg_head head;
    int ret;

    tims_fill_head(&head, type, dest, adr, sendPrio, seqNr, 0, TIMS_HEADLEN);

    ret = tims_sendmsg(fd, &head, NULL, 0, 0);

    if (ret < 0)
        return ret;

    if (ret != (int)TIMS_HEADLEN)
       return -EFAULT;

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

    tims_fill_head(&head, type, msgInfo->src, adr, msgInfo->priority, msgInfo->seq_nr, 0, TIMS_HEADLEN);

    ret = tims_sendmsg(fd, &head, NULL, 0, 0);

    if (ret < 0)
        return ret;

    if (ret != (int)TIMS_HEADLEN)
       return -EFAULT;

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
int RackMailbox::sendDataMsg(int8_t type, uint32_t dest, uint8_t seqNr,
                             int dataPointers, void* data1, uint32_t datalen1, ...)
{
    int i = 1;
    uint32_t msglen;
    int  ret;
    tims_msg_head head;
    struct iovec iov[dataPointers];

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

    tims_fill_head(&head, type, dest, adr, sendPrio, seqNr, 0, msglen);

    ret = tims_sendmsg(fd, &head, iov, dataPointers, 0);

    if (ret < 0)
        return ret;

    if (ret != (int)msglen)
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
int RackMailbox::sendDataMsg(tims_msg_head *p_head, int dataPointers, void* data1, uint32_t datalen1, ...)
{
    int i = 1;
    uint32_t msglen;
    int ret;
    struct iovec iov[dataPointers];

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

    ret = tims_sendmsg(fd, p_head, iov, dataPointers, 0);

    if (ret < 0)
        return ret;

    if (ret != (int)msglen)
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
                                  int dataPointers, void* data1, uint32_t datalen1, ...)
{
    int             i = 1;
    uint32_t        msglen;
    int32_t         ret;
    tims_msg_head   head;
    struct iovec    iov[dataPointers];

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

    ret = tims_sendmsg(fd, &head, iov, dataPointers, 0);

    if (ret < 0)
        return ret;

    if (ret != (int)msglen)
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
    tims_msg_head *p_peek_head;
    int ret;

    ret = tims_peek_timed(fd, &p_peek_head, TIMS_INFINITE);

    if (ret < 0)
        return ret;

    fillMessagePeekInfo(msgInfo, p_peek_head);
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
    tims_msg_head *p_peek_head;
    int ret;

    ret = tims_peek_timed(fd, &p_peek_head, timeout_ns);

    if (ret < 0)
        return ret;

    fillMessagePeekInfo(msgInfo, p_peek_head);
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
    tims_msg_head *p_peek_head;
    int ret;

    ret = tims_peek_timed(fd, &p_peek_head, TIMS_NONBLOCK);

    if (ret < 0)
        return ret;

    fillMessagePeekInfo(msgInfo, p_peek_head);
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
    return tims_peek_end(fd);
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

    ret = tims_recvmsg_timed(fd, (tims_msg_head *)msgInfo, NULL, 0, TIMS_INFINITE, 0);

    if (ret < 0)
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

    ret = tims_recvmsg_timed(fd, (tims_msg_head *)msgInfo, NULL, 0, timeout_ns, 0);

    if (ret < 0)
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

    ret = tims_recvmsg_timed(fd, (tims_msg_head *)msgInfo, NULL, 0, TIMS_NONBLOCK, 0);

    if (ret < 0)
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
int     RackMailbox::recvDataMsg(void *p_data, uint32_t maxDatalen, message_info *msgInfo)
{
    int ret;

    ret = tims_recvmsg_timed(fd, (tims_msg_head *)msgInfo, p_data, maxDatalen, TIMS_INFINITE, 0);

    if (ret < 0)
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
int     RackMailbox::recvDataMsgTimed(uint64_t timeout_ns, void *p_data, uint32_t maxDatalen, message_info *msgInfo)
{
    int ret;

    ret = tims_recvmsg_timed(fd, (tims_msg_head *)msgInfo, p_data, maxDatalen, timeout_ns, 0);

    if (ret < 0)
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
int     RackMailbox::recvDataMsgIf(void *p_data, uint32_t maxDatalen, message_info *msgInfo)
{
    int ret;

    ret = tims_recvmsg_timed(fd, (tims_msg_head *)msgInfo, p_data, maxDatalen, TIMS_NONBLOCK, 0);

    if (ret < 0)
        return ret;

    fillMessageRecvInfo(msgInfo, p_data);
    return 0;
}

/*@}*/
