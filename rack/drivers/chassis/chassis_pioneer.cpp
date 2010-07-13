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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
#include <iostream>

#include "chassis_pioneer.h"

//
// data structures
//

#define MAX_SIP_PACKAGE_SIZE 208

#define CALIBRATION_ROT 0.66
#define TICKS_PER_MM    12426.34 //durch messung. bei 3bar reifendruck rechnerisch: 8700000.0 / (208.0 * M_PI) ;

const unsigned char sync0Command[]     = {0xFA, 0xFB, 3, 0, 0x00, 0x00};
const unsigned char sync1Command[]     = {0xFA, 0xFB, 3, 1, 0x00, 0x01};
const unsigned char sync2Command[]     = {0xFA, 0xFB, 3, 2, 0x00, 0x02};

const unsigned char openCommand[]      = {0xFA, 0xFB, 3, 1, 0x00, 0x01};
const unsigned char closeCommand[]     = {0xFA, 0xFB, 3, 2, 0x00, 0x02};
const unsigned char motorEnable[]      = {0xFA, 0xFB, 6, 4, 0x3b, 0x01,
                                          0x00, 0x05, 0x3b};
const unsigned char motorDisable[]     = {0xFA, 0xFB, 6, 4, 0x3b, 0x00, 0x00,
                                          0x04, 0x3b};

const unsigned char pulseCommand[]     = {0xFA, 0xFB, 3, 0, 0x00, 0x00};

const unsigned char encoderOnCommand[] = {0xFA, 0xFB, 6, 19, 0x3b, 0x02, 0x00,
                                          0x15, 0x3b};

const unsigned char sonarOffCommand[]  = {0xFA, 0xFB, 6, 28, 0x3b, 0x00, 0x00,
                                          0x1C, 0x3b};

