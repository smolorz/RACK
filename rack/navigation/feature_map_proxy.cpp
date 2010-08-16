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
 */
#include <navigation/feature_map_proxy.h>

//
// proxy functions
//

int FeatureMapProxy::getData(feature_map_data *recv_data, ssize_t recv_datalen,
                      rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    RackMessage msgInfo;

    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen,
                                     timeStamp, reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = FeatureMapData::parse(&msgInfo);
    return 0;
}

int FeatureMapProxy::addLine(feature_map_feature *recv_data, ssize_t recv_datalen,
                              uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_FEATURE_MAP_ADD_LINE, &recv_data, recv_datalen,
                            reply_timeout_ns);
}

int FeatureMapProxy::saveMap(feature_map_filename *recv_data, ssize_t recv_datalen,
                              uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_FEATURE_MAP_SAVE_MAP, &recv_data, recv_datalen,
                            reply_timeout_ns);
}

int FeatureMapProxy::deleteLine(feature_map_feature *recv_data, ssize_t recv_datalen,
                              uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_FEATURE_MAP_DELETE_LINE, &recv_data, recv_datalen,
                            reply_timeout_ns);
}

int FeatureMapProxy::displaceLine(feature_map_feature *recv_data, ssize_t recv_datalen,
                              uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_FEATURE_MAP_DISPLACE_LINE, &recv_data, recv_datalen,
                            reply_timeout_ns);
}

int FeatureMapProxy::setLayer(feature_map_layer_data *recv_data, ssize_t recv_datalen,
                              uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_FEATURE_MAP_SET_LAYER, recv_data, recv_datalen,
                             reply_timeout_ns);
}

int FeatureMapProxy::getLayer(feature_map_layer_data *recv_data, ssize_t recv_datalen,
                        uint64_t reply_timeout_ns)
{
    RackMessage msgInfo;

    int ret = proxyRecvDataCmd(MSG_FEATURE_MAP_GET_LAYER, MSG_FEATURE_MAP_LAYER,
                              (void *)recv_data, recv_datalen,
                              reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = FeatureMapLayerData::parse(&msgInfo);
    return 0;
}
