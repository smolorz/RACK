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

#include "chassis_sim.h"

//
// data structures
//

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE                0
#define INIT_BIT_MTX_CREATED                1

ChassisSim *p_inst;

argTable_t argTab[] = {

    { 0, "", 0, 0, "", { 0 } } // last entry

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

int ChassisSim::moduleOn(void)
{
    commandData.vx    = 0;  // in mm/s
    commandData.vy    = 0;  // in mm/s
    commandData.omega = 0;  // in mm
    activePilot = CHASSIS_INVAL_PILOT;

    return RackDataModule::moduleOn(); // have to be last command in moduleOn();
}

void ChassisSim::moduleOff(void)
{
    RackDataModule::moduleOff();       // have to be first command in moduleOff();

    activePilot = CHASSIS_INVAL_PILOT;
}

int ChassisSim::moduleLoop(void)
{
    chassis_data* p_data = NULL;
    ssize_t datalength = 0;

    // get datapointer from rackdatabuffer
    p_data = (chassis_data *)getDataBufferWorkSpace();

    mtx.lock(RACK_INFINITE);

    p_data->recordingTime = rackTime.get();
    p_data->vx            = (float)commandData.vx;    // in mm/s
    p_data->vy            = (float)commandData.vy;    // in mm/s
    p_data->omega         = (float)commandData.omega; // in rad/s
    p_data->deltaX        = p_data->vx / 10.0f;       // in mm
    p_data->deltaY        = p_data->vy / 10.0f;       // in mm
    p_data->deltaRho      = p_data->omega / 10.0f;    // in rad
    p_data->battery       = 0.0f;
    p_data->activePilot   = activePilot;

    mtx.unlock();

    datalength = sizeof(chassis_data);

    putDataBufferWorkSpace( datalength );

    GDOS_DBG_DETAIL("vx:%f mm/s, vx:%f mm/s, omega:%a deg/s, timestamp: %d\n",
                    p_data->vx, p_data->vy, p_data->omega,
                    p_data->recordingTime);

    RackTask::sleep(100000000llu);

    return 0;
}

int ChassisSim::moduleCommand(message_info *msgInfo)
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

            if (((msgInfo->src & pilot_mask) == RackName::create(GUI, 0)) ||
                ((msgInfo->src & pilot_mask) == RackName::create(PILOT, 0)))
            {
                mtx.lock(RACK_INFINITE);

                activePilot = p_pilot->activePilot & pilot_mask;

                commandData.vx    = 0;
                commandData.vy    = 0;
                commandData.omega = 0;

                mtx.unlock();

                GDOS_DBG_INFO("%x Changed active pilot to %x",
                            msgInfo->src, activePilot);
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

int ChassisSim::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    // create mutex
    ret = mtx.create();
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MTX_CREATED);

    return 0;

init_error:
    // !!! call local cleanup function !!!
    ChassisSim::moduleCleanup();
    return ret;
}

void ChassisSim::moduleCleanup(void)
{
    // destroy mutex
    if (initBits.testAndClearBit(INIT_BIT_MTX_CREATED))
    {
        mtx.destroy();
    }

    // call RackDataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }
}

ChassisSim::ChassisSim()
        : RackDataModule( MODULE_CLASS_ID,
                      5000000000llu,    // 5s cmdtask error sleep time
                      5000000000llu,    // 5s datatask error sleep time
                      100000000llu,     // 100ms datatask disable sleep time
                      16,               // command mailbox slots
                      48,               // command mailbox data size per slot
                      MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                      5,                // max buffer entries
                      10)               // data buffer listener
{
    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(chassis_data));

    // set databuffer period time
    setDataBufferPeriodTime(100); // 100 ms (10 per sec)
}

int main(int argc, char *argv[])
{
    int ret;


    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "ChassisSim");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new ChassisSim

    p_inst = new ChassisSim();
    if (!p_inst)
    {
        printf("Can't create new ChassisSim -> EXIT\n");
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

