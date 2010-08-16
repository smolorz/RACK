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
 #include <drivers/vehicle_proxy.h>

 int VehicleProxy::getData(vehicle_data *recv_data, ssize_t recv_datalen,
                        rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    RackMessage msgInfo;

    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen,
                                     timeStamp, reply_timeout_ns, &msgInfo);
    if (ret) {
        return ret;
    }

    recv_data = VehicleData::parse(&msgInfo);
    return 0;
}

int VehicleProxy::setValue(vehicle_set_value_data *recv_data, ssize_t recv_datalen,
                       uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_VEHICLE_SET_VALUE, recv_data, recv_datalen,
                            reply_timeout_ns);
}
