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
#include <navigation/pilot_proxy.h>

int PilotProxy::getData(pilot_data *recv_data, ssize_t recv_datalen,
                        rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    message_info msgInfo;
    
    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen,
                                     timeStamp, reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = PilotData::parse(&msgInfo);
    return 0;
}

int PilotProxy::setDestination(position_3d *pos, rack_time_t recordingTime, uint64_t reply_timeout_ns)
{
    destination_data sendData;

    sendData.recordingTime = recordingTime;
    sendData.pos.x         = pos->x;
    sendData.pos.y         = pos->y;
    sendData.pos.z         = pos->z;
    sendData.pos.phi       = pos->phi;
    sendData.pos.psi       = pos->psi;
    sendData.pos.rho       = pos->rho;

    return proxySendDataCmd(MSG_SET_DESTINATION, &sendData,
                            sizeof(destination_data), reply_timeout_ns);
}
