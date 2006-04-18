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
#include <main/rack_proxy.h>

#include <main/rack_module.h>
#include <main/rack_name.h>

//######################################################################
//# class RackProxy
//######################################################################

//
// constructor and destructor
//

RackProxy::RackProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t class_id,
                     uint32_t instance)
{
    this->workMbx     = workMbx;
    this->sysId       = sys_id;
    this->classId     = class_id;
    this->instance    = instance;

    onTimeout   = 5000000000llu;	// 5s
    offTimeout  = 1000000000llu;	// 1s
    dataTimeout =  100000000llu;	// 100ms

    destMbxAdr = RackName::create(sysId, classId, instance);

    // only for debug messages
    gdos = new GdosMailbox(workMbx, GDOS_MSG_DEBUG_DEFAULT);
}

RackProxy::~RackProxy()
{
    delete gdos;
}

//
// send functions
//

// sendProxyCmd
// Sends a message with a given type and waits <timeout> ns for a reply.
// The send and the reply message doesn't contain any data
int RackProxy::proxySendCmd(int8_t send_msgtype, uint64_t timeout)
{
    int ret;

    if (!workMbx)
    {
        return -EINVAL;
    }

    ret = workMbx->sendMsg(send_msgtype, destMbxAdr, 0);
    if (ret)
    {
        GDOS_WARNING("Proxy cmd to %n: Can't send command %d, code = %d\n",
                        destMbxAdr, send_msgtype, ret);
        return ret;
    }
    GDOS_DBG_DETAIL("Proxy cmd to %n: command %d has been sent\n",
                        destMbxAdr, send_msgtype);

    while (1)
    {   // waiting for reply (without data)
        ret = workMbx->recvMsgTimed(timeout, &info);
        if (ret)
        {
            GDOS_WARNING("Proxy cmd to %n: Can't receive reply of "
                         "command %d, code = %d\n",
                         destMbxAdr, send_msgtype, ret);
            return ret;
        }

        if (info.src == destMbxAdr)
        {
            switch(info.type)
            {
                case MSG_ERROR:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - error -\n",
                                 send_msgtype, destMbxAdr);
                    return -ECOMM;

                case MSG_TIMEOUT:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - timeout -\n",
                                 send_msgtype, destMbxAdr);
                    return -ETIMEDOUT;

                case MSG_NOT_AVAILABLE:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - not available \n",
                                 send_msgtype, destMbxAdr);
                    return -ENODATA;

                case MSG_OK:
                    return 0;
            }
        }
    } // while-loop
    return -EINVAL;
}

// sendProxyDataCmd
// Sends a data message with a given type, a send-pointer and the send-datasize
// and waits <timeout> ns for a reply.
// The reply message doesn't contain any data
int RackProxy::proxySendDataCmd(int8_t send_msgtype, void *send_data,
                                size_t send_datalen, uint64_t timeout)
{
    int ret;

    if (!workMbx)
    {
        return -EINVAL;
    }

    ret = workMbx->sendDataMsg(send_msgtype, destMbxAdr, 0, 1, send_data, send_datalen);
    if (ret)
    {
        GDOS_WARNING("Proxy cmd to %n: Can't send command %d, code = %d\n",
                     destMbxAdr, send_msgtype, ret);
        return ret;
    }

    while (1)
    {   // waiting for reply (without data)
        ret = workMbx->recvMsgTimed(timeout, &info);
        if (ret)
        {
            GDOS_WARNING("Proxy cmd to %n: Can't receive reply of "
                         "command %d, code = %d\n",
                         destMbxAdr, send_msgtype, ret);
            return ret;
        }

        if (info.src == destMbxAdr)
        {
            switch(info.type) {
                case MSG_ERROR:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - error -\n",
                                 send_msgtype, destMbxAdr);
                    return -ECOMM;

                case MSG_TIMEOUT:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - timeout -\n",
                                 send_msgtype, destMbxAdr);
                    return -ETIMEDOUT;

                case MSG_NOT_AVAILABLE:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - not available \n",
                                 send_msgtype, destMbxAdr);
                    return -ENODATA;

                case MSG_OK:
                return 0;
            }
        }
    } // while-loop
    return -EINVAL;
}

