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
 *      Matthias Hentschel  <hentschel@rts.uni-hannover.de>
 *
 */
#include <drivers/ptz_drive_proxy.h>

int PtzDriveProxy::getData(ptz_drive_data *recv_data, ssize_t recv_datalen,
                           rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    RackMessage msgInfo;

    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen, timeStamp,
                                     reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = PtzDriveData::parse(&msgInfo);
    return 0;
}

int PtzDriveProxy::movePos(ptz_drive_move_pos_data *recv_data, ssize_t recv_datalen,
                           uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_PTZ_DRIVE_MOVE_POS, recv_data, recv_datalen,
                            reply_timeout_ns);
}

int PtzDriveProxy::moveVel(ptz_drive_move_vel_data *recv_data, ssize_t recv_datalen,
                           uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_PTZ_DRIVE_MOVE_VEL, recv_data, recv_datalen,
                            reply_timeout_ns);
}
