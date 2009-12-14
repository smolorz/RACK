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
#include <iostream>

#include "chassis_usarsim.h"

//
// data structures
//

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE                0
#define INIT_BIT_MTX_CREATED                1
#define INIT_BIT_MBX_WORK                   2
#define INIT_BIT_PROXY_POSITION_UPDATE      3
#define INIT_BIT_PROXY_LADAR                4
#define INIT_BIT_PROXY_ODOMETRY             5
#define INIT_BIT_PROXY_POSITION             6

ChassisUsarsim *p_inst;

argTable_t argTab[] = {

    { ARGOPT_OPT, "vxMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "max vehicle velocity in x direction, default 700 m/s", { 700 } },

    { ARGOPT_OPT, "vxMin", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "min vehicle velocity in x direction, default 50 m/s)", { 50 } },

    { ARGOPT_OPT, "accMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "max vehicle acceleration, default 500", { 500 } },

    { ARGOPT_OPT, "decMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "max vehicle deceleration, default 500", { 500 } },

    { ARGOPT_OPT, "omegaMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "omegaMax, default 60 deg/s", { 60 } },

    { ARGOPT_OPT, "minTurningRadius", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "min vehicle turning radius, default 200 (mm)", { 200 } },

    { ARGOPT_OPT, "breakConstant", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "breakConstant, default 100 (1.0f)", { 100 } },

    { ARGOPT_OPT, "safetyMargin", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "safetyMargin, default 50 (mm)", { 50 } },

    { ARGOPT_OPT, "safetyMarginMove", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "safety margin in move directiom, default 200 (mm)", { 200 } },

    { ARGOPT_OPT, "comfortMargin", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "comfortMargin, default 300 (mm)", { 300 } },

    { ARGOPT_OPT, "front", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "front, default 250 (mm)", { 250 } },

    { ARGOPT_OPT, "back", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "back, default 250 (mm)", { 250 } },

    { ARGOPT_OPT, "left", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "left, default 250 (mm)", { 250 } },

    { ARGOPT_OPT, "right", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "right, default 250 (mm)", { 250 } },

    { ARGOPT_OPT, "wheelBase", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "wheel distance, default 280 (mm)", { 280 } },

    { ARGOPT_OPT, "wheelRadius", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "wheelRadius, default 110 (mm)", { 110 } },

    { ARGOPT_OPT, "trackWidth", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "trackWidth, default 280 (mm)", { 280 } },

    { ARGOPT_OPT, "pilotParameterA", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "pilotParameterA, default 10 (0.001f)", { 10 } },

    { ARGOPT_OPT, "pilotParameterB", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "pilotParameterB, default 200 (2.0f)", { 200 } },

    { ARGOPT_OPT, "pilotVTransMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "pilotVTransMax, default 200", { 200 } },

    { ARGOPT_OPT, "periodTime", ARGOPT_REQVAL, ARGOPT_VAL_INT,
        "1 / sampling rate in ms, default 200", { 200 } },

    { ARGOPT_REQ, "usarsimIp", ARGOPT_REQVAL, ARGOPT_VAL_STR,
      "Ip address of the USARSIM server", { 0 } },
      
    { ARGOPT_REQ, "usarsimChassis", ARGOPT_REQVAL, ARGOPT_VAL_STR,
      "Name of the USARSIM chassis", { 0 } },

    { ARGOPT_OPT, "usarsimPort", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Port address of the usarsim port, default '3000'", { 3000 } },

    { ARGOPT_OPT, "odometryRelaySys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the odometry relay, default 0", { 0 } },

    { ARGOPT_OPT, "odometryRelayInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the odometry relay, default 0", { 0 } },

    { ARGOPT_OPT, "positionUpdateSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the position module, default 0", { 0 } },

    { ARGOPT_OPT, "positionUpdateInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the position module, default -1", { -1 } },

    { ARGOPT_OPT, "positionGndTruthRelaySys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the position relay, default 0", { 0 } },

    { ARGOPT_OPT, "positionGndTruthRelayInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the position relay, default -1", { -1 } },
      
    { ARGOPT_OPT, "ladarRelaySys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the ladar relay, default 0", { 0 } },

    { ARGOPT_OPT, "ladarRelayInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the ladar relay, default -1", { -1 } },

    { ARGOPT_OPT, "chassisInitPosX", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The init position of the simulated robot, default 0", { 0 } },

    { ARGOPT_OPT, "chassisInitPosY", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The init position of the simulated robot, default 0", { 0 } },

    { ARGOPT_OPT, "chassisInitPosZ", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The init position of the simulated robot, default 0", { 0 } },

    { ARGOPT_OPT, "chassisInitPosRho", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The init orientation of the simulated robot in degree, default 0", { 0 } },

    { ARGOPT_OPT, "controlTraceState", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Tracing the path that the robot takes (on = 1), default 0", { 0 } },

    { ARGOPT_OPT, "controlTraceColor", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "color of the trace red 0, yellow 1 ... until 6, default 0", { 0 } },
      
    { 0, "", 0, 0, "", { 0 } } // last entry
};


// vehicle parameter

chassis_param_data param = {
    vxMax:            0,                    // mm/s
    vyMax:            0,
    vxMin:            0,                    // mm/s
    vyMin:            0,

    accMax:           0,                    // mm/s/s
    decMax:           0,

    omegaMax:         0.0,                  // rad/s
    minTurningRadius: 0,                    // mm

    breakConstant:    1.0f,                 // mm/mm/s
    safetyMargin:     50,                   // mm
    safetyMarginMove: 200,                  // mm
    comfortMargin:    300,                  // mm

    boundaryFront:    250,                  // mm
    boundaryBack:     250,                  // mm
    boundaryLeft:     250,                  // mm
    boundaryRight:    250,                  // mm

    wheelBase:        280,                  // mm
    wheelRadius:      110,                  // mm
    trackWidth:       280,

    pilotParameterA:  0.001f,
    pilotParameterB:  2.0f,
    pilotVTransMax:   200,                  // mm/s
};

/*******************************************************************************
 *   !!! REALTIME CONTEXT !!!
 *
 *   moduleOn,
 *   moduleOff,
 *   moduleLoop,
 *   moduleCommand,
 *
 *   own realtime user functions
 ******************************************************************************/

int ChassisUsarsim::moduleOn(void)
{
    int ret;

    // get dynamic module parameter
    param.vxMax             = getInt32Param("vxMax");
    param.vxMin             = getInt32Param("vxMin");
    param.accMax            = getInt32Param("accMax");
    param.decMax            = getInt32Param("decMax");
    param.omegaMax          = getInt32Param("omegaMax") * M_PI / 180.0;
    param.minTurningRadius  = getInt32Param("minTurningRadius");
    param.breakConstant     = (float)getInt32Param("breakConstant") / 100.0f;
    param.safetyMargin      = getInt32Param("safetyMargin");
    param.safetyMarginMove  = getInt32Param("safetyMarginMove");
    param.comfortMargin     = getInt32Param("comfortMargin");
    param.boundaryFront     = getInt32Param("front");
    param.boundaryBack      = getInt32Param("back");
    param.boundaryLeft      = getInt32Param("left");
    param.boundaryRight     = getInt32Param("right");
    param.wheelBase         = getInt32Param("wheelBase");
    param.wheelRadius       = getInt32Param("wheelRadius");
    param.trackWidth        = getInt32Param("trackWidth");
    param.pilotParameterA   = (float)getInt32Param("pilotParameterA") / 10000.0f;
    param.pilotParameterB   = (float)getInt32Param("pilotParameterB") / 100.0f;
    param.pilotVTransMax    = getInt32Param("pilotVTransMax");
    dataBufferPeriodTime    = getInt32Param("periodTime");
    usarsimIp               = getStringParam("usarsimIp");
    usarsimPort             = getInt32Param("usarsimPort");
    usarsimChassis          = getStringParam("usarsimChassis");
    controlTraceState       = getInt32Param("controlTraceState");
    controlTraceColor       = getInt32Param("controlTraceColor");
    
    chassisInitPos.x        = getInt32Param("chassisInitPosX");
    if (chassisInitPos.x == 0)
    {
        chassisInitPos.x = 1; // bug of USARSIM
    }
    chassisInitPos.y        = getInt32Param("chassisInitPosY");
    chassisInitPos.z        = getInt32Param("chassisInitPosZ");
    chassisInitPos.rho      = (float)getInt32Param("chassisInitPosRho") * M_PI / 180.0;

    chassisInitPos.phi      = 0;
    chassisInitPos.psi      = 0;
    
    commandData.vx    = 0;  // in mm/s
    commandData.vy    = 0;  // in mm/s
    commandData.omega = 0;  // in mm
    activePilot = CHASSIS_INVAL_PILOT;
    
    //preparing tcp Socket
    inet_pton(AF_INET, usarsimIp, &(tcpAddr.sin_addr));
    tcpAddr.sin_port = htons((unsigned short)usarsimPort);
    tcpAddr.sin_family = AF_INET;
    bzero(&(tcpAddr.sin_zero), 8);

    RackTask::disableRealtimeMode();
    
    //openning tcp Socket
    GDOS_DBG_INFO("open network socket...\n");
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == -1)
    {
            GDOS_ERROR("Can't create tcp Socket, (%d)\n",errno);
            return -errno;
    }

    //connect to tcp Socket
    GDOS_DBG_INFO("Connect to network socket\n");
    ret = connect(tcpSocket, (struct sockaddr *)&tcpAddr, sizeof(tcpAddr));
    if(ret)
    {
           GDOS_ERROR("Can't connect to tcp Socket, (%d)\n",errno);
           return    errno;
    }

    ret = chassisInit(usarsimChassis, chassisInitPos);
    if(ret)
    {
        GDOS_ERROR("Can't send init command to USARSIM server\n");
        return  ret;
    }
    
    
    //odometryRelay
    odometryData.recordingTime = rackTime.get();
 
    ret = workMbx.sendDataMsg(MSG_DATA, odometryRelayMbxAdr + 1, 1, 1,
                             &odometryData, sizeof(odometry_data));
    if (ret)
    {
        GDOS_WARNING("Error while sending first odometry data from %x to %x (bytes %d)\n",
                     workMbx.getAdr(), odometryRelayMbxAdr, sizeof(odometry_data));
        return ret;
    }

    GDOS_DBG_DETAIL("Turn on Odometry(%d/%d)\n", odometryRelaySys, odometryRelayInst);
    ret = odometryRelay->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on OdometryRelay(%i/%i), code = %d\n",
                   odometryRelaySys, odometryRelayInst, ret);
        return ret;
    }
    
/*
    if (positionUpdateInst >= 0)
    {
        ret = positionUpdate->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on positionUpdate(%i/%i), code = %d\n",
                        positionUpdateSys, positionUpdateInst, ret);
            return ret;
        }

        positionData.recordingTime = rackTime.get();
        positionData.pos = chassisInitPos;
        positionData.var.x      = 0;
        positionData.var.y      = 0;
        positionData.var.z      = 0;
        positionData.var.phi    = 0.0f;
        positionData.var.psi    = 0.0f;
        positionData.var.rho    = 0.0f;

        positionUpdate->update(&positionData);
    }
*/
    // ladar relay
    if (ladarRelayInst >= 0)
    {
        // init sonar values and send first dataMsg
        ladarData.data.recordingTime = rackTime.get();
        ladarData.data.maxRange = 30000;
        ladarData.data.duration = 200;
        ladarData.data.endAngle = 90.0 * M_PI /180.0;
        ladarData.data.pointNum      = 0;

        ret = workMbx.sendDataMsg(MSG_DATA, ladarRelayMbxAdr + 1, 1, 1,
                                  &ladarData, sizeof(ladar_data));
        if (ret)
        {
            GDOS_WARNING("Error while sending first ladar relay data from %x to %x (bytes %d)\n",
                         workMbx.getAdr(), ladarRelayMbxAdr, sizeof(ladar_data));
            return ret;
        }

        GDOS_DBG_DETAIL("Turn on Ladar(%d/%d)\n", ladarRelaySys, ladarRelayInst);
        ret = ladarRelay->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on Ladar(%i/%i), code = %d\n",
                       ladarRelaySys, ladarRelayInst, ret);
            return ret;
        }
    }
    
    if (positionGndTruthRelayInst >= 0)
    {
        //positionGroundTruthRelay
        groundTruthData.recordingTime = rackTime.get();

        ret = workMbx.sendDataMsg(MSG_DATA, positionGndTruthRelayMbxAdr + 1, 1, 1,
                                 &groundTruthData, sizeof(position_data));
        if (ret)
        {
            GDOS_WARNING("Error while sending first position ground truth data from %x to %x (bytes %d)\n",
                         workMbx.getAdr(), positionGndTruthRelayMbxAdr, sizeof(position_data));
            return ret;
        }

        GDOS_DBG_DETAIL("Turn on Position Ground Truth(%d/%d)\n",  positionGndTruthRelaySys, positionGndTruthRelayInst);
        ret = positionGndTruthRelay->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on positionGndTruthRelay(%i/%i), code = %d\n",
                       positionGndTruthRelaySys, positionGndTruthRelayInst, ret);
            return ret;
        }
    }
    
    controlTrace(controlTraceState, 0.0f, controlTraceColor);
    
    statusMsgTime = rackTime.get();

    return RackDataModule::moduleOn(); // has to be last command in moduleOn();
}

void ChassisUsarsim::moduleOff(void)
{
    RackDataModule::moduleOff();       // has to be first command in moduleOff();

    activePilot = CHASSIS_INVAL_PILOT;

    // closing tcp Socket
    RackTask::disableRealtimeMode();

    if(tcpSocket !=-1)
    {
        close(tcpSocket);
        tcpSocket = -1;
    }

    RackTask::enableRealtimeMode();
}

int ChassisUsarsim::moduleLoop(void)
{
    chassis_data* p_data = NULL;
    ssize_t datalength = 0;
    int ret;
    int currentBatteryState;
    rack_time_t currentTime;

    RackTask::disableRealtimeMode();
    
    ret = recv(tcpSocket, messageData, USARSIM_MAX_MSG_SIZE, 0);
    
    if (ret <= 0)
    {
        GDOS_ERROR("Can't get data from USARSIM server\n");
        return ret;
    }
    else if (ret >= USARSIM_MAX_MSG_SIZE)
    {
        GDOS_ERROR("USARSim message exceeds buffer size !\n");
    }
    
    messageStr = messageData;

    if (messageStr.find("SEN") != string::npos)
    {
        ret = searchOdometryData();
        if (ret)
        {
            GDOS_ERROR("Can't receive odometry data\n");
            return ret;
        }

        if (ladarRelayInst >= 0)
        {
            ret = searchRangeScannerData();
            if (ret)
            {
                GDOS_ERROR("Can't receive range scanner data\n");
                //return ret;
            }
        }

        if (positionGndTruthRelayInst >= 0)
        {
            ret = searchGroundTruthData();
            if (ret)
            {
                GDOS_ERROR("Can't receive ground truth data\n");
                return ret;
            }
        }
    }

    if (messageStr.find("STA") != string::npos)
    {
        // get datapointer from rackdatabuffer
        p_data = (chassis_data *)getDataBufferWorkSpace();
        
        statusMsgTime = rackTime.get();
    
        currentBatteryState = getBatteryState();
        if (currentBatteryState < 0)
        {
            GDOS_ERROR("Can't get battery state\n");
        }
        else if (currentBatteryState > maxBatteryState)
        {
            maxBatteryState = currentBatteryState;
        }
    
        mtx.lock(RACK_INFINITE);

        p_data->recordingTime = statusMsgTime;
        p_data->vx            = (float)commandData.vx;    // in mm/s
        p_data->vy            = 0.0f;
        p_data->omega         = (float)commandData.omega; // in rad/s
        p_data->deltaX        = p_data->vx * (float)dataBufferPeriodTime / 1000.0f;       // in mm
        p_data->deltaY        = 0.0f;
        p_data->deltaRho      = p_data->omega * (float)dataBufferPeriodTime / 1000.0f;    // in rad
        p_data->battery       = (float)(int)(currentBatteryState * 100 / maxBatteryState);
        p_data->activePilot   = activePilot;

        mtx.unlock();

        ret  = sendMoveCommand(commandData.vx, commandData.omega, 0);
        if(ret)
        {
            GDOS_ERROR("Can't send drive command to USARSIM server\n");
            return  ret;
        }

        datalength = sizeof(chassis_data);

        putDataBufferWorkSpace( datalength );


        GDOS_DBG_DETAIL("vx:%f mm/s, vx:%f mm/s, omega:%a deg/s, timestamp: %d\n",
                        p_data->vx, p_data->vy, p_data->omega,
                        p_data->recordingTime);
    }

    currentTime = rackTime.get();
    if ( (currentTime - statusMsgTime) > (dataBufferPeriodTime * 5) )
    {
        GDOS_ERROR("Can't get status message from usarsim server\n");
        return -1;
    }
    
    return 0;
}

int ChassisUsarsim::moduleCommand(message_info *msgInfo)
{
    unsigned int pilot_mask = RackName::getSysMask()   |
                              RackName::getClassMask() |
                              RackName::getInstMask();
    chassis_move_data               *p_move;
    chassis_set_active_pilot_data   *p_pilot;

    switch (msgInfo->type)
    {
        case MSG_CHASSIS_MOVE:
            if (status != MODULE_STATE_ENABLED)
            {
                cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
                break;
            }

            p_move = ChassisMoveData::parse(msgInfo);
            if ((msgInfo->src & pilot_mask) != activePilot)
            {
                cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
                break;
            }

            mtx.lock(RACK_INFINITE);

            // set min speed
            if ((p_move->vx > 0) && (p_move->vx < param.vxMin))
                p_move->vx = param.vxMin;
            if ((p_move->vx < 0) && (p_move->vx > -param.vxMin))
                p_move->vx = -param.vxMin;
            if ((p_move->vy > 0) && (p_move->vy < param.vyMin))
                p_move->vy = param.vyMin;
            if ((p_move->vy < 0) && (p_move->vy > -param.vyMin))
                p_move->vy = -param.vyMin;

            memcpy(&commandData, p_move, sizeof(commandData));

            mtx.unlock();

            cmdMbx.sendMsgReply(MSG_OK, msgInfo);
            break;

        case MSG_CHASSIS_GET_PARAMETER:
            cmdMbx.sendDataMsgReply(MSG_CHASSIS_PARAMETER, msgInfo, 1, &param,
                                    sizeof(chassis_param_data));
            break;

        case MSG_CHASSIS_SET_ACTIVE_PILOT:
            p_pilot = ChassisSetActivePilotData::parse(msgInfo);

            mtx.lock(RACK_INFINITE);

            activePilot = p_pilot->activePilot & pilot_mask;

            commandData.vx    = 0;
            commandData.vy    = 0;
            commandData.omega = 0;

            mtx.unlock();

            GDOS_PRINT("%n changed active pilot to %n", msgInfo->src, activePilot);
            cmdMbx.sendMsgReply(MSG_OK, msgInfo);
            break;

        default:
            // not for me -> ask RackDataModule
            return RackDataModule::moduleCommand(msgInfo);
        }
    return 0;
}

int ChassisUsarsim::chassisInit(char *usarsimChassis, position_3d chassisInitPos)
{
    char buffer[USARSIM_BUFFER];
    int ret, strLen;
    
    strLen = snprintf(buffer, USARSIM_BUFFER, "INIT {ClassName USARBot.%s} {Location %.3f,%.3f,%.3f} {Rotation %.3f,%.3f,%.3f} \r\n",
                usarsimChassis,(double)chassisInitPos.x /1000.0,(double)chassisInitPos.y /1000.0,(double)chassisInitPos.z /1000.0,
                chassisInitPos.psi,chassisInitPos.phi,chassisInitPos.rho);

    if (strLen >= USARSIM_BUFFER)
    {
        GDOS_ERROR("Can't create INIT message. Message exceeds buffer size");
        return -1;
    }
    
    ret = send(tcpSocket, buffer, strLen, 0);
    if (ret < 0)
    {
        GDOS_ERROR("Error sending data, (%ret)",ret);
        return ret;
    }
    return 0;
}


int ChassisUsarsim::sendMoveCommand(int speed, float omega, int type)
{
    char buffer[USARSIM_BUFFER];
    int ret, strLen;
    double leftSpeed, rightSpeed;

    mtx.lock(RACK_INFINITE);
    
    switch (type)
    {
        case 0:
    
            leftSpeed   = 100.0 *  (float) speed / param.vxMax;
            rightSpeed  = 100.0 *  (float) speed / param.vxMax;

            leftSpeed  += 100.0 * (omega *(float)param.trackWidth / param.vxMax);
            rightSpeed -= 100.0 * (omega *(float)param.trackWidth / param.vxMax);

            strLen = snprintf(buffer, USARSIM_BUFFER, "DRIVE {Left %f} {Right %f} {Normalized true} \r\n", leftSpeed, rightSpeed);

            break;
        default:
            GDOS_ERROR("Drive command not implemented !");
            return -1;
    }

    mtx.unlock();

    if (strLen >= USARSIM_BUFFER)
    {
        GDOS_ERROR("Can't create DRIVE message. Message exceeds buffer size");
        return -1;
    }

    ret = send(tcpSocket, buffer, strLen, 0);
    if (ret < 0)
    {
        GDOS_ERROR("Error sending data, (%ret)",ret);
        return ret;
    }
    return 0;
}

int ChassisUsarsim::controlTrace(int state, float interval, int color)
{
    char buffer[USARSIM_BUFFER];
    int ret, strLen;

    strLen = snprintf(buffer, USARSIM_BUFFER, "Trace {On %i} {Interval %f} {Color %i} \r\n", state, interval, color);
    if (strLen >= USARSIM_BUFFER)
    {
        GDOS_ERROR("Can't create trace message.");
        return -1;
    }

    ret = send(tcpSocket, buffer, strLen, 0);
    if (ret < 0)
    {
        GDOS_ERROR("Error sending data, (%ret)",ret);
        return ret;
    }
    return 0;
}

int ChassisUsarsim::searchRangeScannerData()
{
    size_t magicWordPos, startPos, endPos;
    int ret;
    dataStr.clear();
    valueStr.clear();

    magicWordPos = messageStr.find("{Type RangeScanner");
    
    if (magicWordPos != string::npos)
    {
        magicWordPos = messageStr.find("{Range", magicWordPos);
        if (magicWordPos != string::npos)
        {
            startPos = magicWordPos + sizeof("{Range");
            endPos = messageStr.find("}", startPos);
            if (endPos != string::npos)
            {
                dataStr = messageStr.substr(startPos, endPos - startPos);
                startPos = 0;
                endPos = 0;
                ladarData.data.pointNum = 0;

                while ((startPos != string::npos) && (endPos != string::npos))
                {
                    endPos = dataStr.find(',', startPos);
                    if (endPos != string::npos)
                    {
                        valueStr = dataStr.substr(startPos, endPos - startPos);
                        startPos = endPos + 1;
                    }
                    else
                    {
                        valueStr = dataStr.substr(startPos);
                    }
                    ladarData.data.point[ladarData.data.pointNum].distance = (int)(atof(valueStr.c_str()) * 1000.0);
                    ladarData.data.point[ladarData.data.pointNum].angle    =
                                                ladarData.data.endAngle - (0.01745f * (float)ladarData.data.pointNum);
                    ladarData.data.point[ladarData.data.pointNum].type = LADAR_POINT_TYPE_UNKNOWN;
                    ladarData.data.pointNum++;
                }
                ladarData.data.startAngle = ladarData.data.endAngle - (0.01745f * (float)ladarData.data.pointNum);

                GDOS_DBG_DETAIL("ladarData.data.pointNum = %i",ladarData.data.pointNum);

                ret = workMbx.sendDataMsg(MSG_DATA, ladarRelayMbxAdr + 1, 1, 1,
                                         &ladarData, sizeof(ladar_data) + ladarData.data.pointNum * sizeof(ladar_point));
                if (ret)
                {
                    GDOS_ERROR("Error while sending ladarData data from %x to %x, error code %i\n LadarDataPointNum %i\n",
                               workMbx.getAdr(), ladarRelayMbxAdr, ret, ladarData.data.pointNum );
                    return ret;
                }
            }
            else
            {
                GDOS_ERROR("Can't receive Range Scanner Data. Incomplete message");
                return -1;
            }
        }
        else
        {
            GDOS_ERROR("Can't receive Range Scanner Data. Incomplete message");
            return -1;
        }
    }
    return 0;
}

int ChassisUsarsim::searchOdometryData()
{
    size_t magicWordPos, startPos, endPos;
    int ret;
    int dataNum;
    dataStr.clear();
    valueStr.clear();

    magicWordPos = messageStr.find("{Type Odometry");

    if (magicWordPos != string::npos)
    {
        magicWordPos = messageStr.find("{Pose", magicWordPos);
        if (magicWordPos != string::npos)
        {
            startPos = magicWordPos + sizeof("{Pose");
            endPos = messageStr.find("}", startPos);
            if (endPos != string::npos)
            {
                dataStr = messageStr.substr(startPos, endPos - startPos);
                startPos = 0;
                endPos = 0;
                dataNum = 0;

                while ((startPos != string::npos) && (endPos != string::npos))
                {
                    endPos = dataStr.find(',', startPos);
                    if (endPos != string::npos)
                    {
                        valueStr = dataStr.substr(startPos, endPos - startPos);
                        startPos = endPos + 1;
                    }
                    else
                    {
                        valueStr = dataStr.substr(startPos);
                    }
                    
                    switch (dataNum)
                    {
                        case 0:
                                odometryData.pos.x = (int)(atof(valueStr.c_str()) * 1000.0);
                            break;

                        case 1:
                                odometryData.pos.y = (int)(atof(valueStr.c_str()) * 1000.0);
                            break;

                        case 2:
                                odometryData.pos.rho = atof(valueStr.c_str());
                            break;
                        default:
                                GDOS_ERROR("Received odometry message doesn't match data format !");
                    }
                    dataNum++;
                }
                //odometryRelay
                odometryData.recordingTime = rackTime.get();

                ret = workMbx.sendDataMsg(MSG_DATA, odometryRelayMbxAdr + 1, 1, 1,
                                 &odometryData, sizeof(odometry_data));
                if (ret)
                {
                    GDOS_WARNING("Error while sending odometry data from %x to %x (bytes %d), %i\n",
                        workMbx.getAdr(), odometryRelayMbxAdr, sizeof(odometry_data), ret);
                    return ret;
                }
            }
            else
            {
                GDOS_ERROR("Can't receive odometry data. Incomplete message");
                return -1;
            }
        }
        else
        {
            GDOS_ERROR("Can't receive odometry data. Incomplete message");
            return -1;
        }
    }
    return 0;
}

int ChassisUsarsim::searchGroundTruthData()
{
    size_t magicWordPos, startPos, endPos;
    int ret;
    int dataNum;
    dataStr.clear();
    valueStr.clear();

    magicWordPos = messageStr.find("{Type GroundTruth");

    if (magicWordPos != string::npos)
    {
        magicWordPos = messageStr.find("{Location", magicWordPos);
        if (magicWordPos != string::npos)
        {
            startPos = magicWordPos + sizeof("{Location");
            endPos = messageStr.find("}", startPos);
            if (endPos != string::npos)
            {
                dataStr = messageStr.substr(startPos, endPos - startPos);
                startPos = 0;
                endPos = 0;
                dataNum = 0;

                while ((startPos != string::npos) && (endPos != string::npos))
                {
                    endPos = dataStr.find(',', startPos);
                    if (endPos != string::npos)
                    {
                        valueStr = dataStr.substr(startPos, endPos - startPos);
                        startPos = endPos + 1;
                    }
                    else
                    {
                        valueStr = dataStr.substr(startPos);
                    }

                    switch (dataNum)
                    {
                        case 0:
                                groundTruthData.pos.x = (int)(atof(valueStr.c_str()) * 1000.0);
                            break;

                        case 1:
                                groundTruthData.pos.y = (int)(atof(valueStr.c_str()) * 1000.0);
                            break;

                        case 2:
                                groundTruthData.pos.z = (int)(atof(valueStr.c_str()) * 1000.0);
                            break;
                        default:
                                GDOS_ERROR("Received ground truth doesn't match data format !");
                    }
                    dataNum++;
                }
            }
            else
            {
                GDOS_ERROR("Can't receive ground truth data. Incomplete message");
                return -1;
            }
        }
        magicWordPos = messageStr.find("{Orientation", magicWordPos);
        if (magicWordPos != string::npos)
        {
            startPos = magicWordPos + sizeof("{Orientation");
            endPos = messageStr.find("}", startPos);
            if (endPos != string::npos)
            {
                dataStr = messageStr.substr(startPos, endPos - startPos);
                startPos = 0;
                endPos = 0;
                dataNum = 0;

                while ((startPos != string::npos) && (endPos != string::npos))
                {
                    endPos = dataStr.find(',', startPos);
                    if (endPos != string::npos)
                    {
                        valueStr = dataStr.substr(startPos, endPos - startPos);
                        startPos = endPos + 1;
                    }
                    else
                    {
                        valueStr = dataStr.substr(startPos);
                    }

                    switch (dataNum)
                    {
                        case 0:
                                groundTruthData.pos.psi = atof(valueStr.c_str());
                            break;

                        case 1:
                                groundTruthData.pos.rho = atof(valueStr.c_str());
                            break;

                        case 2:
                                groundTruthData.pos.phi = atof(valueStr.c_str());
                            break;
                        default:
                                GDOS_ERROR("Received ground truth message doesn't match data format !");
                    }
                    dataNum++;
                }
            }
            else
            {
                GDOS_ERROR("Can't receive ground truth data. Incomplete message");
                return -1;
            }
        }
        else
        {
            GDOS_ERROR("Can't ground truth data. Incomplete message");
            return -1;
        }

        //positionRelay
        groundTruthData.recordingTime = rackTime.get();

        ret = workMbx.sendDataMsg(MSG_DATA, positionGndTruthRelayMbxAdr + 1, 1, 1,
                         &groundTruthData, sizeof(position_data));
        if (ret)
        {
            GDOS_WARNING("Error while sending ground truth data from %x to %x (bytes %d), errno %i\n",
                workMbx.getAdr(), positionGndTruthRelayMbxAdr, sizeof(position_data), ret);
            return ret;
        }
    }
    return 0;
}

int ChassisUsarsim::getBatteryState()
{
    size_t magicWordPos, startPos, endPos;
    valueStr.clear();

    magicWordPos = messageStr.find("{Battery");

    if (magicWordPos != string::npos)
    {
        startPos = magicWordPos + sizeof("{Battery");
        endPos = messageStr.find("}", startPos);
        if (endPos != string::npos)
        {
            valueStr = messageStr.substr(startPos, endPos - startPos);

            return (atoi(valueStr.c_str()));
        }
    }

    GDOS_ERROR("Can't receive battery state. Incomplete message");

    return -1;
}

int ChassisUsarsim::getUsarsimTime()
{
    size_t magicWordPos, startPos, endPos;
    valueStr.clear();

    magicWordPos = messageStr.find("{Time");

    if (magicWordPos != string::npos)
    {
        startPos = magicWordPos + sizeof("{Time");
        endPos = messageStr.find("}", startPos);
        if (endPos != string::npos)
        {
            valueStr = messageStr.substr(startPos, endPos - startPos);

            return ((int)(atof(valueStr.c_str()) * 1000.0));
        }
    }

    GDOS_ERROR("Can't receive Usarsim time. Incomplete message");

    return -1;
}

/*******************************************************************************
 *   !!! NON REALTIME CONTEXT !!!
 *
 *   moduleInit,
 *   moduleCleanup,
 *   Constructor,
 *   Destructor,
 *   main,
 *
 *   own non realtime user functions
 ******************************************************************************/

int ChassisUsarsim::moduleInit(void)
{
    int ret;

    // call RackDataModule init function
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);
    
    ladarRelaySys               = getInt32Param("ladarRelaySys");
    ladarRelayInst              = getInt32Param("ladarRelayInst");
    odometryRelaySys            = getInt32Param("odometryRelaySys");
    odometryRelayInst           = getInt32Param("odometryRelayInst");
    positionGndTruthRelaySys    = getInt32Param("positionGndTruthRelaySys");
    positionGndTruthRelayInst   = getInt32Param("positionGndTruthRelayInst");
    positionUpdateSys           = getInt32Param("positionUpdateSys");
    positionUpdateInst          = getInt32Param("positionUpdateInst");

    // create mutex
    ret = mtx.create();
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MTX_CREATED);
    
    // create mailbox
    ret = createMbx(&workMbx, 10, sizeof(ladar_data_msg),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // create positionUpdate proxy
    if (positionUpdateInst >= 0)
    {
        // position
        positionUpdate = new PositionProxy(&workMbx, positionUpdateSys, positionUpdateInst);
        if (!positionUpdate)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_POSITION_UPDATE);
    }

    // create ladarSonar proxy
    if (ladarRelayInst >= 0)
    {
        ladarRelayMbxAdr = RackName::create(ladarRelaySys, LADAR, ladarRelayInst);
        ladarRelay       = new LadarProxy(&workMbx, ladarRelaySys, ladarRelayInst);
        if (!ladarRelay)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_LADAR);
    }

    odometryRelayMbxAdr = RackName::create(odometryRelaySys, ODOMETRY, odometryRelayInst);
    odometryRelay       = new OdometryProxy(&workMbx, odometryRelaySys, odometryRelayInst);
    if (!odometryRelay)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_ODOMETRY);

    if (positionGndTruthRelayInst >= 0)
    {
        positionGndTruthRelayMbxAdr = RackName::create(positionGndTruthRelaySys, POSITION, positionGndTruthRelayInst);
        positionGndTruthRelay       = new PositionProxy(&workMbx, positionGndTruthRelaySys, positionGndTruthRelayInst);
        if (!positionGndTruthRelay)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_POSITION);
    }
    
    return 0;

