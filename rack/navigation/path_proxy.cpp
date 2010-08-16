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
#include <navigation/path_proxy.h>

//
// proxy functions
//

int PathProxy::getData(path_data *recv_data, ssize_t recv_datalen,
                       rack_time_t timeStamp, uint64_t reply_timeout_ns)
{
    RackMessage msgInfo;

    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen,
                                     timeStamp, reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = PathData::parse(&msgInfo);
    return 0;
}

int PathProxy::getNextData(path_data *recv_data, ssize_t recv_datalen,
                        uint64_t reply_timeout_ns)
{
    RackMessage msgInfo;

    int ret = RackDataProxy::getNextData((void *)recv_data, recv_datalen,
                                    reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = PathData::parse(&msgInfo);
    return 0;
}

int PathProxy::make(path_make_data *recv_data, ssize_t recv_datalen,
                    uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_PATH_MAKE, recv_data, recv_datalen,
                            reply_timeout_ns);
}


int PathProxy::replan(path_replan_data *recv_data, ssize_t recv_datalen,
                      uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_PATH_REPLAN, recv_data, recv_datalen,
                            reply_timeout_ns);
}


int PathProxy::setDestination(path_dest_data *recv_data, ssize_t recv_datalen,
                              uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_PATH_SET_DESTINATION, recv_data, recv_datalen,
                             reply_timeout_ns);
}


int PathProxy::setRddf(path_rddf_data *recv_data, ssize_t recv_datalen,
                       uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_PATH_SET_RDDF, recv_data, recv_datalen,
                            reply_timeout_ns);
}


// get_dbg_gridmap

    // TODO

int PathProxy::setLayer(path_layer_data *recv_data, ssize_t recv_datalen,
                        uint64_t reply_timeout_ns)
{
    return proxySendDataCmd(MSG_PATH_SET_LAYER, recv_data, recv_datalen,
                            reply_timeout_ns);
}


int PathProxy::getLayer(path_layer_data *recv_data, ssize_t recv_datalen,
                        uint64_t reply_timeout_ns)
{
    RackMessage msgInfo;

    int ret = proxyRecvDataCmd(MSG_PATH_GET_LAYER, MSG_PATH_LAYER,
                              (void *)recv_data, recv_datalen,
                              reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = PathLayerData::parse(&msgInfo);
    return 0;
}
