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
 *      Oliver Wulf  <oliver.wulf@gmx.de>
 *
 */
#include <perception/scan3d_proxy.h>

int Scan3dProxy::getData(scan3d_data *recv_data, ssize_t recv_datalen,
                         rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    RackMessage msgInfo;

    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen, timeStamp,
                                     reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = Scan3dData::parse(&msgInfo);
    return 0;
}

int Scan3dProxy::getNextData(scan3d_data *recv_data, ssize_t recv_datalen,
                             uint64_t reply_timeout_ns)
{
    RackMessage msgInfo;

    int ret = RackDataProxy::getNextData((void *)recv_data, recv_datalen,
                                         reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = Scan3dData::parse(&msgInfo);
    return 0;
}

int Scan3dProxy::getRangeImage(scan3d_range_img_data *recv_data,
                               ssize_t recv_datalen, uint64_t reply_timeout_ns)
{
    RackMessage msgInfo;

    int ret = proxyRecvDataCmd(MSG_SCAN3D_GET_RANGE_IMAGE,
                               MSG_SCAN3D_RANGE_IMAGE, recv_data,
                               recv_datalen, reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = Scan3dRangeImgData::parse(&msgInfo);
    return 0;
}