arg_table_t argTab[] = {

    { ARGOPT_REQ, "serialDev", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Serial device number", { -1 } },

    { ARGOPT_REQ, "sonar", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "disable sonar = 0; enable sonar = 1", { 0 } },

    { ARGOPT_OPT, "ladarSonarSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the ladar relay for sonar data, default 0", { 0 } },

    { ARGOPT_OPT, "ladarSonarInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the ladar relay for sonar data, default -1", { -1 } },

    { ARGOPT_OPT, "vxMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "max vehicle velocity in x direction, default 700 m/s", { 700 } },

    { ARGOPT_OPT, "vxMin", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "min vehicle velocity in x direction, default 50 m/s)", { 50 } },

    { ARGOPT_OPT, "accMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "max vehicle acceleration, default 500", { 500 } },

    { ARGOPT_OPT, "decMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "max vehicle deceleration, default 500", { 500 } },

    { ARGOPT_OPT, "omegaMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "omegaMax, default 30 deg/s", { 30 } },

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

    { 0, "", 0, 0, "", { 0 } } // last entry
};

const struct rtser_config pioneer_serial_config = {
    config_mask       : 0xFFFF,
    baud_rate         : 9600,
    parity            : RTSER_NO_PARITY,
    data_bits         : RTSER_8_BITS,
    stop_bits         : RTSER_1_STOPB,
    handshake         : RTSER_DEF_HAND,
    fifo_depth        : RTSER_DEF_FIFO_DEPTH,
    rx_timeout        : RTSER_DEF_TIMEOUT,
    tx_timeout        : RTSER_DEF_TIMEOUT,
    event_timeout     : RTSER_DEF_TIMEOUT,
    timestamp_history : RTSER_RX_TIMESTAMP_HISTORY,
    event_mask        : RTSER_EVENT_RXPEND
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

 int ChassisPioneer::moduleOn(void)
{
    unsigned char buffer[MAX_SIP_PACKAGE_SIZE];
    int totalCount;
    int ret;

    // get dynamic module parameter
    sonar                   = getInt32Param("sonar");
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

    serialPort.clean();
    serialPort.setRecvTimeout(200000000llu);

    // check if server connection is already open
    if (receivePackage(buffer, NULL) == 0)
    {
        // check if received package id standard SIP or encoder SIP
        if (((buffer[3] & 0xF0) == 0x30) | (buffer[3] == 0x90))
        {   //motor disable
            ret = sendPackage(motorDisable, sizeof(motorDisable));
            if (ret)
            {
                GDOS_ERROR("Can't send motor disable command\n");
                return ret;
            }
        }
        else
        {
            GDOS_ERROR("Received unexpected SIP\n");
            return -EIO;
        }
    }
    else  // open new connection to server
    {
        serialPort.setRecvTimeout(1000000000llu);
        //sync0
        ret = sendPackage(sync0Command, sizeof(sync0Command));
        if (ret)
        {
            GDOS_WARNING("Can't send sync0 to serial dev %i\n", serialDev);
            return ret;
        }

        totalCount = 0;
        do
        {
            ret = receivePackage(buffer, NULL);
            if (ret)
            {
                GDOS_WARNING("No response on sync0\n");
                //         return ret;
                break;
            }
            if (++totalCount > 10)
            {
                GDOS_WARNING("No propper response on sync0\n");
                //         return -ETIME;
                break;
            }
        }
        while (memcmp(buffer, sync0Command, sizeof(sync0Command)) != 0);

        //sync1
        ret = sendPackage(sync1Command, sizeof(sync1Command));
        if (ret)
        {
            GDOS_WARNING("Can't send sync1\n");
            return ret;
        }

        totalCount = 0;
        do
        {
            ret = receivePackage(buffer, NULL);
            if (ret)
            {
                GDOS_WARNING("No response on sync1\n");
                //         return ret;
                break;
            }
            if (++totalCount > 10)
            {
                GDOS_WARNING("No propper response on sync1\n");
                //         return -ETIME;
                break;
            }
        }
        while (memcmp(buffer, sync1Command, sizeof(sync1Command)) != 0);

        //sync2
        ret = sendPackage(sync2Command, sizeof(sync2Command));
        if (ret)
        {
            GDOS_WARNING("Can't send sync2 to serial dev %i\n", serialDev);
            return ret;
        }

        totalCount = 0;
        do
        {
            ret = receivePackage(buffer, NULL);
            if (ret)
            {
                GDOS_WARNING("No response on sync2\n");
                //         return ret;
                break;
            }
            if (++totalCount > 10)
            {
                GDOS_WARNING("No propper response on sync2\n");
                //         return -ETIME;
                break;
            }
        }
        while (memcmp(buffer, sync2Command, sizeof(sync2Command)) != 0);

        RackTask::sleep(200000000llu);

        //open
        ret = sendPackage(openCommand, sizeof(openCommand));
        if (ret)
        {
            GDOS_ERROR("Can't send open command\n");
            return ret;
        }
    }

    RackTask::sleep(200000000llu);

    //sonar off
    if (sonar == 0)
    {
        ret = sendPackage(sonarOffCommand, sizeof(sonarOffCommand));
        if (ret)
        {
            GDOS_ERROR("Can't send sonar off command\n");
            return ret;
        }
    }

    // sonar on
    else
    {
        // ladar relay
        if (ladarSonarInst >= 0)
        {
            // init sonar values and send first dataMsg
            sonarData.data.recordingTime = rackTime.get();
            sonarData.data.pointNum      = 0;

            ret = workMbx.sendDataMsg(MSG_DATA, ladarSonarMbxAdr + 1, 1, 1,
                                      &sonarData, sizeof(ladar_data));
            if (ret)
            {
                GDOS_WARNING("Error while sending first sonar data from %x to %x (bytes %d)\n",
                             workMbx.getAdr(), ladarSonarMbxAdr, sizeof(ladar_data));
            }

            GDOS_DBG_DETAIL("Turn on Ladar(%d/%d)\n", ladarSonarSys, ladarSonarInst);
            ret = ladarSonar->on();
            if (ret)
            {
                GDOS_ERROR("Can't turn on Ladar(%i/%i), code = %d\n",
                           ladarSonarSys, ladarSonarInst, ret);
                return ret;
            }
        }
    }

    RackTask::sleep(200000000llu);

    //pulse
    ret = sendPackage(pulseCommand, sizeof(pulseCommand));
    if (ret)
    {
        GDOS_ERROR("Can't send pulse command\n");
        return ret;
    }

    RackTask::sleep(200000000llu);

    //encoder on
    ret = sendPackage(encoderOnCommand, sizeof(encoderOnCommand));
    if (ret)
    {
        GDOS_ERROR("Can't send encoder on command\n");
        return ret;
    }

    watchdogCounter = 0;
    activePilot     = CHASSIS_INVAL_PILOT;
    leftEncoderOld  = 0x7fffffff;
    rightEncoderOld = 0x7fffffff;

    ret = sendPackage(motorEnable, sizeof(motorEnable));
    if (ret)
    {
        GDOS_WARNING("Can't enable motor\n");
        return ret;
    }

    serialPort.setRecvTimeout(200000000llu);

    return RackDataModule::moduleOn();  // has to be last command in moduleOn();
}

// realtime context
void ChassisPioneer::moduleOff(void)
{
    RackDataModule::moduleOff();        // has to be first command in moduleOff();

    activePilot = CHASSIS_INVAL_PILOT;

    sendPackage(motorDisable, sizeof(motorDisable));
    sendPackage(closeCommand, sizeof(closeCommand));
}

// realtime context
int ChassisPioneer::moduleLoop(void)
{
    chassis_data*   p_data = NULL;
    ssize_t         datalength = 0;
    unsigned char   buffer[MAX_SIP_PACKAGE_SIZE];
    rack_time_t     time;
    float           deltaT, vL, vR;
    int             ret, i;
    int             leftEncoderNew, rightEncoderNew;
    int             sonarsChanged;
    int             sonarNum, sonarValue;

    // get datapointer from rackdatabuffer
    p_data = (chassis_data *)getDataBufferWorkSpace();

    ret = receivePackage(buffer, &time);
    if (ret)
    {
        GDOS_ERROR("Can't receive SIP package\n");
        return ret;
    }

    if ((buffer[3] & 0xF0) == 0x30)  //standard SIP
    {
        /*  vL = (float)((signed short)( ((buffer[11] << 8) & 0xff00) | buffer[10]));
          vR = (float)((signed short)( ((buffer[13] << 8) & 0xff00) | buffer[12]));
          vL = vL * 1.1;
          vL = vR * 1.1;
          vTrans  = (vL + vR) / 2.0;
          vRot    = (vL - vR) / 2.0 / 200.0;*/

        battery = (float)buffer[14] / 10.0;

        // sonar data
        sonarsChanged = (unsigned int) buffer[CHASSIS_PIONEER_SIP_SONAR_NR_OFFSET]; // p2opman.pdf sonarReadings SIP
        if (sonarsChanged > 0)
        {
            GDOS_PRINT("sonarsChanged %d\n", sonarsChanged);
            for (i = 0; i <= sonarsChanged; i++)
            {
                sonarNum   = buffer[CHASSIS_PIONEER_SIP_SONAR_VALUE_OFFSET + 3*i];   //sonarnumber SIP
                sonarValue =0.268*(          // 0.0268 for centimeters
                         (buffer[CHASSIS_PIONEER_SIP_SONAR_VALUE_OFFSET+1+3*i])+     //low byte Sonarrange SIP
                          0xff*(buffer[CHASSIS_PIONEER_SIP_SONAR_VALUE_OFFSET+2+3*i])   ); //low byte Sonarrange SIP
                sonarData.point[sonarNum].distance = sonarValue;
                GDOS_DBG_DETAIL("sonar[%d]: distance %d\n", sonarNum, sonarValue);

                switch (sonarNum)
                {
                    case 0:
                        sonarData.point[sonarNum].angle = -90.0f * M_PI / 180.0;
                        break;
                    case 1:
                        sonarData.point[sonarNum].angle = -50.0f * M_PI / 180.0;
                        break;
                    case 2:
                        sonarData.point[sonarNum].angle = -30.0f * M_PI / 180.0;
                        break;
                    case 3:
                        sonarData.point[sonarNum].angle = -10.0f * M_PI / 180.0;
                        break;
                    case 4:
                        sonarData.point[sonarNum].angle = +10.0f * M_PI / 180.0;
                        break;
                    case 5:
                        sonarData.point[sonarNum].angle = +30.0f * M_PI / 180.0;
                        break;
                    case 6:
                        sonarData.point[sonarNum].angle = +50.0f * M_PI / 180.0;
                        break;
                    case 7:
                        sonarData.point[sonarNum].angle = +90.0f * M_PI / 180.0;
                        break;

                    case 8:
                        sonarData.point[sonarNum].angle = +90.0f * M_PI / 180.0;
                        break;
                    case 9:
                        sonarData.point[sonarNum].angle = +130.0f * M_PI / 180.0;
                        break;
                    case 10:
                        sonarData.point[sonarNum].angle = 150.0f * M_PI / 180.0;
                        break;
                    case 11:
                        sonarData.point[sonarNum].angle = 170.0f * M_PI / 180.0;
                        break;
                    case 12:
                        sonarData.point[sonarNum].angle = 190.0f * M_PI / 180.0;
                        break;
                    case 13:
                        sonarData.point[sonarNum].angle = 210.0f * M_PI / 180.0;
                        break;
                    case 14:
                        sonarData.point[sonarNum].angle = 230.0f * M_PI / 180.0;
                        break;
                    case 15:
                        sonarData.point[sonarNum].angle = 270.0f * M_PI / 180.0;
                        break;
                    default:
                        sonarData.point[sonarNum].angle = 0.0f;
                        sonarData.point[sonarNum].distance = 0;
                }
            }

            sonarData.data.recordingTime = time;
            sonarData.data.pointNum      = CHASSIS_PIONEER_SONAR_NUM_MAX;

            // send sonar data
            if (ladarSonarInst >= 0)
            {
                GDOS_DBG_DETAIL("sonarData recordingtime %i pointNum %i\n",
                                sonarData.data.recordingTime,
                                sonarData.data.pointNum);
                datalength =  sizeof(ladar_data) +
                              sonarData.data.pointNum * sizeof(ladar_point);

                ret = workMbx.sendDataMsg(MSG_DATA, ladarSonarMbxAdr + 1, 1, 1,
                                         &sonarData, datalength);
                if (ret)
                {
                    GDOS_ERROR("Error while sending objRecogBound data from %x to %x (bytes %d)\n",
                               workMbx.getAdr(), ladarSonarMbxAdr, datalength);
                    return ret;
                }
            }
        }
    }

    if (buffer[3] == 0x90) //Encoder Package
    {
        leftEncoderNew = (unsigned int)( ((buffer[6] << 24) & 0xff000000) | ((buffer[5] << 16) & 0x00ff0000) | ((buffer[4] << 8) & 0x0000ff00) | buffer[3]);
        rightEncoderNew = (unsigned int)( ((buffer[10] << 24) & 0xff000000) | ((buffer[9] << 16) & 0x00ff0000) | ((buffer[8] << 8) & 0x0000ff00) | buffer[7]);

        if (leftEncoderOld == 0x7fffffff)
        {
            leftEncoderOld = leftEncoderNew;
            rightEncoderOld = rightEncoderNew;
            oldTimestamp = time;
        }

        vL = ((float)(leftEncoderNew - leftEncoderOld) / TICKS_PER_MM);
        vR = ((float)(rightEncoderNew - rightEncoderOld) / TICKS_PER_MM);

        p_data->deltaX   = (vL + vR) / 2.0;
        p_data->deltaY   = 0.0f;
        p_data->deltaRho = (vL - vR) / 2.0 / 200.0 * CALIBRATION_ROT;

        deltaT = (float)(time - oldTimestamp) / 1000.0;

        vL = (vL / deltaT);
        vR = (vR / deltaT);

        p_data->vx    = (vL + vR) / 2.0;
        p_data->vy    = 0.0f;
        p_data->omega = (vL - vR) / 2.0 / 200.0 * CALIBRATION_ROT;

        p_data->recordingTime = time;
        p_data->battery       = battery;
        p_data->activePilot   = activePilot;

        datalength = sizeof(chassis_data);

        putDataBufferWorkSpace(datalength);

        GDOS_DBG_DETAIL("lEncoder=%i rEncoder=%i vx=%f mm/s omega=%a deg/s\n",
                        leftEncoderNew, rightEncoderNew,
                        p_data->vx, p_data->omega);

        leftEncoderOld = leftEncoderNew;
        rightEncoderOld = rightEncoderNew;
        oldTimestamp = time;
    }

    if (++watchdogCounter > 10)
    {
        GDOS_DBG_DETAIL("Send pulse command\n");

        //pulse
        hwMtx.lock(RACK_INFINITE);

        ret = sendPackage(pulseCommand, sizeof(pulseCommand));
        if (ret)
        {
            GDOS_ERROR("Can't send pulse command\n");
            hwMtx.unlock();
            return ret;
        }

        watchdogCounter = 0;

        hwMtx.unlock();
    }

  //  RackTask::sleep(50000000llu);   //50ms

    return 0;
}

// realtime context
int ChassisPioneer::moduleCommand(RackMessage *msgInfo)
{
    unsigned int pilot_mask = RackName::getSysMask() |
                              RackName::getClassMask() |
                              RackName::getInstMask();
    chassis_move_data               *p_move;
    chassis_set_active_pilot_data   *p_pilot;

    switch (msgInfo->getType())
    {
    case MSG_CHASSIS_MOVE:
        if (status != MODULE_STATE_ENABLED)
        {
            cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
            break;
        }

        p_move = ChassisMoveData::parse(msgInfo);
        if ((msgInfo->getSrc() & pilot_mask) != activePilot)
        {
            cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
            break;
        }

        // set min speed
        if ((p_move->vx > 0) && (p_move->vx < param.vxMin))
            p_move->vx = param.vxMin;
        if ((p_move->vx < 0) && (p_move->vx > -param.vxMin))
            p_move->vx = -param.vxMin;

        if (sendMovePackage(p_move->vx, p_move->omega))
        {
            cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
            break;
        }
        cmdMbx.sendMsgReply(MSG_OK, msgInfo);
        break;

    case MSG_CHASSIS_GET_PARAMETER:
        cmdMbx.sendDataMsgReply(MSG_CHASSIS_PARAMETER, msgInfo, 1, &param,
                                sizeof(chassis_param_data));
        break;

    case MSG_CHASSIS_SET_ACTIVE_PILOT:
        p_pilot = ChassisSetActivePilotData::parse(msgInfo);

        activePilot = p_pilot->activePilot & pilot_mask;

        sendMovePackage(0, 0);

        GDOS_PRINT("%n nhanged active pilot to %n", msgInfo->getSrc(), activePilot);
        cmdMbx.sendMsgReply(MSG_OK, msgInfo);
        break;

    default:
        // not for me -> ask RackDataModule
        return RackDataModule::moduleCommand(msgInfo);
    }
    return 0;
}

int ChassisPioneer::calculate_checksum(unsigned char *ptr)
{
    int n;
    int c = 0;

    n = *(ptr++);
    n -= 2;
    while (n > 1)
    {
        c += (*(ptr) << 8) | *(ptr + 1);
        c = c & 0xffff;
        n -= 2;
        ptr += 2;
    }
    if (n > 0)
        c = c ^ (int) * (ptr++);
    *(ptr + 1) = (unsigned char)(c & 0x00ff);
    *(ptr) = (unsigned char)(c >> 8);
    return c;
}

int ChassisPioneer::receivePackage(unsigned char *sipBuffer, rack_time_t *timestamp)
{
    int totalCount = 0;
    int ret;

    do
    {
        ret = serialPort.recv(sipBuffer, 1, timestamp);
        if (ret)
        {
            GDOS_ERROR("Receive package timeout on serial dev %i\n", serialDev);
            return ret;
        }
        if (++totalCount > MAX_SIP_PACKAGE_SIZE)
        {
            GDOS_ERROR("Can't synchronize on package head\n");
            return -ETIME;
        }
    }
    while (sipBuffer[0] != 0xFA);

    ret = serialPort.recv(&sipBuffer[1], 2, NULL);
    if (ret)
    {
        GDOS_ERROR("Can't read package head\n");
        return ret;
    }

    ret = serialPort.recv(&sipBuffer[3], sipBuffer[2], NULL);
    if (ret)
    {
        GDOS_ERROR("Can't read package body\n");
        return ret;
    }

    GDOS_DBG_DETAIL("Received SIP: [0]=%x [1]=%x [2]=%x [3]=%x ...\n",
                    sipBuffer[0], sipBuffer[1], sipBuffer[2], sipBuffer[3]);

    return 0;
}

int ChassisPioneer::sendPackage(const unsigned char *package, int packageSize)
{
    int ret;

    ret = serialPort.send(package, packageSize);
    if (ret)
    {
        return ret;
    }

    GDOS_DBG_DETAIL("Send package: [0]=%x [1]=%x [2]=%x [3]=%x ...\n",
                    package[0], package[1], package[2], package[3]);

    // wait at least 10ms between sending two commands
    RackTask::sleep(10000000llu);

    return 0;
}

int ChassisPioneer::sendMovePackage(int vx, float omega)
{
    int omegaDeg;
    int ret;

    hwMtx.lock(RACK_INFINITE);

    memcpy(moveCommand, moveTemplate, sizeof(moveCommand));
    memcpy(turnCommand, turnTemplate, sizeof(turnCommand));

    if (vx >= 0)
    {
        moveCommand[4] = 0x3B;
    }
    else
    {
        moveCommand[4] = 0x1B;
        vx = -vx;
    }
    moveCommand[5] = vx & 0x00ff;
    moveCommand[6] = vx >> 8;

    calculate_checksum(&moveCommand[2]);
    ret = sendPackage(moveCommand, sizeof(moveCommand));
    if (ret)
    {
        hwMtx.unlock();
        return ret;
    }

    if (omega == 0)
    {
        turnCommand[4] = 0x1B;
        turnCommand[5] = 0x00;
        turnCommand[6] = 0x00;
    }
    else
    {
        omegaDeg = (int)((180.0 / M_PI) * omega);

        if (omegaDeg >= 0)
        {
            omegaDeg = ((2 * omegaDeg) + 1) / 2;
            turnCommand[4] = 0x3B;
        }
        else
        {
            omegaDeg = ((2 * omegaDeg) - 1) / 2;
            turnCommand[4] = 0x3B;
        }

        omegaDeg = -omegaDeg;  // chassis counts left positive

        turnCommand[5] = omegaDeg & 0x00ff;
        turnCommand[6] = omegaDeg >> 8;
    }

    calculate_checksum(&turnCommand[2]);
    ret = sendPackage(turnCommand, sizeof(turnCommand));
    if (ret)
    {
        hwMtx.unlock();
        return ret;
    }

    watchdogCounter = 0;

    hwMtx.unlock();

    return 0;
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

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE                0
#define INIT_BIT_RTSERIAL_OPENED            1
#define INIT_BIT_MTX_CREATED                2
#define INIT_BIT_MBX_WORK                   3
#define INIT_BIT_PROXY_SONAR                4

int ChassisPioneer::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    // open serial port
    ret = serialPort.open(serialDev, &pioneer_serial_config, this);
    if (ret)
    {
        printf("Can't open serialDev %i\n", serialDev);
        goto init_error;
    }
    initBits.setBit(INIT_BIT_RTSERIAL_OPENED);

    // create hardware mutex
    ret = hwMtx.create();
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

    // create ladarSonar proxy
    if (ladarSonarInst >= 0)
    {
        ladarSonarMbxAdr = RackName::create(ladarSonarSys, LADAR, ladarSonarInst);
        ladarSonar       = new LadarProxy(&workMbx, ladarSonarSys, ladarSonarInst);
        if (!ladarSonar)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_SONAR);
    }

    return 0;

init_error:
    ChassisPioneer::moduleCleanup();
    return ret;
}

void ChassisPioneer::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // destroy ladarSonar proxy
    if (ladarSonarInst >= 0)
    {
        if (initBits.testAndClearBit(INIT_BIT_PROXY_SONAR))
        {
            delete ladarSonar;
        }
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }

    // destroy mutex
    if (initBits.testAndClearBit(INIT_BIT_MTX_CREATED))
    {
        hwMtx.destroy();
    }

    // close serial port
    if (initBits.testAndClearBit(INIT_BIT_RTSERIAL_OPENED))
    {
        serialPort.close();
    }
}

ChassisPioneer::ChassisPioneer()
        : RackDataModule( MODULE_CLASS_ID,
                      5000000000llu,        // 5s datatask error sleep time
                      16,                   // command mailbox slots
                      48,                   // command mailbox data size per slot
                      MBX_IN_KERNELSPACE | MBX_SLOT, // command mailbox flags
                      5,                    // max buffer entries
                      10)                   // data buffer listener
{
    // get static module parameter
    serialDev               = getIntArg("serialDev", argTab);
    ladarSonarSys           = getIntArg("ladarSonarSys", argTab);
    ladarSonarInst          = getIntArg("ladarSonarInst", argTab);
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
    dataBufferPeriodTime    = 100; // 100 ms (10 per sec)
}

int main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "ChassisPioneer");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new ChassisPioneer

    ChassisPioneer *pInst;

    pInst = new ChassisPioneer();
    if (!pInst)
    {
        printf("Can't create new ChassisPioneer -> EXIT\n");
        return -ENOMEM;
    }

    // init

    ret = pInst->moduleInit();
    if (ret)
        goto exit_error;

    pInst->run();

    return 0;

exit_error:

    delete (pInst);

    return ret;
}
