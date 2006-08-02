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
#include <drivers/joystick_proxy.h>

int JoystickProxy::getData(joystick_data *recv_data, ssize_t recv_datalen,
                           rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    MessageInfo msgInfo;
    
    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen, timeStamp,
                                     reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = JoystickData::parse(&msgInfo);
    return 0;
}
