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
 *      YourName <YourMailAddress>
 *
 */
#include <iostream>

#include "dummy_abc.h"

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE                0
#define INIT_BIT_DO_SOMETHING               1

//
// data structures
//

DummyAbc *p_inst;

argTable_t argTab[] = {

    { ARGOPT_REQ,                                  // argument required
      "reqVal" ,                                   // name of argument
      ARGOPT_REQVAL,                               // this argument needs a value
      ARGOPT_VAL_INT,                              // value is an integer
      "new required int argument",                 // help string
      { -1 } },                                    // default value

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

int  DummyAbc::moduleOn(void)
{
    // do something ...

    return RackDataModule::moduleOn();   // has to be last command in moduleOn();
}

void DummyAbc::moduleOff(void)
{
    RackDataModule::moduleOff();         // has to be first command in moduleOff();

    // do something ...
}

int  DummyAbc::moduleLoop(void)
{
    dummy_data    *pData;
    uint32_t      datalength;

    // get datapointer from rackDataBuffer
    pData = (dummy_data*)getDataBufferWorkSpace();

    // do something and count data bytes ...

    pData->recordingTime = rackTime.get();
    pData->valA          = 0.1f;
    pData->valB          = 5;
    pData->value[0]      = 123;
    pData->value[1]      = 456;
    pData->valueNum      = 2;

    // write data buffer slot (and send it to all listener)
    datalength = sizeof(dummy_data) + pData->valueNum * sizeof(int32_t);
    putDataBufferWorkSpace(datalength);

    RackTask::sleep(1000000000ull);  // 1000ms = 1Hz

    return 0;
}

int  DummyAbc::moduleCommand(message_info *msgInfo)
{
    dummy_param *param;
    dummy_param replyParam;

    switch (msgInfo->type)
    {
        case DUMMY_SEND_CMD:
            GDOS_PRINT("handle DUMMY_SEND_CMD\n");
            // ...
            cmdMbx.sendMsgReply(MSG_OK, msgInfo);
            break;

        case DUMMY_SEND_PARAM:
            param = DummyParam::parse(msgInfo);
            GDOS_PRINT("handle DUMMY_SEND_PARAM valueX %f valueY %i\n", param->valX, param->valY);
            // ...
            cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
            break;

        case DUMMY_RECV_PARAM:
            GDOS_PRINT("handle DUMMY_RECV_PARAM\n");
            // ...
            cmdMbx.sendDataMsgReply(DUMMY_PARAM, msgInfo, 1, &replyParam,
                                    sizeof(replyParam));
            break;

        case DUMMY_SEND_RECV_PARAM:
            param = DummyParam::parse(msgInfo);
            GDOS_PRINT("handle DUMMY_SEND_RECV_PARAM valueX %f valueY %i\n", param->valX, param->valY);
            // ...
            cmdMbx.sendDataMsgReply(DUMMY_PARAM, msgInfo, 1, &replyParam,
                                    sizeof(replyParam));
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

int  DummyAbc::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    // !!! DON'T FORGET SETTING THE INIT BITS !!!
    // ON error -> goto init_error !!! (see below)

    ret = 0; // function call returns value ...
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_DO_SOMETHING);

    // create mailboxes
    // ...

    // create proxies
    // ...

    // init own stuff
    // -> open files, allocate buffers, ...

    return 0;

init_error:

    // !!! call local cleanup function !!!
    DummyAbc::moduleCleanup();
    return ret;
}

// non realtime context
void DummyAbc::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // free own stuff
    // ...

    // free proxies
    // ...

    // delete mailboxes
    // ...


    if (initBits.testAndClearBit(INIT_BIT_DO_SOMETHING))
    {
        // free something
    }
}

DummyAbc::DummyAbc()
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    5,                // max buffer entries
                    10)               // data buffer listener
{

  // get static module parameter
  reqVal        = getIntArg("reqVal", argTab);

  dataBufferMaxDataSize = sizeof(dummy_data_msg);
  dataBufferPeriodTime = 1000;  // 1000ms = 1Hz
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "DummyAbc");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new DummyAbc

    p_inst = new DummyAbc();
    if (!p_inst)
    {
        printf("Can't create new DummyAbc -> EXIT\n");
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
