/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Daniel Lecking <lecking@rts.uni-hannover.de>
 *
 */

#include <control/planner_proxy.h>

int PlannerProxy::getData(planner_data *recv_data, ssize_t recv_datalen,
                        rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    RackMessage msgInfo;

    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen,
                                     timeStamp, reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = PlannerData::parse(&msgInfo);
    return 0;
}

int PlannerProxy::getCommandList(planner_command *recv_data, ssize_t recv_datalen,
                          uint64_t reply_timeout_ns)
{
    RackMessage msgInfo;

    int ret = proxyRecvDataCmd(MSG_PLANNER_GET_COMMAND, MSG_PLANNER_COMMAND,
                              (void *)recv_data, recv_datalen,
                              reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = PlannerCommand::parse(&msgInfo);
    return 0;
}

int PlannerProxy::sendCommand(planner_command *recv_data, ssize_t recv_datalen,
                          uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_PLANNER_COMMAND, recv_data, recv_datalen,
                            reply_timeout_ns);
}


