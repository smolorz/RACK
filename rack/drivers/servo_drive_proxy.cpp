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
 *      Joerg Langenberg  <joerg.langenberg@gmx.net>
 *
 */
#include <drivers/servo_drive_proxy.h>

int ServoDriveProxy::getData(servo_drive_data *recv_data, ssize_t recv_datalen,
                             rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    RackMessage msgInfo;

    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen, timeStamp,
                                     reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = ServoDriveData::parse(&msgInfo);
    return 0;
}

int ServoDriveProxy::movePos(float position, float vel, float acc,
                             uint32_t replyPositionReached,
                             int32_t replyPositionReachedId,
                             uint64_t reply_timeout_ns)
{
    servo_drive_move_pos_data send_data;
    send_data.position             = position;
    send_data.vel                  = vel;
    send_data.acc                  = acc;
    send_data.replyPositionReached = replyPositionReached;

    return proxySendDataCmd(MSG_SERVO_DRIVE_MOVE_POS, &send_data,
                            sizeof(servo_drive_move_pos_data),
                            reply_timeout_ns);
}

int ServoDriveProxy::moveVel(float vel, uint64_t reply_timeout_ns)
{
    servo_drive_move_vel_data send_data;
    send_data.vel = vel;

    return proxySendDataCmd(MSG_SERVO_DRIVE_MOVE_VEL, &send_data,
                            sizeof(servo_drive_move_vel_data),
                            reply_timeout_ns);
}
