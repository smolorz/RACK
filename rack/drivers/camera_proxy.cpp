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
 *      Marko Reimer     <reimer@l3s.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#include <drivers/camera_proxy.h>

//
// proxy functions
//

int CameraProxy::getData(camera_data *recv_data, ssize_t recv_datalen,
                          RACK_TIME timeStamp, uint64_t reply_timeout_ns)
{
    MessageInfo msgInfo;
    
    int ret = RackDataProxy::getData((void *)recv_data, recv_datalen, timeStamp,
                                    reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = CameraData::parse(&msgInfo);
    return 0;
}

int CameraProxy::getParam(camera_param_data *recv_data, ssize_t recv_datalen,
                           uint64_t reply_timeout_ns)
{
    MessageInfo msgInfo;
    
    int ret = proxyRecvDataCmd(MSG_CAMERA_GET_PARAMETER, MSG_CAMERA_PARAMETER,
                              (void *)recv_data, recv_datalen,
                              reply_timeout_ns, &msgInfo);
    if (ret)
    {
        return ret;
    }

    recv_data = CameraParamData::parse(&msgInfo);
    return 0;
}
