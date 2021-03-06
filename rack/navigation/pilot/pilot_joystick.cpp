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
#include "pilot_joystick.h"

//
// data structures
//

arg_table_t argTab[] = {

    { ARGOPT_OPT, "scan2dSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the Scan2D module", { 0 } },

    { ARGOPT_REQ, "scan2dInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the Scan2D module", { -1 } },

    { ARGOPT_OPT, "chassisSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the chassis module", { 0 } },

    { ARGOPT_OPT, "chassisInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the chassis module", { 0 } },

    { ARGOPT_OPT, "joystickSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the Joystick module ", { 0 } },

    { ARGOPT_OPT, "joystickInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the Joystick module ", { 0 } },

    { ARGOPT_OPT, "maxSpeed", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Maximal Speed, default 4000", { 4000 } },

    { ARGOPT_OPT, "maxSpeedSide", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Maximal Speed in longitudinal direction, default 0", { 0 } },

    { ARGOPT_OPT, "mode", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Control mode (0 = speed-radius; 1 = speed-omega) (default 0)", { 0 } },

    { ARGOPT_OPT, "chassisMinTurnRadius", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "min turning radius for chassis module", { 300 } },

    { ARGOPT_OPT, "joystickExpX", ARGOPT_REQVAL, ARGOPT_VAL_FLT,
      "exponent for flatten joystick curve x-axis, 0 for linear (default 0)", { 0 } },

    { ARGOPT_OPT, "joystickExpY", ARGOPT_REQVAL, ARGOPT_VAL_FLT,
      "exponent for flatten joystick curve y-axis (2^(x*exp)-1), 0 for linear (default 0)", { 0 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
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

 int  PilotJoystick::moduleOn(void)
{
    int ret;

    // get dynamic module parameter
    maxSpeed             = getInt32Param("maxSpeed");
    maxSpeedSide         = getInt32Param("maxSpeedSide");
    mode                 = getInt32Param("mode");
    chassisMinTurnRadius = getInt32Param("chassisMinTurnRadius");
    joystickExpX         = getFloatParam("joystickExpX");
    joystickExpY         = getFloatParam("joystickExpY");
    if (joystickExpX < 0.0f) joystickExpX = 0.0f;
    if (joystickExpY < 0.0f) joystickExpY = 0.0f;

    ret = chassis->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on chassis, code = %d\n", ret);
        return ret;
    }

    // get chassis parameter data

    ret = chassis->getParam(&chasParData, sizeof(chassis_param_data));
    if (ret)
    {
        GDOS_ERROR("Can't get chassis parameter, code = %d\n", ret);
        return ret;
    }

    if (maxSpeed > chasParData.vxMax)
    {
        maxSpeed = chasParData.vxMax;
        GDOS_DBG_DETAIL("Max speed : %d\n", maxSpeed);
    }

    if (maxSpeedSide > chasParData.vyMax)
    {
        maxSpeedSide = chasParData.vyMax;
        GDOS_DBG_DETAIL("Max speed side : %d\n", maxSpeedSide);
    }

    if (chasParData.minTurningRadius < chassisMinTurnRadius)
    {
        chasParData.minTurningRadius = chassisMinTurnRadius;
        GDOS_DBG_DETAIL("Min turn radius: %i\n", chassisMinTurnRadius);
    }

    // enable joystick

    ret = joystick->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on joystick(%i/%i), code = %d\n", joystickSys, joystickInst, ret);
        return ret;
    }

    // get joystick data

    ret = joystick->getData(&jstkData, sizeof(joystick_data), 0);
    if (ret)
    {
        GDOS_ERROR("Can't get data from Joystick(%i/%i), code  %d\n",
                   joystickSys, joystickInst, ret);
        return ret;
    }

    if ((jstkData.position.x != 0) ||
        (jstkData.position.y != 0))
    {
        GDOS_ERROR("Joystick(%i/%i) must be (0,0) to turn pilot on\n", joystickSys, joystickInst);
        return -EINVAL;
    }

    // enable scan2d module

    if (scan2dInst >= 0) // scan2d is not used if id is -1
    {
        ret = scan2d->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on Scan2D(%i/%i), code = %d\n",
                       scan2dSys, scan2dInst, ret);
            return ret;
        }
    }

    // get continuous data from joystick

    joystickMbx.clean();

    ret = joystick->getContData(0, &joystickMbx, NULL);
    if (ret)
    {
        GDOS_ERROR("Can't get continuous data from Joystick(%i/%i), code = %d\n",
                   joystickSys, joystickInst, ret);
        return ret;
    }

    joystickSpeed = 0;
    joystickCurve = 0.0f;
    joystickOmega = 0.0f;
    joystickForce = 0;
    joystikDataMissing = 0;

    if (scan2dInst >= 0) // scan2d is not used if id is -1
    {
        scan2dMbx.clean();

        ret = scan2d->getContData(0, &scan2dMbx, NULL);
        if (ret)
        {
            GDOS_ERROR("Can't get continuous data from scan2D(%i/%i), "
                       "code = %d\n", scan2dSys, scan2dInst, ret);
            return ret;
        }

        scan2dMsg.data.pointNum = 0;
        scan2dDataMissing = 0;
    }

    GDOS_PRINT("maxSpeed %f m/s, minRadius %f m  Scan2D(%i/%i)\n",
                (float)maxSpeed / 1000.0f,
                (float)chasParData.minTurningRadius / 1000.0f, scan2dSys, scan2dInst);

    return RackDataModule::moduleOn(); // has to be last command in moduleOn();
}

void PilotJoystick::moduleOff(void)
{
    RackDataModule::moduleOff();        // has to be first command in moduleOff();

    chassis->moveCurve(0, joystickCurve);
    joystick->stopContData(&joystickMbx);

    if (scan2dInst >= 0) // scan2d is not used if id is -1
    {
        scan2d->stopContData(&scan2dMbx);
    }
}

// realtime context
int  PilotJoystick::moduleLoop(void)
{
    int         speed, speedSafe;
    int         speedSide;
    int         ret;
    float       omega;
    float       joystickPositionY;
    RackMessage jstkMsg;
    RackMessage s2dMsg;
    pilot_data* pilotData = NULL;
    // get all joystick messages

    jstkMsg.clear();
    do
    {
        ret = joystickMbx.recvDataMsgIf(&jstkData, sizeof(joystick_data),
                                        &jstkMsg);
        if (ret && ret != -EWOULDBLOCK) // error
            return ret;
    }
    while (!ret);

    // newest message received and mailbox is empty

    if ((jstkMsg.getSrc()  == RackName::create(joystickSys, JOYSTICK, joystickInst)) &&
        (jstkMsg.getType() == MSG_DATA))
    {
        // joystick data message received

        JoystickData::parse(&jstkMsg);

        if (joystickExpX <= 0.0f)
        {
            joystickSpeed = (int)rint((float)maxSpeed *
                                      ((float)jstkData.position.x / 100.0f));
        }
        else
        {
            joystickSpeed = (int)rint((float)maxSpeed *
                ((powf(2.0f, joystickExpX * (float)jstkData.position.x / 100.0f) - 1.0f)
                 / (powf(2.0f, joystickExpX) - 1.0f)));
        }

        joystickSpeedSide = (int)rint((float)maxSpeedSide *
                                     ((float)jstkData.position.z / 100.0f));

        if (joystickExpY <= 0.0f)
        {
            joystickPositionY = (float)jstkData.position.y / 100.0f;
        }
        else
        {
            if (jstkData.position.y < 0)
            {
                joystickPositionY = (powf(2.0f, joystickExpY * (float)-jstkData.position.y / 100.0f) - 1.0f)
                                    / (powf(2.0f, joystickExpY) - 1.0f);
                joystickPositionY = -joystickPositionY;
            }
            else
            {
                joystickPositionY = (powf(2.0f, joystickExpY * (float)jstkData.position.y / 100.0f) - 1.0f)
                                    / (powf(2.0f, joystickExpY) - 1.0f);
            }
        }

        switch(mode)
        {
        case 1:  // speed-omega
            joystickOmega = chasParData.omegaMax * joystickPositionY;
            break;
        case 0:  // speed-radius
        default:
            if (joystickPositionY != 0.0f)
            {
                joystickCurve = radius2Curve(chasParData.minTurningRadius) *
                                joystickPositionY;
            }
            else
            {
                joystickCurve = 0.0f;
            }
        }

        joystickForce = jstkData.buttons;
        joystikDataMissing = 0;
    }
    else
    {
        joystikDataMissing++;

        if (joystikDataMissing >= 10)
        {
            GDOS_ERROR("No data from Joystick(%i/%i)\n", joystickSys, joystickInst);
            return -ETIMEDOUT;
        }
    }

    if ((jstkMsg.getSrc()  == RackName::create(joystickSys, JOYSTICK, joystickInst)) &&
        (jstkMsg.getType() == MSG_ERROR))
    {
        GDOS_ERROR("Joystick(%i/%i) reported ERROR\n", joystickSys, joystickInst);
        return -EFAULT;
    }

    if (scan2dInst >= 0) // scan2d is not used if id is -1
    {
        s2dMsg.clear();
        do
        {
            ret = scan2dMbx.recvDataMsgIf(&scan2dMsg.data,
                                          sizeof(scan2d_data_msg), &s2dMsg);
            if (ret && ret != -EWOULDBLOCK) // error
                return ret;
        }
        while (!ret);

        // joystick data message received

        if ((s2dMsg.getSrc()  == RackName::create(scan2dSys, SCAN2D, scan2dInst)) &&
            (s2dMsg.getType() == MSG_DATA))
        {
            scan2dDataMissing = 0;
            Scan2dData::parse(&s2dMsg);
        }
        else
        {
            scan2dDataMissing++;

            if (scan2dDataMissing >= 10)
            {
                GDOS_ERROR("No data from Scan2D(%i/%i)\n", scan2dSys, scan2dInst);
                return -ETIMEDOUT;
            }
        }

        if ((s2dMsg.getSrc()  == RackName::create(scan2dSys, SCAN2D, scan2dInst)) &&
            (s2dMsg.getType() == MSG_ERROR))
        {
            GDOS_ERROR("Scan2D(%i/%i) reported ERROR\n", scan2dSys, scan2dInst);
            return -EFAULT;
        }

        // set speed and curve

        if(joystickForce == 0)
        {
            if(joystickSpeed >= 0)
            {
                speedSafe = safeSpeed(2 * chasParData.vxMax, curve2Radius(joystickCurve), NULL,
                                   &scan2dMsg.data, &chasParData);

                if(joystickSpeed < speedSafe)
                {
                    speed = joystickSpeed;
                }
                else
                {
                    speed = speedSafe;
                }
            }
            else
            {
                speedSafe = safeSpeed(-2 * chasParData.vxMax, curve2Radius(joystickCurve), NULL,
                                   &scan2dMsg.data, &chasParData);

                if(joystickSpeed > speedSafe)
                {
                    speed = joystickSpeed;
                }
                else
                {
                    speed = speedSafe;
                }
            }
        }
        else
        {
            if(joystickSpeed >= 0)
            {
                speed = joystickSpeed;
                speedSafe = 2 * chasParData.vxMax;
            }
            else
            {
                speed = joystickSpeed;
                speedSafe = -2 * chasParData.vxMax;
            }
        }
    }
    else // scan2d is not used if id is -1
    {
        speed  = joystickSpeed;
        speedSafe = chasParData.vxMax;
    }

    // calculate omega
    switch(mode)
    {
    case 1:  // speed-omega
        if(joystickSpeed != 0)
        {
            omega = joystickOmega * ((float)speed / (float)joystickSpeed);
        }
        else
        {
            omega = joystickOmega;
        }
        break;
    case 0:  // speed-radius
    default:
        omega = speed * joystickCurve;
    }

    // move chassis without lateral movement allowed
    if (maxSpeedSide == 0)
    {
        ret = chassis->move(speed, speedSafe, omega);
        if (ret)
        {
            GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
            return ret;
        }
    }
    // move chassis with lateral movement allowed
    else
    {
        speedSide = safeSpeedSide(joystickSpeedSide,  NULL,
                                  &scan2dMsg.data, &chasParData);

        ret = chassis->move(speed, speedSide, omega);
        if (ret)
        {
            GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
            return ret;
        }
    }


    // get datapointer from rackdatabuffer
    pilotData = (pilot_data *)getDataBufferWorkSpace();

    pilotData->recordingTime = rackTime.get();
    memset(&(pilotData->pos), 0, sizeof(pilotData->pos));
    memset(&(pilotData->dest), 0, sizeof(pilotData->dest));
    pilotData->speed     = speed;
    if(speed != 0)
    {
        pilotData->curve     = omega / (float)speed;
    }
    else
    {
        pilotData->curve     = 0.0f;
    }

    pilotData->distanceToDest = -1;
    pilotData->splineNum      = 1;

    memset(&(pilotData->spline[0].basepoint), 0, sizeof(pilotData->spline[0].basepoint));
    pilotData->spline[0].radius         = 0;
    pilotData->spline[0].length         = joystickSpeed;
    pilotData->spline[0].vMax           = joystickSpeed;
    pilotData->spline[0].vStart         = joystickSpeed;
    pilotData->spline[0].vEnd           = joystickSpeed;
    pilotData->spline[0].accMax         = chasParData.accMax;
    pilotData->spline[0].decMax         = chasParData.decMax;
    pilotData->spline[0].type           = 0;
    pilotData->spline[0].request        = 0;
    pilotData->spline[0].lbo            = 0;
    pilotData->spline[0].startPos.x     = 0;
    pilotData->spline[0].startPos.y     = 0;
    pilotData->spline[0].startPos.rho   = 0.0f;
    pilotData->spline[0].centerPos.x    = 0;
    pilotData->spline[0].centerPos.y    = 0;
    pilotData->spline[0].centerPos.rho  = 0.0f;
    pilotData->spline[0].endPos.x       = joystickSpeed;
    pilotData->spline[0].endPos.y       = 0;
    pilotData->spline[0].endPos.rho     = 0.0f;

/*    if (pilotData->curve > 0)
    {
        pilotData->spline[0].centerPos.y   = pilotData->spline[0].radius;
        pilotData->spline[0].centerPos.rho = -M_PI;
    }
    if (pilotData->curve < 0)
    {
        pilotData->spline[0].centerPos.y   = -pilotData->spline[0].radius;
        pilotData->spline[0].centerPos.rho = M_PI;
    }

    if (joystickSpeed != 0)
    {
        pilotData->spline[0].endPos.rho = normaliseAngle((float)pilotData->spline[0].radius / (float)joystickSpeed);
    }
    else
    {
        pilotData->spline[0].endPos.rho = 0.0f;
    }

    pilotData->spline[0].endPos.x   = (int)(pilotData->spline[0].radius * cos(pilotData->spline[0].endPos.rho));
    pilotData->spline[0].endPos.y   = (int)(pilotData->spline[0].radius * sin(pilotData->spline[0].endPos.rho));*/

    putDataBufferWorkSpace(sizeof(pilot_data) + pilotData->splineNum * sizeof(polar_spline));

    sleepDataBufferPeriodTime();

    return 0;
}

int  PilotJoystick::moduleCommand(RackMessage *msgInfo)
{
    switch(msgInfo->getType())
    {
        case MSG_PILOT_SET_DESTINATION:  // not supported by this module
            cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
            break;

        default:
            // not for me -> ask RackDataModule
            return RackDataModule::moduleCommand(msgInfo);
      }
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
#define INIT_BIT_DATA_MODULE        0
#define INIT_BIT_MBX_JOYSTICK       1
#define INIT_BIT_MBX_SCAN2D         2
#define INIT_BIT_MBX_WORK           3
#define INIT_BIT_PROXY_JOYSTICK     4
#define INIT_BIT_PROXY_SCAN2D       5
#define INIT_BIT_PROXY_CHASSIS      6

int  PilotJoystick::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    //
    // create mailboxes
    //

    // joystick
    ret = createMbx(&joystickMbx, 10, sizeof(joystick_data),
                    MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_JOYSTICK);

    // scan2d
    ret = createMbx(&scan2dMbx, 2, sizeof(scan2d_data_msg),
                    MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_SCAN2D);

    // work mailbox
    // -> gets chassis parameter package
    // -> gets joystick data ...
    ret = createMbx(&workMbx, 10, sizeof(chassis_param_data),
                    MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    //
    // create Proxys
    //

    // joystick -> use chassiskMbx to send commands
    joystick = new JoystickProxy(&workMbx, joystickSys, joystickInst);
    if (!joystick)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_JOYSTICK);

    // scan2d
    scan2d = new Scan2dProxy(&workMbx, scan2dSys, scan2dInst);
    if (!scan2d)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_SCAN2D);

    // chassis
    chassis = new ChassisProxy(&workMbx, chassisSys, chassisInst);
    if (!chassis)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_CHASSIS);

    return 0;

init_error:
    moduleCleanup();
    return ret;
}

