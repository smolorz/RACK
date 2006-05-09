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
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#include <iostream>

#include "chassis_pioneer.h"

//
// data structures
//

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE                0
#define INIT_BIT_RTSERIAL_OPENED            1
#define INIT_BIT_MTX_CREATED                2

#define MAX_SIP_PACKAGE_SIZE 208

#define CALIBRATION_ROT 0.66
#define TICKS_PER_MM    12426.34 //durch messung. bei 3bar reifendruck rechnerisch: 8700000.0 / (208.0 * M_PI) ;

//#define MAX_NUMBER_OF_SONARS 16
//#define SIP_SONAR_NR_OFFSET    22
//#define SIP_SONAR_VALUE_OFFSET (SIP_SONAR_NR_OFFSET+1)

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

ChassisPioneer *p_inst;

argTable_t argTab[] = {

  { ARGOPT_REQ, "serialDev", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "Serial device number", (int)-1 },

  { ARGOPT_REQ, "sonar", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "disable sonar = 0; enable sonar = 1", (int)0 },

  { 0,"",0,0,""}                                  // last entry
};

const struct rtser_config pioneer_serial_config = {
    config_mask       : 0xFFFF,
    baud_rate         : 9600,
    parity            : RTSER_NO_PARITY,
    data_bits         : RTSER_8_BITS,
    stop_bits         : RTSER_1_STOPB,
    handshake         : RTSER_DEF_HAND,
    fifo_depth        : RTSER_DEF_FIFO_DEPTH,
    rx_timeout        : 200000000llu,
    tx_timeout        : RTSER_DEF_TIMEOUT,
    event_timeout     : RTSER_DEF_TIMEOUT,
    timestamp_history : RTSER_RX_TIMESTAMP_HISTORY,
    event_mask        : RTSER_EVENT_RXPEND
};

// Parameter des Fahrzeugs

chassis_param_data param = {
    vxMax:            700,                  // mm/s
    vyMax:            0,
    vxMin:            50,                   // mm/s
    vyMin:            0,
    axMax:            500,                  // mm/s
    ayMax:            0,
    omegaMax:         (20.0 * M_PI / 180.0),// rad/s
    omegaDotMax:      (30.0 * M_PI / 180.0),// rad/s
    minTurningRadius: 200,                  // mm

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

    serialPort.clean();
    serialPort.setRxTimeout(200000000llu);

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
        serialPort.setRxTimeout(1000000000llu);
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

    serialPort.setRxTimeout(200000000llu);

    return DataModule::moduleOn();  // have to be last command in moduleOn();
}

// realtime context
void ChassisPioneer::moduleOff(void)
{
    DataModule::moduleOff();        // have to be first command in moduleOff();

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
    RACK_TIME       time;
    float           deltaT, vL, vR;
    int             ret, leftEncoderNew, rightEncoderNew;

    /* SONAR_DATA_PACKAGE sonarData;  // datastructure for sonar mbx
        int i;
        int sonarsChanged;
        int sonarnumber;
        int sonarvalue; */

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

        /*// begin sonar driver pioneer2AT +++++++++++++++++++++++++++++++++++++++++++

          sonarData.body.distanceNum = MAX_NUMBER_OF_SONARS;
          sonarsChanged = (unsigned int) buffer[SIP_SONAR_NR_OFFSET]; // p2opman.pdf sonarReadings SIP
          if (sonarsChanged > 0)           // sonarvalues are changed
          {
                 for (i=0;i<=sonarsChanged;i++)
                  {
                      sonarnumber=buffer[SIP_SONAR_VALUE_OFFSET+3*i];   //Sonarnumber SIP
                      sonarvalue=0.268*(          // 0.0268 for centimeters
                         (buffer[SIP_SONAR_VALUE_OFFSET+1+3*i])+     //low byte Sonarrange SIP
                          0xff*(buffer[SIP_SONAR_VALUE_OFFSET+2+3*i])   ); //low byte Sonarrange SIP
                      sonarData.body.distance[sonarnumber] = sonarvalue;      //assign values to datastructur from sonar.h for MBX
                  } // for

                 sonarData.body.recordingtime = time; //timestamp
                 sonarData.head.flags = 0;
                 sonarData.head.dataLen = 2 * sizeof(int) + sonarData.body.distanceNum * sizeof(int);
                 package_set_body_byteorder((PACKAGE_HEAD*)&sonarData);// set byteorder Wichtig Head und Body setzen
                 package_set_head_byteorder((PACKAGE_HEAD*)&sonarData);
                 package_send(DATA, NAME_CREATE(SONAR,0)+1, CMD_MBX(id), PACKAGE_RP, 0, &sonarData); //senden der Daten an die SonarMBX

              GDOS_DBG_DETAIL("Sonar distanceNum %i\n", sonarData.body.distanceNum);
                }
        // end sonar driver pioneer2AT +++++++++++++++++++++++++++++++++++++++++++*/
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

        GDOS_DBG_DETAIL("lEncoder=%i rEncoder=%i vx=%.1f mm/s omega=%.1a deg/s\n",
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

    return 0;
}

// realtime context
int ChassisPioneer::moduleCommand(MessageInfo *msgInfo)
{
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

        if (((msgInfo->src & pilot_mask) == RackName::create(GUI, 0)) ||
                ((msgInfo->src & pilot_mask) == RackName::create(PILOT, 0)))
        {
            activePilot = p_pilot->activePilot & pilot_mask;
            sendMovePackage(0, 0);
            GDOS_DBG_INFO("%x Changed active pilot to %x", msgInfo->src, activePilot);
            cmdMbx.sendMsgReply(MSG_OK, msgInfo);
        }
        else
        {
            GDOS_ERROR("%x has no permission to change active pilot to %x",
                       msgInfo->src, p_pilot->activePilot);
            cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
        }
        break;

    default:
        // not for me -> ask DataModule
        return DataModule::moduleCommand(msgInfo);
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

int ChassisPioneer::receivePackage(unsigned char *sipBuffer, RACK_TIME *timestamp)
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

int ChassisPioneer::moduleInit(void)
{
    int ret;

    // call DataModule init function (first command in init)
    ret = DataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    // open serial port
    ret = serialPort.open(serialDev, &pioneer_serial_config);
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

    return 0;

init_error:
    ChassisPioneer::moduleCleanup();
    return ret;
}

void ChassisPioneer::moduleCleanup(void)
{
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

    // call DataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        DataModule::moduleCleanup();
    }
}

ChassisPioneer::ChassisPioneer()
        : DataModule( MODULE_CLASS_ID,
                      5000000000llu,        // 5s cmdtask error sleep time
                      5000000000llu,        // 5s datatask error sleep time
                      100000000llu,         // 100ms datatask disable sleep time
                      16,                   // command mailbox slots
                      48,                   // command mailbox data size per slot
                      MBX_IN_KERNELSPACE | MBX_SLOT, // command mailbox flags
                      5,                    // max buffer entries
                      10)                   // data buffer listener
{
    // get value(s) out of your argument table
    serialDev = getIntArg("serialDev", argTab);
    sonar = getIntArg("sonar", argTab);

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(chassis_data));

    // set databuffer period time
    setDataBufferPeriodTime(100); // 100 ms (10 per sec)
}

int main(int argc, char *argv[])
{
    int ret;


    // get args
    ret = Module::getArgs(argc, argv, argTab, "ChassisPioneer");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new ChassisPioneer

    p_inst = new ChassisPioneer();
    if (!p_inst)
    {
        printf("Can't create new ChassisPioneer -> EXIT\n");
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
