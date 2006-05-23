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
#include "pilot_joystick.h"

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE        0
#define INIT_BIT_MBX_JOYSTICK       1
#define INIT_BIT_MBX_SCAN2D         2
#define INIT_BIT_MBX_WORK           3
#define INIT_BIT_PROXY_JOYSTICK     4
#define INIT_BIT_PROXY_SCAN2D       5
#define INIT_BIT_PROXY_CHASSIS      6

//
// data structures
//

PilotJoystick *p_inst;

argTable_t argTab[] = {

    { ARGOPT_REQ, "scan2dInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the Scan2D module", { -1 } },

    { ARGOPT_OPT, "chassisInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the chassis module", { 0 } },

    { ARGOPT_OPT, "joystickInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the Joystick module ", { 0 } },

    { ARGOPT_OPT, "joystickSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the Joystick module ", { 0 } },

    { ARGOPT_OPT, "maxSpeed", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Maximal Speed, default 4000", { 4000 } },

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
    MessageInfo info;

    ret = chassis->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on chassis, code = %d\n", ret);
        return ret;
    }

    // get chassis parameter data

    ret = chassis->getParam(&chasParData, sizeof(chassis_param_data), &info);
    if (ret)
    {
        GDOS_ERROR("Can't get chassis parameter, code = %d\n", ret);
        return ret;
    }

    ChassisParamData::parse(&info);

    if (maxSpeed > chasParData.vxMax)
    {
        maxSpeed = chasParData.vxMax;
        GDOS_DBG_DETAIL("Max speed : %d\n", maxSpeed);
    }

    // enable joystick

    ret = joystick->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on joystick(%i), code = %d\n", joystickInst, ret);
        return ret;
    }

    // get joystick data

    ret = joystick->getData(&jstkData, sizeof(joystick_data), 0, &info);
    if (ret)
    {
        GDOS_ERROR("Can't get data from Joystick(%i), code  %d\n",
                   joystickInst, ret);
        return ret;
    }

    JoystickData::parse(&info);

    if ((jstkData.position.x != 0) ||
        (jstkData.position.y != 0))
    {
        GDOS_ERROR("Joystick(%i) must be (0,0) to turn pilot on\n", joystickInst);
        return -EINVAL;
    }

    // enable scan2d module

    if (scan2dInst >= 0) // scan2d is not used if id is -1
    {
        ret = scan2d->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on Scan2D(%i), code = %d\n",
                       scan2dInst, ret);
            return ret;
        }
    }

    // get continuous data from joystick

    joystickMbx.clean();

    ret = joystick->getContData(0, &joystickMbx, NULL);
    if (ret)
    {
        GDOS_ERROR("Can't get continuous data from Joystick(%i), code = %d\n",
                   joystickInst, ret);
        return ret;
    }

    joystickSpeed = 0;
    joystickCurve = 0.0f;
    joystikDataMissing = 0;

    if (scan2dInst >= 0) // scan2d is not used if id is -1
    {
        scan2dMbx.clean();

        ret = scan2d->getContData(0, &scan2dMbx, NULL);
        if (ret)
        {
            GDOS_ERROR("Can't get continuous data from scan2D(%i), "
                       "code = %d\n", scan2dInst, ret);
            return ret;
        }

        scan2dMsg.data.pointNum = 0;
        scan2dDataMissing = 0;
    }

    GDOS_PRINT("maxSpeed %f m/s, minRadius %f m  scan2D(%i)\n",
                (float)maxSpeed / 1000.0f,
                (float)chasParData.minTurningRadius / 1000.0f, scan2dInst);

    return DataModule::moduleOn(); // have to be last command in moduleOn();
}

void PilotJoystick::moduleOff(void)
{
    DataModule::moduleOff();        // have to be first command in moduleOff();

    chassis->moveCurve(0,0.0f);
    joystick->stopContData(&joystickMbx);

    if (scan2dInst >= 0) // scan2d is not used if id is -1
    {
        scan2d->stopContData(&scan2dMbx);
    }
}

