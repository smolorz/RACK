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
 *      YourName <YourMailAddress>
 *
 */
#include <skel/dummy_data_module_proxy.h>

//
// proxy functions
//


int DummyProxy::getData(dummy_data *recv_data, ssize_t recv_datalen,
                        rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    message_info msgInfo;
    
    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen, timeStamp,
                                    reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = DummyData::parse(&msgInfo);
    return 0;
}

int DummyProxy::sendParam(dummy_param *send_data, size_t send_datalen, uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(DUMMY_SEND_PARAM, send_data, send_datalen, reply_timeout_ns);
}

int DummyProxy::recvParam(dummy_data* recv_data, ssize_t recv_datalen, uint64_t reply_timeout_ns)
{
    message_info msgInfo;
    
    int ret = proxyRecvDataCmd(DUMMY_RECV_PARAM, DUMMY_PARAM, recv_data,
                                recv_datalen, reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = DummyParam::parse(&msgInfo);
    return 0;
}

int sendRecvParam(dummy_param *send_data, size_t send_datalen,
                  dummy_param *recv_data, size_t recv_datalen,
                  uint64_t reply_timeout_ns)
{
    message_info msgInfo;
    
    int ret = proxySendRecvDataCmd(DUMMY_SEND_RECV_PARAM, send_data,
                                   send_datalen, DUMMY_PARAM,
                                   recv_data, recv_datalen,
                                   reply_timeout_ns, &msgInfo);

    if (ret)
    {
        return ret;
    }

    recv_data = DummyParam::parse(&msgInfo);
    return 0;
}
