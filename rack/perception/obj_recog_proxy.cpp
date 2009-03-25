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
 *      Marko Reimer  <reimer@rts.uni-hannover.de>
 *
 */
#include <perception/obj_recog_proxy.h>

int ObjRecogProxy::getData(obj_recog_data *recv_data, ssize_t recv_datalen,
                           rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    message_info msgInfo;

    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen, timeStamp,
                                     reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = ObjRecogData::parse(&msgInfo);
    return 0;
}

int ObjRecogProxy::getNextData(obj_recog_data *recv_data, ssize_t recv_datalen,
                               uint64_t reply_timeout_ns)
{
    message_info msgInfo;

    int ret = RackDataProxy::getNextData((void *)recv_data, recv_datalen,
                                     reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = ObjRecogData::parse(&msgInfo);
    return 0;
}

int ObjRecogProxy::setEstimate(obj_recog_data *recv_data, ssize_t recv_datalen,
                              uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_SET_ESTIMATE, recv_data, recv_datalen,
                             reply_timeout_ns);
}

int ObjRecogProxy::stopRecognition(obj_recog_data *recv_data, ssize_t recv_datalen,
                              uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_STOP_RECOG, recv_data, recv_datalen,
                             reply_timeout_ns);
}

