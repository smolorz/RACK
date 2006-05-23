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
#ifndef __TIMS_MSG_GATEWAY__
#define __TIMS_MSG_GATEWAY__

#define TIMS_MSG_GATEWAY_LOGIN                  10
#define TIMS_MSG_GATEWAY_MBX_INIT               11
#define TIMS_MSG_GATEWAY_MBX_DELETE             12
#define TIMS_MSG_GATEWAY_MBX_INIT_WITH_REPLY    13

typedef struct {
    timsMsgHead                head;
    int mbx;
} timsMsgGateway_MbxMsg;

#endif // __TIMS_MSG_GATEWAY__
