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
 *
 *
 */
#include "scan2d_lab.h"

#include <main/argopts.h>

// init_flags
#define INIT_BIT_DATA_MODULE        0
#define INIT_BIT_MBX_WORK           1

//
// data structures
//

Scan2dLab *p_inst;

argTable_t argTab[] = {

    { 0, "", 0, 0, "", { 0 } } // last entry
};


/*******************************************************************************
 *   !!! REALTIME CONTEXT !!!
 *
 *   moduleOn,
 *   moduleOff,
 *   moduleLoop,
 *   moduleCommand,
 *   initDataBuffer
 *
 *   own realtime user functions
 ******************************************************************************/
int  Scan2dLab::moduleOn(void)
{
    return RackDataModule::moduleOn();  // has to be last command in moduleOn();
}

void Scan2dLab::moduleOff(void)
{
    RackDataModule::moduleOff();        // has to be first command in moduleOff();
}


// realtime context
int  Scan2dLab::moduleLoop(void)
{
    scan2d_data     *scanOutputData;
//    int             ret;
//    message_info    dataInfo;

    // get datapointer from rackdatabuffer
    scanOutputData = (scan2d_data *)getDataBufferWorkSpace();

    scanOutputData->pointNum = 0;

    putDataBufferWorkSpace(Scan2dData::getDatalen(scanOutputData));

    return 0;
}

int  Scan2dLab::moduleCommand(message_info *msgInfo)
{
    // not for me -> ask RackDataModule
    return RackDataModule::moduleCommand(msgInfo);
}

/*******************************************************************************
 *   !!! NON REALTIME CONTEXT !!!
 *
 *   own non realtime user functions,
 *   moduleInit,
 *   moduleCleanup,
 *   Constructor,
 *   Destructor,
 *   main
 *
 ******************************************************************************/
int Scan2dLab::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    GDOS_DBG_DETAIL("Scan2dLab::moduleInit ... \n");

    //
    // create mailboxes
    //

    // work Mbx
    ret = createMbx(&workMbx, 1, 128, MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    //
    // create Proxies
    //

    return 0;

init_error:

    // !!! call local cleanup function !!!
    Scan2dLab::moduleCleanup();
    return ret;
}


// non realtime context
void Scan2dLab::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // delete work mailbox
    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }
}

Scan2dLab::Scan2dLab(void)
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    10,               // max buffer entries
                    10)               // data buffer listener
{
    // get values

    dataBufferMaxDataSize   = sizeof(scan2d_data_msg);
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "Scan2dLab");
    if (ret)
    {
       printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new Scan2dLab
    p_inst = new Scan2dLab();
    if (!p_inst)
    {
        printf("Can't create new Scan2dLab -> EXIT\n");
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
