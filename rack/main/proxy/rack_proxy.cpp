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

    onTimeout   = 5000000000llu;    // 5s
    offTimeout  = 1000000000llu;    // 1s
    dataTimeout =  100000000llu;    // 100ms

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

/** Remote procedure calling with no data in command msg and reply msg.
 *  Sends a message with a given type and waits timeout ns for a reply.
 */
int RackProxy::proxySendCmd(int8_t send_msgtype, uint64_t timeout)
{
    message_info msgInfo;
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
        ret = workMbx->recvMsgTimed(timeout, &msgInfo);
        if (ret)
        {
            GDOS_WARNING("Proxy cmd to %n: Can't receive reply of "
                         "command %d, code = %d\n",
                         destMbxAdr, send_msgtype, ret);
            return ret;
        }

        if (msgInfo.src == destMbxAdr)
        {
            switch(msgInfo.type)
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

/** Remote procedure calling with data in command msg and no data in reply msg.
 *  Sends a data message with a given type, a send-pointer and the send-datasize
 *  and waits timeout ns for a reply.
 */
int RackProxy::proxySendDataCmd(int8_t send_msgtype, void *send_data,
                                size_t send_datalen, uint64_t timeout)
{
    message_info msgInfo;
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
        ret = workMbx->recvMsgTimed(timeout, &msgInfo);
        if (ret)
        {
            GDOS_WARNING("Proxy cmd to %n: Can't receive reply of "
                         "command %d, code = %d\n",
                         destMbxAdr, send_msgtype, ret);
            return ret;
        }

        if (msgInfo.src == destMbxAdr)
        {
            switch(msgInfo.type) {
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

/** Remote procedure calling with no data in command msg and data in reply msg.
 *  Sends a data message with a given type, a send-pointer and the send-datasize
 *  and waits timeout ns for a reply.
 */
int RackProxy::proxyRecvDataCmd(int8_t send_msgtype, const int8_t recv_msgtype,
                                void *recv_data, size_t recv_datalen,
                                uint64_t timeout, message_info *msgInfo)
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

/** Remote procedure calling with data in command msg and reply msg.
 *  Sends a data message with a given type, a send-pointer and the send-datasize
 *  and waits timeout ns for a reply.
 */
int RackProxy::proxySendRecvDataCmd(int8_t send_msgtype, void *send_data,
                                    size_t send_datalen, const int8_t recv_msgtype,
                                    void *recv_data, size_t recv_datalen,
                                    uint64_t timeout, message_info *msgInfo)
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

int RackProxy::getStatus(uint64_t reply_timeout_ns) // use special timeout
{
    message_info msgInfo;
    int ret;

    if (!workMbx)
    {
        return -EINVAL;
    }

    ret = workMbx->sendMsg(MSG_GET_STATUS, destMbxAdr, 0);
    if (ret)
    {
        GDOS_WARNING("Proxy cmd to %n: Can't send command %d, code = %d\n",
                        destMbxAdr, MSG_GET_STATUS, ret);
        return ret;
    }
    GDOS_DBG_DETAIL("Proxy cmd to %n: command %d has been sent\n",
                        destMbxAdr, MSG_GET_STATUS);

    while (1)
    {   // waiting for reply (without data)
        ret = workMbx->recvMsgTimed(reply_timeout_ns, &msgInfo);
        if (ret)
        {
            GDOS_WARNING("Proxy cmd to %n: Can't receive reply of "
                         "command %d, code = %d\n",
                         destMbxAdr, MSG_GET_STATUS, ret);
            return ret;
        }

        if (msgInfo.src == destMbxAdr)
        {
            switch(msgInfo.type)
            {
                case MSG_TIMEOUT:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - timeout -\n",
                                 MSG_GET_STATUS, destMbxAdr);
                    return -ETIMEDOUT;

                case MSG_NOT_AVAILABLE:
                    GDOS_WARNING("Proxy cmd %d to %n: Replied - not available \n",
                                 MSG_GET_STATUS, destMbxAdr);
                    return -ENODATA;

                case MSG_ENABLED:
                case MSG_DISABLED:
                case MSG_ERROR:
                    return msgInfo.type;
            }
        }
    } // while-loop
    return -EINVAL;
}

int RackProxy::getParameter(rack_param_msg *parameter, int maxParameterNum, uint64_t reply_timeout_ns)
{
    message_info msgInfo;

    return proxyRecvDataCmd(MSG_GET_PARAM, MSG_PARAM,
                            parameter, sizeof(rack_param_msg) + maxParameterNum * sizeof(rack_param),
                            reply_timeout_ns, &msgInfo);
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
                           rack_time_t timeStamp, uint64_t reply_timeout_ns,
                           message_info *msgInfo)
{
    rack_get_data send_data;
    send_data.recordingTime = timeStamp;

    return proxySendRecvDataCmd(MSG_GET_DATA, &send_data, sizeof(rack_get_data),
                                MSG_DATA, recv_data, recv_datalen,
                                reply_timeout_ns, msgInfo);
}

//
// get next data
//

int RackDataProxy::getNextData(void *recv_data, ssize_t recv_datalen,
                               uint64_t reply_timeout_ns,
                               message_info *msgInfo)
{
    return proxySendRecvDataCmd(MSG_GET_NEXT_DATA, NULL, 0,
                                MSG_DATA, recv_data, recv_datalen,
                                reply_timeout_ns, msgInfo);
}

//
// get continuous data
//

int RackDataProxy::getContData(rack_time_t requestPeriodTime, RackMailbox *dataMbx,
                               rack_time_t *realPeriodTime, uint64_t reply_timeout_ns)
{
    int ret;
    message_info        msgInfo;
    rack_get_cont_data send_data;
    rack_cont_data     recv_data;

    send_data.periodTime = requestPeriodTime;
    send_data.dataMbxAdr = dataMbx->getAdr();

    ret = proxySendRecvDataCmd(MSG_GET_CONT_DATA, &send_data,
                               sizeof(rack_get_cont_data),
                               MSG_CONT_DATA, &recv_data,
                               sizeof(rack_cont_data), reply_timeout_ns, &msgInfo);
    if (ret)
        return ret;

    RackContData::parse(&msgInfo);

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