// sendProxyDataCmd
// Sends a data message with a given type, a send-pointer and the send-datasize
// and waits <timeout> ns for a reply.
// Only the reply message contains data
int RackProxy::proxyRecvDataCmd(int8_t send_msgtype, const int8_t recv_msgtype,
                                void *recv_data, size_t recv_datalen,
                                uint64_t timeout, MessageInfo *msgInfo)
{
    int ret;

    if (!workMbx)
    {
        return -EINVAL;
    }

    ret = workMbx->sendMsg(send_msgtype, destMbxAdr, 0);
    if (ret)
    {
        GDOS_WARNING("Proxy cmd to %n: Can't send command %d, code = %d\n",
                     destMbxAdr, send_msgtype, ret);
        return ret;
    }

    while (1)
    {
        ret = workMbx->recvDataMsgTimed(timeout, recv_data, recv_datalen, msgInfo);
        if (ret)
        {
            GDOS_WARNING("Proxy cmd to %n: Can't receive reply of "
                         "command %d, code = %d\n",
                         destMbxAdr, send_msgtype, ret);
            return ret;
        }

        if (msgInfo->src == destMbxAdr)
        {
            switch(msgInfo->type) {
                case MSG_ERROR:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - error -\n",
                                 send_msgtype, destMbxAdr);
                    return -ECOMM;

                case MSG_TIMEOUT:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - timeout -\n",
                                 send_msgtype, destMbxAdr);
                    return -ETIMEDOUT;

                case MSG_NOT_AVAILABLE:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - not available \n",
                                 send_msgtype, destMbxAdr);
                    return -ENODATA;
            }

            if ( msgInfo->type == recv_msgtype)
            {
                return 0;
            }
        }
    } // while-loop
    return -EINVAL;
}

// sendProxyDataCmd
// Sends a data message with a given type, a send-pointer and the send-datasize
// and waits <timeout> ns for a reply.
// The send and the reply message contain data
int RackProxy::proxySendRecvDataCmd(int8_t send_msgtype, void *send_data,
                                    size_t send_datalen, const int8_t recv_msgtype,
                                    void *recv_data, size_t recv_datalen,
                                    uint64_t timeout, MessageInfo *msgInfo)
{
    int ret;

    if (!workMbx)
    {
        return -EINVAL;
    }

    ret = workMbx->sendDataMsg(send_msgtype, destMbxAdr, 0, 1, send_data, send_datalen);
    if (ret)
    {
        GDOS_WARNING("Proxy cmd to %n: Can't send command %d, code = %d\n",
                     destMbxAdr, send_msgtype, ret);
        return ret;
    }

    while (1) {
        ret = workMbx->recvDataMsgTimed(timeout, recv_data, recv_datalen, msgInfo);
        if (ret)
        {
            GDOS_WARNING("Proxy cmd to %n: Can't receive reply of "
                         "command %d, code = %d\n",
                         destMbxAdr, send_msgtype, ret);
            return ret;
        }

        if (msgInfo->src == destMbxAdr)
        {
            switch(msgInfo->type) {
                case MSG_ERROR:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - error -\n",
                                 send_msgtype, destMbxAdr);
                    return -ECOMM;

                case MSG_TIMEOUT:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - timeout -\n",
                                 send_msgtype, destMbxAdr);
                    return -ETIMEDOUT;

                case MSG_NOT_AVAILABLE:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - not available \n",
                                 send_msgtype, destMbxAdr);
                    return -ENODATA;
            }

            if ( msgInfo->type == recv_msgtype)
            {
                return 0;
            }
        }
    } // while-loop
    return -EINVAL;
}


//######################################################################
//# class RackDataProxy
//######################################################################

//
// constructor and destructor
//

RackDataProxy::RackDataProxy(RackMailbox *workMbx, uint32_t sys_id,
                             uint32_t class_id, uint32_t instance) :
        RackProxy(workMbx, sys_id, class_id, instance)
{
}

RackDataProxy::~RackDataProxy()
{
}


//
// get data
//

int RackDataProxy::getData(void *recv_data, ssize_t recv_datalen,
                           RACK_TIME timeStamp, uint64_t reply_timeout_ns,
                           MessageInfo *msgInfo)
{
    rack_get_data send_data;
    send_data.recordingTime = timeStamp;

    return proxySendRecvDataCmd(MSG_GET_DATA, &send_data, sizeof(rack_get_data),
                                MSG_DATA, recv_data, recv_datalen,
                                reply_timeout_ns, msgInfo);
}

//
// get continuous data
//

int RackDataProxy::getContData(RACK_TIME requestPeriodTime, RackMailbox *dataMbx,
                               RACK_TIME *realPeriodTime, uint64_t reply_timeout_ns)
{
    int ret;
    MessageInfo        info;
    rack_get_cont_data send_data;
    rack_cont_data     recv_data;

    send_data.periodTime = requestPeriodTime;
    send_data.dataMbxAdr = dataMbx->getAdr();

    ret = proxySendRecvDataCmd(MSG_GET_CONT_DATA, &send_data,
                               sizeof(rack_get_cont_data),
                               MSG_CONT_DATA, &recv_data,
                               sizeof(rack_cont_data), reply_timeout_ns, &info);
    if (ret)
        return ret;

    RackContData::parse(&info);

    if (realPeriodTime)
    {
        *realPeriodTime = recv_data.periodTime;
    }

    return 0;
}

//
// stop continuous data
//

int RackDataProxy::stopContData(RackMailbox *dataMbx, uint64_t reply_timeout_ns)
{
  rack_stop_cont_data send_data;
  send_data.dataMbxAdr = dataMbx->getAdr();

  return proxySendDataCmd(MSG_STOP_CONT_DATA, &send_data,
                          sizeof(rack_stop_cont_data), reply_timeout_ns);
}
