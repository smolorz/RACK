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
#include <skel/new_data_module_proxy.h>

//
// proxy functions
//


int NewDataModuleProxy::getData(new_data *recv_data, ssize_t recv_datalen,
                                RACK_TIME timeStamp, uint64_t reply_timeout_ns,
                                MessageInfo *msgInfo)
{
    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen, timeStamp,
                                    reply_timeout_ns, msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = NewData::parse(msgInfo);
    return 0;
}

int NewDataModuleProxy::sendDataCmd(uint64_t reply_timeout_ns)
{
    send_data data;
    data.val_X = 20.3;
    data.val_Y = 12;

    return proxySendDataCmd(MSG_SEND_DATA_CMD, &send_data, sizeof(send_data),
                            reply_timeout_ns);
}

int NewDataModuleProxy::recvDataCmd(new_data* recv_data, ssize_t recv_datalen,
                                    uint64_t reply_timeout_ns,
                                    MessageInfo *msgInfo)
{
    int ret = proxyRecvDataCmd(MSG_RECV_DATA_CMD, MSG_DATA, recv_data,
                                recv_datalen, reply_timeout_ns, msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = NewData::parse(msgInfo);
    return 0;
}

int sendRecvDataCmd(void *send_data, size_t send_datalen,
                    new_data *recv_data, size_t recv_datalen,
                    uint64_t reply_timeout_ns, MessageInfo *msgInfo)
{
    int ret = proxySendRecvDataCmd(MSG_SEND_RECV_DATA_CMD, &send_data,
                                   sizeof(send_data), MSG_DATA,
                                   (void *)recv_data, recv_datalen,
                                   reply_timeout_ns, msgInfo);

    if (ret)
    {
        return ret;
    }

    recv_data = NewData::parse(msgInfo);
    return 0;
}