// realtime context
int  PilotJoystick::moduleLoop(void)
{
    int         speed;
    float       curve;
    int         ret;
    MessageInfo jstkInfo;
    MessageInfo s2dInfo;
    pilot_data*    pilotData = NULL;
    // get all joystick messages

    jstkInfo.type = 0;
    do
    {
        ret = joystickMbx.recvDataMsgIf(&jstkData, sizeof(joystick_data),
                                        &jstkInfo);
        if (ret && ret != -EWOULDBLOCK) // error
            return ret;
    }
    while (!ret);

    // newest message received and mailbox is empty

    if ((jstkInfo.src  == RackName::create(JOYSTICK, joystickInst)) &&
        (jstkInfo.type == MSG_DATA))
    {
        // joystick data message received

        JoystickData::parse(&jstkInfo);

        joystickSpeed = (int)rint((float)maxSpeed *
                                  ((float)jstkData.position.x / 100.0f));

        if (jstkData.position.y != 0)
        {
            joystickCurve = radius2Curve(chasParData.minTurningRadius) *
                            ((float)jstkData.position.y / 100.0f);
        }
        else
        {
            joystickCurve = 0.0f;
        }

           joystikDataMissing = 0;
    }
    else
    {
        joystikDataMissing++;

        if (joystikDataMissing >= 10)
        {
            GDOS_ERROR("No data from Joystick(%i)\n", joystickInst);
            return -ETIMEDOUT;
        }
    }

    if ((jstkInfo.src  == RackName::create(JOYSTICK, joystickInst)) &&
        (jstkInfo.type == MSG_ERROR))
    {
        GDOS_ERROR("Joystick(%i) reported ERROR\n", joystickInst);
        return -EFAULT;
    }

    if (scan2dInst >= 0) // scan2d is not used if id is -1
    {
        s2dInfo.type = 0;
        do
        {
            ret = scan2dMbx.recvDataMsgIf(&scan2dMsg.data,
                                          sizeof(scan2d_data_msg), &s2dInfo);
            if (ret && ret != -EWOULDBLOCK) // error
                return ret;
        }
        while (!ret);

        // joystick data message received

        if ((s2dInfo.src  == RackName::create(SCAN2D, scan2dInst)) &&
            (s2dInfo.type == MSG_DATA))
        {
            scan2dDataMissing = 0;
            Scan2DData::parse(&s2dInfo);
        }
        else
        {
            scan2dDataMissing++;

            if (scan2dDataMissing >= 10)
            {
                GDOS_ERROR("No data from Scan2D(%i)\n", scan2dInst);
                return -ETIMEDOUT;
            }
        }

        if ((s2dInfo.src  == RackName::create(SCAN2D, scan2dInst)) &&
            (s2dInfo.type == MSG_ERROR))
        {
            GDOS_ERROR("Scan2D(%i) reported ERROR\n", scan2dInst);
            return -EFAULT;
        }

        // scan2d data message received

        speed  = safeSpeed(joystickSpeed, curve2Radius(joystickCurve), NULL,
                           &scan2dMsg.data, &chasParData);
        curve  = joystickCurve;
    }
    else // scan2d is not used if id is -1
    {
        speed  = joystickSpeed;
        curve  = joystickCurve;
    }

    // move chassis
    ret = chassis->moveCurve(speed, curve);
    if (ret)
    {
        GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
        return ret;
    }


    // get datapointer from rackdatabuffer
    pilotData = (pilot_data *)getDataBufferWorkSpace();

    pilotData->recordingTime = rackTime.get();
    memset(&(pilotData->pos), 0, sizeof(pilotData->pos));
    pilotData->speed     = speed;
    pilotData->curve     = curve;
    pilotData->splineNum = 0;

    putDataBufferWorkSpace(sizeof(pilot_data));

    RackTask::sleep(rackTime.toNano(getDataBufferPeriodTime(0)));

    return 0;
}

int  PilotJoystick::moduleCommand(MessageInfo *msgInfo)
{
    // not for me -> ask DataModule
    return DataModule::moduleCommand(msgInfo);
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

int  PilotJoystick::moduleInit(void)
{
    int ret;

    // call DataModule init function (first command in init)
    ret = DataModule::moduleInit();
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
    scan2d = new Scan2DProxy(&workMbx, 0, scan2dInst);
    if (!scan2d)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_SCAN2D);

    // chassis
    chassis = new ChassisProxy(&workMbx, 0, chassisInst);
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

    // call DataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        DataModule::moduleCleanup();
    }
}

PilotJoystick::PilotJoystick()
      : DataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s cmdtask error sleep time
                    5000000000llu,    // 5s datatask error sleep time
                     100000000llu,    // 100ms datatask disable sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    5,                // max buffer entries
                    10)               // data buffer listener
{
    // get value(s) out of your argument table
    chassisInst  = getIntArg("chassisInst", argTab);
    scan2dInst   = getIntArg("scan2dInst", argTab);
    joystickInst = getIntArg("joystickInst", argTab);
    joystickSys  = getIntArg("joystickSys", argTab);
    maxSpeed     = getIntArg("maxSpeed", argTab);

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(pilot_data_msg));

    // set databuffer period time (preset)
    setDataBufferPeriodTime(100); // 100ms (10 per sec)
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = Module::getArgs(argc, argv, argTab, "PilotJoystick");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new PilotJoystick
    p_inst = new PilotJoystick();
    if (!p_inst)
    {
        printf("Can't create new PilotJoystick -> EXIT\n");
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

