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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
#include <drivers/chassis_proxy.h>

//
// proxy functions
//

int ChassisProxy::getData(chassis_data *recv_data, ssize_t recv_datalen,
                          rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    message_info msgInfo;

    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen, timeStamp,
                                    reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = ChassisData::parse(&msgInfo);
    return 0;
}

int ChassisProxy::move(int32_t vx, int32_t vy, float omega, uint64_t reply_timeout_ns)
{
    chassis_move_data send_data;
    send_data.vx    = vx;
    send_data.vy    = vy;
    send_data.omega = omega;

    return proxySendDataCmd(MSG_CHASSIS_MOVE, &send_data,
                            sizeof(chassis_move_data), reply_timeout_ns);
}

int ChassisProxy::getParam(chassis_param_data *recv_data, ssize_t recv_datalen,
                           uint64_t reply_timeout_ns)
{
    message_info msgInfo;

    int ret = proxyRecvDataCmd(MSG_CHASSIS_GET_PARAMETER, MSG_CHASSIS_PARAMETER,
                              (void *)recv_data, recv_datalen,
                              reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = ChassisParamData::parse(&msgInfo);
    return 0;
}

int ChassisProxy::setActivePilot(uint32_t pilotMbxAdr, uint64_t reply_timeout_ns)
{
    chassis_set_active_pilot_data send_data;
    send_data.activePilot = pilotMbxAdr;

    return proxySendDataCmd(MSG_CHASSIS_SET_ACTIVE_PILOT, &send_data,
                            sizeof(chassis_set_active_pilot_data),
                            reply_timeout_ns);
}
