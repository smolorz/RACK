/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Daniel Lecking <lecking@rts.uni-hannover.de>
 *
 */
#ifndef __CHASSIS_USARSIM_H__
#define __CHASSIS_USARSIM_H__

#include <main/rack_data_module.h>
#include <drivers/chassis_proxy.h>
#include <main/defines/position3d.h>
#include <drivers/ladar_proxy.h>
#include <drivers/odometry_proxy.h>
#include <navigation/position_proxy.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>

using namespace std;


// define module class
#define MODULE_CLASS_ID     CHASSIS


#define USARSIM_BUFFER      200
#define USARSIM_MAX_MSG_SIZE    18432

typedef struct
{
    ladar_data          data;
    ladar_point         point[LADAR_DATA_MAX_POINT_NUM];
} __attribute__((packed)) ladar_data_msg;

/**
 *
 * @ingroup modules_chassis
 */
class ChassisUsarsim : public RackDataModule
{
  protected:
    chassis_move_data   commandData;
    uint32_t            activePilot;
    RackMutex           mtx;
    int                 ladarRelaySys;
    int                 ladarRelayInst;
    int                 odometryRelaySys;
    int                 odometryRelayInst;
    int                 positionGndTruthRelaySys;
    int                 positionGndTruthRelayInst;
    int                 positionUpdateSys;
    int                 positionUpdateInst;
    int                 chassisInitPosX;
    int                 chassisInitPosY;
    int                 chassisInitPosZ;
    int                 chassisInitPosRho;
    int                 controlTraceState;
    int                 controlTraceColor;

    int                  tcpSocket;
    struct               sockaddr_in tcpAddr;
    char                 *usarsimIp;
    char                 *usarsimChassis;
    int                  usarsimPort;
    int                 maxBatteryState;

    char                messageData[USARSIM_MAX_MSG_SIZE];

    rack_time_t         statusMsgTime;

    string              messageStr;
    string              dataStr;
    string              valueStr;

    ladar_data_msg      ladarData;
    odometry_data       odometryData;
    position_data       groundTruthData;
    position_data       positionData;
    position_3d         chassisInitPos;


    uint32_t        ladarRelayMbxAdr;
    uint32_t        odometryRelayMbxAdr;
    uint32_t        positionGndTruthRelayMbxAdr;
    // mailboxes
    RackMailbox     workMbx;

    // proxies
    LadarProxy      *ladarRelay;
    OdometryProxy   *odometryRelay;
    PositionProxy   *positionGndTruthRelay;
    PositionProxy   *positionUpdate;
    
  protected:
    // -> realtime context
    int  moduleOn(void);
    void moduleOff(void);
    int  moduleLoop(void);
    int  moduleCommand(RackMessage *msgInfo);

    int chassisInit(char *usarsimChassis, position_3d chassisInitPos);
    int sendMoveCommand(int speed, float omega, int type);
    int searchRangeScannerData(rack_time_t currentTime);
    int searchOdometryData(rack_time_t currentTime);
    int searchGroundTruthData(rack_time_t currentTime);
    int getBatteryState();
    int getUsarsimTime();
    int controlTrace(int state, float interval, int color);

    // -> non realtime context
    void moduleCleanup(void);

  public:
    // constructor und destructor
    ChassisUsarsim();
    ~ChassisUsarsim() {};

    // -> non realtime context
    int  moduleInit(void);
};

#endif // __CHASSIS_USARSIM_H__
