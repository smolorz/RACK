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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
#include <iostream>

#include "chassis_roomba.h"

//
// data structures
//

// init_flags (for init and cleanup)
#define INIT_BIT_MBX_WORK                   0
#define INIT_BIT_DATA_MODULE                1
#define INIT_BIT_RTSERIAL_OPENED            2
#define INIT_BIT_PROXY_SCAN2D               3
#define INIT_BIT_MTX_CREATED                4


ChassisRoomba *p_inst;

argTable_t argTab[] = {

    { ARGOPT_REQ, "serialDev", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Serial device number", { -1 } },

    { ARGOPT_OPT, "scan2dInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Instance for the bumper and wall sensor data relay, default -1", { -1 } },

    { ARGOPT_OPT, "vxMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "max vehicle velocity in x direction, default 500 m/s", { 500 } },

    { ARGOPT_OPT, "vxMin", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "min vehicle velocity in x direction, default 50 m/s)", { 50 } },

    { ARGOPT_OPT, "axMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "max vehicle acceleration in x direction, default 500", { 500 } },

    { ARGOPT_OPT, "omegaMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "omegaMax, default 30 deg/s", { 30 } },

    { ARGOPT_OPT, "minTurningRadius", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "min vehicle turning radius, default 100 (mm)", { 100 } },

    { ARGOPT_OPT, "breakConstant", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "breakConstant, default 100 (1.0f)", { 100 } },

    { ARGOPT_OPT, "safetyMargin", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "safetyMargin, default 50 (mm)", { 50 } },

    { ARGOPT_OPT, "safetyMarginMove", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "safety margin in move directiom, default 0 (mm)", { 0 } },

    { ARGOPT_OPT, "comfortMargin", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "comfortMargin, default 300 (mm)", { 300 } },

    { ARGOPT_OPT, "front", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "front, default 170 (mm)", { 170 } },

    { ARGOPT_OPT, "back", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "back, default 170 (mm)", { 170 } },

    { ARGOPT_OPT, "left", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "left, default 170 (mm)", { 170 } },

    { ARGOPT_OPT, "right", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "right, default 170 (mm)", { 170 } },

    { ARGOPT_OPT, "wheelBase", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "wheel distance, default 258 (mm)", { 258 } },

    { ARGOPT_OPT, "wheelRadius", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "wheelRadius, default 110 (mm)", { 110 } },

    { ARGOPT_OPT, "trackWidth", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "trackWidth, default 258 (mm)", { 258 } },

    { ARGOPT_OPT, "pilotParameterA", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "pilotParameterA, default 10 (0.001f)", { 10 } },

    { ARGOPT_OPT, "pilotParameterB", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "pilotParameterB, default 200 (2.0f)", { 200 } },

    { ARGOPT_OPT, "pilotVTransMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "pilotVTransMax, default 200", { 200 } },

    { ARGOPT_OPT, "motorMainBrush", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "State of the main brush motor, 0 = off, 1 = on, default 0", { 0 } },

    { ARGOPT_OPT, "motorVacuum", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "State of the vacuum motor, 0 = off, 1 = on, default 0", { 0 } },

    { ARGOPT_OPT, "motorSideBrush", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "State of the side brush motor, 0 = off, 1 = on, default 0", { 0 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};

const struct rtser_config roomba_serial_config = {
    config_mask       : 0xFFFF,
    baud_rate         : 57600,
    parity            : RTSER_NO_PARITY,
    data_bits         : RTSER_8_BITS,
    stop_bits         : RTSER_1_STOPB,
    handshake         : RTSER_DEF_HAND,
    fifo_depth        : RTSER_DEF_FIFO_DEPTH,
    rx_timeout        : 500000000llu,
    tx_timeout        : RTSER_DEF_TIMEOUT,
    event_timeout     : 500000000llu,
    timestamp_history : RTSER_RX_TIMESTAMP_HISTORY,
    event_mask        : RTSER_EVENT_RXPEND
};


// vehicle parameter
chassis_param_data param = {
    vxMax:            500,                  // mm/s
    vyMax:            0,
    vxMin:            50,                   // mm/s
    vyMin:            0,
    axMax:            500,                  // mm/s
    ayMax:            0,
    omegaMax:         (30.0 * M_PI / 180.0),// rad/s
    minTurningRadius: 100,                  // mm

    breakConstant:    1.0f,                 // mm/mm/s
    safetyMargin:     50,                   // mm
    safetyMarginMove: 200,                  // mm
    comfortMargin:    300,                  // mm

    boundaryFront:    170,                  // mm
    boundaryBack:     170,                  // mm
    boundaryLeft:     170,                  // mm
    boundaryRight:    170,                  // mm

    wheelBase:        258,                  // mm
    wheelRadius:      110,                  // mm
    trackWidth:       258,

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
 int ChassisRoomba::moduleOn(void)
{
    int ret;

    if (scan2dInst >= 0)
    {
        // init scan2d values and send first dataMsg
        scan2dDataMsg.data.recordingTime = rackTime.get();
        scan2dDataMsg.data.pointNum      = 0;

        ret = workMbx.sendDataMsg(MSG_DATA, scan2dMbxAdr + 1, 1, 1,
                                 &scan2dDataMsg, sizeof(scan2d_data));
        if (ret)
        {
            GDOS_WARNING("Error while sending first scan2d data from %x to %x (bytes %d)\n",
                         workMbx.getAdr(), scan2dMbxAdr, sizeof(scan2d_data));
        }

        GDOS_DBG_DETAIL("Turn on Scan2d(%d)\n", scan2dInst);
        ret = scan2d->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on Scan2d(%i), code = %d\n", scan2dInst, ret);
            return ret;
        }
    }

    // turn on roomba
    ret = setMode(CHASSIS_ROOMBA_ROI_START);
    if (ret)
    {
        return ret;
    }

    // switch roomba baudrate to 19200
    ret = setBaudrate(7);
    if (ret)
    {
        return ret;
    }

    // switch SPB baudrate
    ret = serialPort.setBaudrate(19200);
    if (ret)
    {
        return ret;
    }
    RackTask::sleep(100000000llu);   // 100ms
    serialPort.clean();

    // active external control
    ret = setMode(CHASSIS_ROOMBA_ROI_CONTROL);
    if (ret)
    {
        return ret;
    }

    // play note
    ret = playNote(15, 72, 200);
    if (ret)
    {
        return ret;
    }

    // set cleaning motor
    ret = setCleaningMotor(motorMainBrush, motorVacuum, motorSideBrush);
    if (ret)
    {
        return ret;
    }

    activePilot        = CHASSIS_INVAL_PILOT;
    recordingTimeOld   = rackTime.get();
    speed              = 0;
    omega              = 0.0f;
    overcurrentCounter = 0;

    return RackDataModule::moduleOn();  // has to be last command in moduleOn();
}

// realtime context
void ChassisRoomba::moduleOff(void)
{
    RackDataModule::moduleOff();        // has to be first command in moduleOff();

    speed = 0;
    omega = 0.0f;
    activePilot = CHASSIS_INVAL_PILOT;

    sendMoveCommand(speed, omega);
    setCleaningMotor(0, 0, 0);
    playNote(15, 72, 200);
}

// realtime context
int ChassisRoomba::moduleLoop(void)
{
    int             ret;
    int             dT;
    chassis_data*   p_data = NULL;
    ssize_t         datalength = 0;

    // get datapointer from rackdatabuffer
    p_data = (chassis_data *)getDataBufferWorkSpace();

    // move roomba
    ret = sendMoveCommand(speed, omega);
    if (ret)
    {
        GDOS_ERROR("Can't send move command to roomba, code = %d\n", ret);
        hwMtx.unlock();
        return ret;
    }

    // get sensor data from roomba
    ret = readSensorData(&sensorData);
    if (ret)
    {
        return ret;
    }

    p_data->recordingTime = sensorData.recordingTime;
    p_data->deltaRho      = -(float)sensorData.angle * CHASSIS_ROOMBA_DEGREES_PER_MM * M_PI / 180.0f;
    p_data->deltaX        =  (float)sensorData.distance;
    p_data->deltaY        =  0.0f;
    dT                    = (int)(p_data->recordingTime - recordingTimeOld);
    p_data->vx            = p_data->deltaX * 1000.0f / dT;
    p_data->vy            = 0.0f;
    p_data->omega         = p_data->deltaRho * 1000.0f / dT;
    p_data->battery       = (float)sensorData.batteryVoltage / 1000.0f;
    p_data->activePilot   = activePilot;
    recordingTimeOld      = p_data->recordingTime;

    datalength = sizeof(chassis_data);
    putDataBufferWorkSpace(datalength);


    // send scan2d data
    if (scan2dInst >= 0)
    {
        createScan2d(&sensorData, &scan2dDataMsg.data);
        datalength =  sizeof(scan2d_data) + scan2dDataMsg.data.pointNum * sizeof(scan_point);

        ret = workMbx.sendDataMsg(MSG_DATA, scan2dMbxAdr + 1, 1, 1,
                                 &scan2dDataMsg, datalength);
        if (ret)
        {
            GDOS_ERROR("Error while sending scan2d data from %x to %x (bytes %d)\n",
                       workMbx.getAdr(), scan2dMbxAdr, datalength);
            return ret;
        }
    }

    // overcurrent protection
    if (sensorData.motorOvercurrents)
    {
        if ((sensorData.motorOvercurrents & 0x1) == 0x1)
        {
            GDOS_ERROR("Overcurrent in motor for side brush!\n");
        }

        if ((sensorData.motorOvercurrents & 0x2) == 0x2)
        {
            GDOS_ERROR("Overcurrent in motor for vacuum!\n");
        }

        if ((sensorData.motorOvercurrents & 0x3) == 0x3)
        {
            GDOS_ERROR("Overcurrent in motor for main brush!\n");
        }

        if ((sensorData.motorOvercurrents & 0x4) == 0x4)
        {
            GDOS_ERROR("Overcurrent in drive right!\n");
        }

        if ((sensorData.motorOvercurrents & 0x5) == 0x5)
        {
            GDOS_ERROR("Overcurrent in drive left!\n");
        }

        overcurrentCounter++;

        // overcurrent for 1s
        if (overcurrentCounter >= 10)
        {
            GDOS_ERROR("Shutting down...\n");
            return -EIO;
        }
    }
    else
    {
        overcurrentCounter = 0;
    }

    return 0;
}

// realtime context
int ChassisRoomba::moduleCommand(message_info *msgInfo)
{
    int             ret;
    unsigned int pilot_mask = RackName::getSysMask() |
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

        // set min speed
        if ((p_move->vx > 0) && (p_move->vx < param.vxMin))
            p_move->vx = param.vxMin;
        if ((p_move->vx < 0) && (p_move->vx > -param.vxMin))
            p_move->vx = -param.vxMin;

        // store speed values
        hwMtx.lock(RACK_INFINITE);
        speed = p_move->vx;
        omega = p_move->omega;
        hwMtx.unlock();

        cmdMbx.sendMsgReply(MSG_OK, msgInfo);
        break;

    case MSG_CHASSIS_GET_PARAMETER:
        cmdMbx.sendDataMsgReply(MSG_CHASSIS_PARAMETER, msgInfo, 1, &param,
                                sizeof(chassis_param_data));
        break;

    case MSG_CHASSIS_SET_ACTIVE_PILOT:
        p_pilot = ChassisSetActivePilotData::parse(msgInfo);

        activePilot = p_pilot->activePilot & pilot_mask;

        ret = sendMoveCommand(0,0.0);
        if (ret)
        {
            cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
            break;
        }

        GDOS_PRINT("%n Changed active pilot to %n", msgInfo->src, activePilot);
        cmdMbx.sendMsgReply(MSG_OK, msgInfo);
        break;

    default:
        // not for me -> ask RackDataModule
        return RackDataModule::moduleCommand(msgInfo);
    }
    return 0;
}

int ChassisRoomba::setBaudrate(int baudNum)
{
    int ret;

    serialBuffer[0] = (char)CHASSIS_ROOMBA_ROI_BAUD;
    serialBuffer[1] = (char)(baudNum & 0xff);

    ret = serialPort.send(serialBuffer, 2);
    if (ret)
    {
        GDOS_ERROR("Can't send BAUD-COMMAND to roomba, code = %d\n", ret);
    }
    RackTask::sleep(250000000llu);   // 250ms
    return ret;
}

int ChassisRoomba::setMode(int mode)
{
    int ret;

    serialBuffer[0] = (char)(mode & 0xff);
    ret = serialPort.send(serialBuffer, 1);
    if (ret)
    {
        GDOS_ERROR("Can't send mode coomand %x to roomba, code = %d\n", mode, ret);
    }
    RackTask::sleep(20000000llu);   // 20ms

    return ret;
}

int ChassisRoomba::playNote(int songNum, int note, int duration)
{
    int ret;

    // define note
    serialBuffer[0] = (char)CHASSIS_ROOMBA_ROI_SONG;
    serialBuffer[1] = (char)(songNum & 0xff);
    serialBuffer[2] = (char)1;                                  // song length
    serialBuffer[3] = (char)(note & 0xff);
    serialBuffer[4] = (char)((duration * 64 / 1000) & 0xff);    // from ms to 1/64 s

    ret = serialPort.send(serialBuffer, 5);
    if (ret)
    {
        GDOS_ERROR("Can't send SONG-COMMAND to roomba, code = %d\n", ret);
        return ret;
    }

    // play note
    ret = playSong(songNum);
    return ret;
}

int ChassisRoomba::playSong(int songNum)
{
    int ret;

    serialBuffer[0] = (char)CHASSIS_ROOMBA_ROI_PLAY;
    serialBuffer[1] = (char)(songNum & 0xff);

    ret = serialPort.send(serialBuffer, 2);
    if (ret)
    {
        GDOS_ERROR("Can't send PLAY-COMMAND to roomba, code = %d\n", ret);
    }
    return ret;
}

int ChassisRoomba::setCleaningMotor(int mainBrush, int vacuum, int sideBrush)
{
    int             ret;

    serialBuffer[0] = (char)CHASSIS_ROOMBA_ROI_MOTORS;
    serialBuffer[1] = (char)0x0;

    // main brush
    if (mainBrush)
    {
        serialBuffer[1] |= (char)0x04;
    }

    // vacuum
    if (vacuum)
    {
        serialBuffer[1] |= (char)0x02;
    }

    // side brush
    if (sideBrush)
    {
        serialBuffer[1] |= (char)0x01;
    }

    ret = serialPort.send(serialBuffer, 2);
    if (ret)
    {
        GDOS_ERROR("Can't send MOTORS-COMMAND to roomba, code = %d\n", ret);
    }

    RackTask::sleep(20000000llu);       //20 ms

    return ret;
}

int ChassisRoomba::sendMoveCommand(int speed, float omega)
{
    int             ret;
    int             radius;

    hwMtx.lock(RACK_INFINITE);

    if (omega != 0)
    {
        radius = -(int)rint((float)speed / omega);

        if (abs(radius) > CHASSIS_ROOMBA_RADIUS_MAX)
        {
            radius = 0x8000;                            // straight line
        }

        if (speed == 0)
        {
            if (omega > 0)
            {
                speed  = (int)rint((omega / param.omegaMax) * (float)param.vxMax);
                radius = -1;                            // spin clockwise
            }
            else
            {
                speed  = (int)rint((-omega / param.omegaMax) * (float)param.vxMax);
                radius = 1;                             // spin counter-clockwise
            }
        }
    }
    else
    {
        radius = 0x8000;                                // straight line
    }

    serialBuffer[0] = (char)CHASSIS_ROOMBA_ROI_DRIVE;
    serialBuffer[1] = (char)(speed >> 8);
    serialBuffer[2] = (char)(speed & 0xff);
    serialBuffer[3] = (char)(radius >> 8);
    serialBuffer[4] = (char)(radius & 0xff);

    ret = serialPort.send(serialBuffer, 5);
    if (ret)
    {
        GDOS_ERROR("Can't send DRIVE-COMMAND to roomba, code = %d\n", ret);
    }

    hwMtx.unlock();

    RackTask::sleep((dataBufferPeriodTime / 3) * 1000000llu);     // periodTime / 3
    return ret;
}

int ChassisRoomba::readSensorData(chassis_roomba_sensor_data *sensor)
{
    int             ret;
    rack_time_t     timestamp;

    //request all sensor data
    serialBuffer[0] = (char)CHASSIS_ROOMBA_ROI_SENSORS;
    serialBuffer[1] = (char)0x0;

    ret = serialPort.send(serialBuffer, 2);
    if (ret)
    {
        GDOS_ERROR("Can't send SENSORS-COMMAND to roomba, code = %d\n", ret);
        return ret;
    }

    RackTask::sleep((dataBufferPeriodTime / 3) * 1000000llu);     // periodTime / 3


    // receive sensor data
    ret = serialPort.recv(serialBuffer, 26, &timestamp);
    if (ret)
    {
        GDOS_ERROR("Error on receiving sensor data from roomba, code = %d\n", ret);
        return ret;
    }

    // physical sensors
    sensor->recordingTime      = timestamp;
    sensor->bumpAndWheeldrop   = (int)serialBuffer[0];
    sensor->wall               = (int)serialBuffer[1];
    sensor->cliffLeft          = (int)serialBuffer[2];
    sensor->cliffFrontLeft     = (int)serialBuffer[3];
    sensor->cliffFrontRight    = (int)serialBuffer[4];
    sensor->cliffRight         = (int)serialBuffer[5];
    sensor->virtualWall        = (int)serialBuffer[6];
    sensor->motorOvercurrents  = (int)serialBuffer[7];
    sensor->dirtDetectorLeft   = (int)serialBuffer[8];
    sensor->dirtDetectorRight  = (int)serialBuffer[9];

    // buttons and internal sensors
    sensor->remoteControl      = (int)serialBuffer[10];
    sensor->buttons            = (int)serialBuffer[11];
    sensor->distance           = (int)((serialBuffer[12] << 8) | (serialBuffer[13] & 0xff));
    sensor->angle              = (int)((serialBuffer[14] << 8) | (serialBuffer[15] & 0xff));

    //power sensor
    sensor->chargingState      = (int)serialBuffer[16];
    sensor->batteryVoltage     = (int)((serialBuffer[17] << 8) | (serialBuffer[18] & 0xff));
    sensor->batteryCurrent     = (int)((serialBuffer[19] << 8) | (serialBuffer[20] & 0xff));
    sensor->batteryTemperature = (int)serialBuffer[21];
    sensor->batteryCharge      = (int)((serialBuffer[22] << 8) | (serialBuffer[23] & 0xff));
    sensor->batteryCapacity    = (int)((serialBuffer[24] << 8) | (serialBuffer[25] & 0xff));

    RackTask::sleep((dataBufferPeriodTime / 3) * 1000000llu);     // periodTime / 3

    return 0;
}

void ChassisRoomba::createScan2d(chassis_roomba_sensor_data *sensor, scan2d_data *scan2d)
{
    // init values
    scan2d->recordingTime = sensor->recordingTime;
    scan2d->duration      = dataBufferPeriodTime;
    scan2d->maxRange      = 0;
    scan2d->refPos.x      = 0;
    scan2d->refPos.y      = 0;
    scan2d->refPos.z      = 0;
    scan2d->refPos.phi    = 0;
    scan2d->refPos.psi    = 0;
    scan2d->refPos.rho    = 0;
    scan2d->pointNum      = 0;

    if (sensor->wall == 1)
    {
        scan2d->point[scan2d->pointNum].x = CHASSIS_ROOMBA_POS_WALL_X;
        scan2d->point[scan2d->pointNum].y = CHASSIS_ROOMBA_POS_WALL_Y;
        scan2d->point[scan2d->pointNum].z = (int)sqrt(scan2d->point[scan2d->pointNum].x *
                                                      scan2d->point[scan2d->pointNum].x +
                                                      scan2d->point[scan2d->pointNum].y *
                                                      scan2d->point[scan2d->pointNum].y);
        scan2d->point[scan2d->pointNum].type      = TYPE_UNKNOWN;
        scan2d->point[scan2d->pointNum].segment   = 0;
        scan2d->point[scan2d->pointNum].intensity = 0;
        scan2d->pointNum++;
    }

    if (sensor->cliffLeft == 1)
    {
        scan2d->point[scan2d->pointNum].x = CHASSIS_ROOMBA_POS_CLIFF_LEFT_X;
        scan2d->point[scan2d->pointNum].y = CHASSIS_ROOMBA_POS_CLIFF_LEFT_Y;
        scan2d->point[scan2d->pointNum].z = (int)sqrt(scan2d->point[scan2d->pointNum].x *
                                                      scan2d->point[scan2d->pointNum].x +
                                                      scan2d->point[scan2d->pointNum].y *
                                                      scan2d->point[scan2d->pointNum].y);
        scan2d->point[scan2d->pointNum].type      = TYPE_UNKNOWN;
        scan2d->point[scan2d->pointNum].segment   = 1;
        scan2d->point[scan2d->pointNum].intensity = 0;
        scan2d->pointNum++;
    }

    if (sensor->cliffFrontLeft == 1)
    {
        scan2d->point[scan2d->pointNum].x = CHASSIS_ROOMBA_POS_CLIFF_FRONTLEFT_X;
        scan2d->point[scan2d->pointNum].y = CHASSIS_ROOMBA_POS_CLIFF_FRONTLEFT_Y;
        scan2d->point[scan2d->pointNum].z = (int)sqrt(scan2d->point[scan2d->pointNum].x *
                                                      scan2d->point[scan2d->pointNum].x +
                                                      scan2d->point[scan2d->pointNum].y *
                                                      scan2d->point[scan2d->pointNum].y);
        scan2d->point[scan2d->pointNum].type      = TYPE_UNKNOWN;
        scan2d->point[scan2d->pointNum].segment   = 1;
        scan2d->point[scan2d->pointNum].intensity = 0;
        scan2d->pointNum++;
    }

    if (sensor->cliffFrontRight == 1)
    {
        scan2d->point[scan2d->pointNum].x = CHASSIS_ROOMBA_POS_CLIFF_FRONTRIGHT_X;
        scan2d->point[scan2d->pointNum].y = CHASSIS_ROOMBA_POS_CLIFF_FRONTRIGHT_Y;
        scan2d->point[scan2d->pointNum].z = (int)sqrt(scan2d->point[scan2d->pointNum].x *
                                                      scan2d->point[scan2d->pointNum].x +
                                                      scan2d->point[scan2d->pointNum].y *
                                                      scan2d->point[scan2d->pointNum].y);
        scan2d->point[scan2d->pointNum].type      = TYPE_UNKNOWN;
        scan2d->point[scan2d->pointNum].segment   = 1;
        scan2d->point[scan2d->pointNum].intensity = 0;
        scan2d->pointNum++;
    }

    if (sensor->cliffRight == 1)
    {
        scan2d->point[scan2d->pointNum].x = CHASSIS_ROOMBA_POS_CLIFF_RIGHT_X;
        scan2d->point[scan2d->pointNum].y = CHASSIS_ROOMBA_POS_CLIFF_RIGHT_Y;
        scan2d->point[scan2d->pointNum].z = (int)sqrt(scan2d->point[scan2d->pointNum].x *
                                                      scan2d->point[scan2d->pointNum].x +
                                                      scan2d->point[scan2d->pointNum].y *
                                                      scan2d->point[scan2d->pointNum].y);
        scan2d->point[scan2d->pointNum].type      = TYPE_UNKNOWN;
        scan2d->point[scan2d->pointNum].segment   = 1;
        scan2d->point[scan2d->pointNum].intensity = 0;
        scan2d->pointNum++;
    }

    if ((sensor->bumpAndWheeldrop & 0x2) == 2)
    {
        scan2d->point[scan2d->pointNum].x = CHASSIS_ROOMBA_POS_BUMP_LEFT_X;
        scan2d->point[scan2d->pointNum].y = CHASSIS_ROOMBA_POS_BUMP_LEFT_Y;
        scan2d->point[scan2d->pointNum].z = (int)sqrt(scan2d->point[scan2d->pointNum].x *
                                                      scan2d->point[scan2d->pointNum].x +
                                                      scan2d->point[scan2d->pointNum].y *
                                                      scan2d->point[scan2d->pointNum].y);
        scan2d->point[scan2d->pointNum].type      = TYPE_UNKNOWN;
        scan2d->point[scan2d->pointNum].segment   = 2;
        scan2d->point[scan2d->pointNum].intensity = 0;
        scan2d->pointNum++;
    }

    if ((sensor->bumpAndWheeldrop & 0x1) == 1)
    {
        scan2d->point[scan2d->pointNum].x = CHASSIS_ROOMBA_POS_BUMP_RIGHT_X;
        scan2d->point[scan2d->pointNum].y = CHASSIS_ROOMBA_POS_BUMP_RIGHT_Y;
        scan2d->point[scan2d->pointNum].z = (int)sqrt(scan2d->point[scan2d->pointNum].x *
                                                      scan2d->point[scan2d->pointNum].x +
                                                      scan2d->point[scan2d->pointNum].y *
                                                      scan2d->point[scan2d->pointNum].y);
        scan2d->point[scan2d->pointNum].type      = TYPE_UNKNOWN;
        scan2d->point[scan2d->pointNum].segment   = 2;
        scan2d->point[scan2d->pointNum].intensity = 0;
        scan2d->pointNum++;
    }
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
int ChassisRoomba::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    // create mailbox
    ret = createMbx(&workMbx, 10, sizeof(scan2d_data_msg),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // open serial port
    ret = serialPort.open(serialDev, &roomba_serial_config, this);
    if (ret)
    {
        printf("Can't open serialDev %i\n", serialDev);
        goto init_error;
    }
    initBits.setBit(INIT_BIT_RTSERIAL_OPENED);

    // create scan2d proxy
    if (scan2dInst >= 0)
    {
        scan2d = new Scan2dProxy(&workMbx, 0, scan2dInst);
        if (!scan2d)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_SCAN2D);
    }

    // create hardware mutex
    ret = hwMtx.create();
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MTX_CREATED);

    return 0;

init_error:
    ChassisRoomba::moduleCleanup();
    return ret;
}

void ChassisRoomba::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // destroy mutex
    if (initBits.testAndClearBit(INIT_BIT_MTX_CREATED))
    {
        hwMtx.destroy();
    }

    // destroy scan2d proxy
    if (scan2dInst >= 0)
    {
        if (initBits.testAndClearBit(INIT_BIT_PROXY_SCAN2D))
        {
            delete scan2d;
        }
    }

    // close serial port
    if (initBits.testAndClearBit(INIT_BIT_RTSERIAL_OPENED))
    {
        serialPort.close();
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }
}

ChassisRoomba::ChassisRoomba()
        : RackDataModule( MODULE_CLASS_ID,
                      5000000000llu,        // 5s datatask error sleep time
                      16,                   // command mailbox slots
                      48,                   // command mailbox data size per slot
                      MBX_IN_KERNELSPACE | MBX_SLOT, // command mailbox flags
                      5,                    // max buffer entries
                      10)                   // data buffer listener
{
    // get value(s) out of your argument table
    serialDev               = getIntArg("serialDev", argTab);
    scan2dInst              = getIntArg("scan2dRelayInst", argTab);
    param.vxMax             = getIntArg("vxMax", argTab);
    param.vxMin             = getIntArg("vxMin", argTab);
    param.axMax             = getIntArg("axMax", argTab);
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
    motorMainBrush          = getIntArg("motorMainBrush", argTab);
    motorVacuum             = getIntArg("motorVacuum", argTab);
    motorSideBrush          = getIntArg("motorSideBrush", argTab);

    // scan2d mbx adress
    scan2dMbxAdr = RackName::create(SCAN2D, scan2dInst);

    dataBufferMaxDataSize   = sizeof(chassis_data);
    dataBufferPeriodTime    = 100; // 100 ms (10 per sec)
}

int main(int argc, char *argv[])
{
    int ret;


    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "ChassisRoomba");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new ChassisRoomba
    p_inst = new ChassisRoomba();
    if (!p_inst)
    {
        printf("Can't create new ChassisRoomba -> EXIT\n");
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
