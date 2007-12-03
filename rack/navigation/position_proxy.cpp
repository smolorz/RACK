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
 #include <navigation/position_proxy.h>

int PositionProxy::getData(position_data *recv_data, ssize_t recv_datalen,
                           rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    message_info msgInfo;

    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen, timeStamp,
                                     reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = PositionData::parse(&msgInfo);
    return 0;
}

int PositionProxy::update(position_data *posData, uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_POSITION_UPDATE, posData,
                            sizeof(position_data), reply_timeout_ns);
}

int PositionProxy::wgs84ToPos(position_wgs84_data *wgs84Data, position_data *posData,
                                   uint64_t reply_timeout_ns)
{
    message_info msgInfo;

    int ret = proxySendRecvDataCmd(MSG_POSITION_WGS84_TO_POS, (void *)wgs84Data,
                              sizeof(position_wgs84_data),MSG_POSITION_POS,
                              (void *)posData, sizeof(position_data),
                              reply_timeout_ns, &msgInfo);

    if (ret)
    {
        return ret;
    }

    posData = PositionData::parse(&msgInfo);
    return 0;
}

int PositionProxy::posToWgs84(position_data *posData, position_wgs84_data *wgs84Data,
                                   uint64_t reply_timeout_ns)
{
    message_info msgInfo;

    int ret = proxySendRecvDataCmd(MSG_POSITION_POS_TO_WGS84, (void *)posData,
                              sizeof(position_data),MSG_POSITION_WGS84,
                              (void *)wgs84Data, sizeof(position_wgs84_data),
                              reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    wgs84Data = PositionWgs84Data::parse(&msgInfo);
    return 0;
}