init_error:
    // !!! call local cleanup function !!!
    ChassisUsarsim::moduleCleanup();
    return ret;
}

void ChassisUsarsim::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    if (positionGndTruthRelayInst >= 0)
    {
        if (initBits.testAndClearBit(INIT_BIT_PROXY_POSITION))
        {
            delete positionGndTruthRelay;
        }
    }
    
    if (initBits.testAndClearBit(INIT_BIT_PROXY_ODOMETRY))
    {
        delete odometryRelay;
    }

    // destroy ladarRelay proxy
    if (ladarRelayInst >= 0)
    {
        if (initBits.testAndClearBit(INIT_BIT_PROXY_LADAR))
        {
            delete ladarRelay;
        }
    }

    // free position proxy
    if (positionUpdateInst >= 0)
    {
        if (initBits.testAndClearBit(INIT_BIT_PROXY_POSITION_UPDATE))
        {
            delete positionUpdate;
        }
    }
    
    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }
    
    // destroy mutex
    if (initBits.testAndClearBit(INIT_BIT_MTX_CREATED))
    {
        mtx.destroy();
    }
}

ChassisUsarsim::ChassisUsarsim()
        : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    240,              // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    10,               // max buffer entries
                    10)               // data buffer listener
{
    // get static module parameter
    param.vxMax             = getIntArg("vxMax", argTab);
    param.vxMin             = getIntArg("vxMin", argTab);
    param.accMax            = getIntArg("accMax", argTab);
    param.decMax            = getIntArg("decMax", argTab);
    param.omegaMax          = getIntArg("omegaMax", argTab) * M_PI / 180.0;
    param.minTurningRadius  = getIntArg("minTurningRadius", argTab);
    param.breakConstant     = (float)getIntArg("breakConstant", argTab) / 100.0f;
    param.safetyMargin      = getIntArg("safetyMargin", argTab);
    param.safetyMarginMove  = getIntArg("safetyMarginMove", argTab);
    param.comfortMargin     = getIntArg("comfortMargin", argTab);
    param.boundaryFront     = getIntArg("front", argTab);
    param.boundaryBack      = getIntArg("back", argTab);
    param.boundaryLeft      = getIntArg("left", argTab);
    param.boundaryRight     = getIntArg("right", argTab);
    param.wheelBase         = getIntArg("wheelBase", argTab);
    param.wheelRadius       = getIntArg("wheelRadius", argTab);
    param.trackWidth        = getIntArg("trackWidth", argTab);
    param.pilotParameterA   = (float)getIntArg("pilotParameterA", argTab) / 10000.0f;
    param.pilotParameterB   = (float)getIntArg("pilotParameterB", argTab) / 100.0f;
    param.pilotVTransMax    = getIntArg("pilotVTransMax", argTab);
    dataBufferMaxDataSize   = sizeof(chassis_data);
}

int main(int argc, char *argv[])
{
    int ret;


    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "ChassisUsarsim");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new ChassisUsarsim

    p_inst = new ChassisUsarsim();
    if (!p_inst)
    {
        printf("Can't create new ChassisUsarsim -> EXIT\n");
        return -ENOMEM;
    }

    // init

    ret = p_inst->moduleInit();
    if (ret)
        goto exit_error;

    p_inst->run();

    return 0;

exit_error:

    delete (p_inst);

    return ret;
}

