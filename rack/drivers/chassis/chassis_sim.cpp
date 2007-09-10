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

    { ARGOPT_OPT, "vxMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "max vehicle velocity in x direction, default 700 m/s", { 700 } },

    { ARGOPT_OPT, "vxMin", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "min vehicle velocity in x direction, default 50 m/s)", { 50 } },

    { ARGOPT_OPT, "axMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "max vehicle acceleration in x direction, default 500", { 500 } },

    { ARGOPT_OPT, "omegaMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "omegaMax, default 20 deg/s", { 20 } },

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
        "1 / sampling rate in ms, default 100", { 100 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};


// vehicle parameter

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

    return RackDataModule::moduleOn(); // has to be last command in moduleOn();
}

void ChassisSim::moduleOff(void)
{
    RackDataModule::moduleOff();       // has to be first command in moduleOff();

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
    p_data->deltaX        = p_data->vx * (float)periodTime / 1000.0f;       // in mm
    p_data->deltaY        = p_data->vy * (float)periodTime / 1000.0f;       // in mm
    p_data->deltaRho      = p_data->omega * (float)periodTime / 1000.0f;    // in rad
    p_data->battery       = 0.0f;
    p_data->activePilot   = activePilot;

    mtx.unlock();

    datalength = sizeof(chassis_data);

    putDataBufferWorkSpace( datalength );

    GDOS_DBG_DETAIL("timestamp offset: %u\n",(unsigned long) rackTime.getOffset());
   
    GDOS_DBG_DETAIL("vx:%f mm/s, vx:%f mm/s, omega:%a deg/s, timestamp: %d\n",
                    p_data->vx, p_data->vy, p_data->omega,
                    p_data->recordingTime);

    RackTask::sleep(periodTime * 1000000llu);

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

    // call RackDataModule init function
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        goto init_error;
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
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // destroy mutex
    if (initBits.testAndClearBit(INIT_BIT_MTX_CREATED))
    {
        mtx.destroy();
    }
}

ChassisSim::ChassisSim()
        : RackDataModule( MODULE_CLASS_ID,
                      5000000000llu,    // 5s datatask error sleep time
                      16,               // command mailbox slots
                      48,               // command mailbox data size per slot
                      MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                      5,                // max buffer entries
                      10)               // data buffer listener
{
    // get value(s) out of your argument table
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
    periodTime              = getIntArg("periodTime", argTab);

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(chassis_data));

    // set databuffer period time
    setDataBufferPeriodTime(periodTime); // default 10Hz
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

