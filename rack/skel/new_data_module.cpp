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

// include own header file
#include "new_data_module.h"

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE                0
#define INIT_BIT_DO_SOMETHING               1

//
// data structures
//

NewDataModule *p_inst;

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

int  NewDataModule::moduleOn(void)
{
    // do something ...

    return DataModule::moduleOn();   // have to be last command in moduleOn();
}

void NewDataModule::moduleOff(void)
{
    DataModule::moduleOff();         // have to be first command in moduleOff();

    // do something ...
}

int  NewDataModule::moduleLoop(void)
{
    new_data_msg  *p_data     = NULL;
    uint32_t      datalength = 0;

    // get datapointer from rackdatabuffer
    // you don't need check this pointer
    p_data = (new_data_msg *)getDataBufferWorkSpace();

    // do something and count data bytes ...

    p_data->data.recordingTime = rackTime.get();
    datalength = sizeof(new_data_msg);

    // write data buffer slot (and send it to all listener)
    if (datalength > 0 && datalength <= getDataBufferMaxDataSize() )
    {
        putDataBufferWorkSpace( datalength );
        return 0;
    }
    return -ENOSPC;
}

int  NewDataModule::moduleCommand(MessageInfo *msgInfo)
{
    send_data *p_data;
    switch (msgInfo->type)
    {
        case MSG_SEND_CMD:
            GDOS_PRINT("NewDataModule: handle MSG_SEND_CMD\n");
            // ...
            break;

        case MSG_SEND_DATA_CMD:
            GDOS_PRINT("NewDataModule: handle MSG_SEND_DATA_CMD\n");
            p_data = SendData::parse(msgInfo);
            // ...
            break;

        case MSG_RECV_DATA_CMD:
            GDOS_PRINT("NewDataModule: handle MSG_RECV_DATA_CMD\n");
            // ...
            break;

        case MSG_SEND_RECV_DATA_CMD:
            GDOS_PRINT("NewDataModule: handle MSG_SEND_RECV_DATA_CMD\n");
            p_data = SendData::parse(msgInfo);
            // ...
            break;

        default:
            // not for me -> ask DataModule
            return DataModule::moduleCommand(msgInfo);
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

int  NewDataModule::moduleInit(void)
{
    int ret;

    // call DataModule init function (first command in init)
    ret = DataModule::moduleInit();
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
    NewDataModule::moduleCleanup();
    return ret;
}

// non realtime context
void NewDataModule::moduleCleanup(void)
{
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

    // call DataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        DataModule::moduleCleanup();
    }
}

NewDataModule::NewDataModule()
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
  reqVal        = getIntArg("reqVal", argTab);

  // set dataBuffer size
  setDataBufferMaxDataSize(sizeof(new_data_msg));

  // set databuffer period time
  setDataBufferPeriodTime(10);
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = Module::getArgs(argc, argv, argTab, "NewDataModule");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new NewDataModule

    p_inst = new NewDataModule();
    if (!p_inst)
    {
        printf("Can't create new NewDataModule -> EXIT\n");
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
