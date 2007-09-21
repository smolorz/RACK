/*
 * RACK-RTS - Robotics Application Construction Kit (RTS internal)
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * All rights reserved.
 *
 * Authors
 *
 */

#include "pilot_lab.h"

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE        0
#define INIT_BIT_MBX_WORK           1
#define INIT_BIT_PROXY_CHASSIS      2

// data structures
//

PilotLab *p_inst;

argTable_t argTab[] = {

    { ARGOPT_OPT, "chassisInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the chassis module", { 0 } },

    { ARGOPT_OPT, "maxSpeed", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Maximum Speed, default 500", { 500 } },

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

 int  PilotLab::moduleOn(void)
{
    int ret;

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

    GDOS_PRINT("maxSpeed %f m/s,\n",
                (float)maxSpeed / 1000.0f);

    return RackDataModule::moduleOn(); // has to be last command in moduleOn();
}

void PilotLab::moduleOff(void)
{
    RackDataModule::moduleOff();        // has to be first command in moduleOff();

    chassis->moveRadius(0, 0);
}

// realtime context
int  PilotLab::moduleLoop(void)
{
    int          speed = 0;
    int          ret;
    message_info msgInfo;
    pilot_data*  pilotData = NULL;

    // get datapointer from rackdatabuffer
    pilotData = (pilot_data *)getDataBufferWorkSpace();

    pilotData->recordingTime = rackTime.get();
    memset(&(pilotData->pos), 0, sizeof(pilotData->pos));
    memset(&(pilotData->dest), 0, sizeof(pilotData->dest));
    pilotData->speed     = speed;

    pilotData->curve     = 0.0f;

    pilotData->distanceToDest = -1;
    pilotData->splineNum      = 0;

    putDataBufferWorkSpace(sizeof(pilot_data));

    RackTask::sleep(rackTime.toNano(getDataBufferPeriodTime(0)));

    return 0;
}

int  PilotLab::moduleCommand(message_info *msgInfo)
{
    // not for me -> ask RackDataModule
    return RackDataModule::moduleCommand(msgInfo);
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

int  PilotLab::moduleInit(void)
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

    // work mailbox
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

void PilotLab::moduleCleanup(void)
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

    //
    // delete mailboxes
    //

    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }
}

PilotLab::PilotLab()
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    5,                // max buffer entries
                    10)               // data buffer listener
{
    // get value(s) out of your argument table
    chassisInst  = getIntArg("chassisInst", argTab);
    maxSpeed     = getIntArg("maxSpeed", argTab);

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(pilot_data_msg));

    setDataBufferPeriodTime(100);
}


int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "PilotLab");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new PilotLab
    p_inst = new PilotLab();
    if (!p_inst)
    {
        printf("Can't create new PilotLab -> EXIT\n");
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