void PilotJoystick::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    //
    // free proxies
    //

    if (initBits.testAndClearBit(INIT_BIT_PROXY_CHASSIS))
    {
        delete chassis;
    }

    if (initBits.testAndClearBit(INIT_BIT_PROXY_SCAN2D))
    {
        delete scan2d;
    }

    if (initBits.testAndClearBit(INIT_BIT_PROXY_JOYSTICK))
    {
        delete joystick;
    }

    //
    // delete mailboxes
    //

    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_SCAN2D))
    {
        destroyMbx(&scan2dMbx);
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_JOYSTICK))
    {
        destroyMbx(&joystickMbx);
    }
}

PilotJoystick::PilotJoystick()
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    240,              // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    5,                // max buffer entries
                    10)               // data buffer listener
{
    // get static module parameter
    chassisSys   = getIntArg("chassisSys", argTab);
    chassisInst  = getIntArg("chassisInst", argTab);
    scan2dSys    = getIntArg("scan2dSys", argTab);
    scan2dInst   = getIntArg("scan2dInst", argTab);
    joystickSys  = getIntArg("joystickSys", argTab);
    joystickInst = getIntArg("joystickInst", argTab);

    dataBufferMaxDataSize   = sizeof(pilot_data_msg);
    dataBufferPeriodTime    = 100; // 100ms (10 per sec)
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "PilotJoystick");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    PilotJoystick *pInst;

    // create new PilotJoystick
    pInst = new PilotJoystick();
    if (!pInst)
    {
        printf("Can't create new PilotJoystick -> EXIT\n");
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

