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
 *      Oliver Wulf        <oliver.wulf@gmx.de>
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 */
#include <tools/datalog_proxy.h>

//
// proxy functions
//
int DatalogProxy::getData(datalog_data *recv_data, ssize_t recv_datalen,
                         rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    message_info msgInfo;

    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen, timeStamp,
                                    reply_timeout_ns, &msgInfo);
    if (ret) {
        return ret;
    }

    recv_data = DatalogProxy::parse(&msgInfo);
    return 0;
}

int DatalogProxy::getLogStatus(datalog_info_data *recv_data, ssize_t recv_datalen,
                               uint64_t reply_timeout_ns)
{
    message_info msgInfo;

    int ret = proxyRecvDataCmd(MSG_DATALOG_GET_LOG_STATUS, MSG_DATALOG_LOG_STATUS,
                               recv_data, recv_datalen,
                               reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = DatalogProxy::parse(&msgInfo);
    return 0;
}

int DatalogProxy::setLog(datalog_info_data *recv_data, ssize_t recv_datalen,
                         uint64_t reply_timeout_ns);
{
    return proxySendDataCmd(MSG_DATALOG_SET_LOG, recv_data,
                            recv_datalen,
                            reply_timeout_ns);
}

int DatalogProxy::reset(uint64_t reply_timeout_ns)
{
    chassis_set_active_pilot_data send_data;
    send_data.activePilot = pilotMbxAdr;

    return proxySendCmd(MSG_DATALOG_RESET, reply_timeout_ns);
}

