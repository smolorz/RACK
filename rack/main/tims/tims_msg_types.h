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
#ifndef _TIMS_MSG_TYPES_H_
#define _TIMS_MSG_TYPES_H_

//
// sending message types ( > 0 )
//

// TimsMsgRouter -> TcpTimsMsgRouter
#define TIMS_MSG_ROUTER_LOGIN                  10
#define TIMS_MSG_ROUTER_MBX_INIT               11 // ohne Quittung
#define TIMS_MSG_ROUTER_MBX_DELETE             12 // ohne Quittung
#define TIMS_MSG_ROUTER_MBX_INIT_WITH_REPLY    13 // Sendet TIMS_MSG_OK oder TIMS_MSG_ERROR zurueck
#define TIMS_MSG_ROUTER_MBX_DELETE_WITH_REPLY  14 // Sendet TIMS_MSG_OK oder TIMS_MSG_ERROR zurueck
#define TIMS_MSG_ROUTER_MBX_PURGE              15

// TimsMsgRouter -> TimsMsgClient
#define TIMS_MSG_ROUTER_GET_CONFIG             16

// TcpTimsMsgRouter -> TimsMsgClient
#define TIMS_MSG_ROUTER_GET_STATUS             17 // Client -> TCP Router TIMS_MSG_OK

// TimsMsgClient -> TimsMsgRouter
#define TIMS_MSG_ROUTER_CONFIG                -10
#define TIMS_MSG_ROUTER_CONNECTED             -11

//
//  return / reply message types ( <=0 )
//

#define TIMS_MSG_OK                             0
#define TIMS_MSG_ERROR                         -1
#define TIMS_MSG_TIMEOUT                       -2
#define TIMS_MSG_NOT_AVAILABLE                 -3


#endif  // _TIMS_MSG_TYPES_H_
