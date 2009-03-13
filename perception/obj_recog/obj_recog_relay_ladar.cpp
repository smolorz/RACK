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
 *      Matthias Hentschel      <hentschel@rts.uni-hannover.de>
 *
 */

#include "obj_recog_relay_ladar.h"

// init_flags
#define INIT_BIT_DATA_MODULE            0
#define INIT_BIT_MBX_WORK               1
#define INIT_BIT_MBX_DATA               2
#define INIT_BIT_PROXY_POSITION         3

//
// init data structures
//

ObjRecogRelayLadar *p_inst;

argTable_t argTab[] = {
    { ARGOPT_OPT, "maxDataSize", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The maximal size of the cameradata, default (max)", { sizeof(obj_recog_data_msg) } },

    {ARGOPT_OPT, "periodTime", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The period time of the source in ms, default vertical (1s)", { 1000 } },

    {ARGOPT_OPT, "positionInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the position module (default -1)", { -1 } },

    { ARGOPT_OPT, "ladarOffsetX", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Ladar X offset (default 0)", { 0 } },

    { ARGOPT_OPT, "ladarOffsetY", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Ladar Y offset (default 0)", { 0 } },

    {0,"",0,0,""}
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
int  ObjRecogRelayLadar::moduleOn(void)
{
    int ret;

    // get dynamic module parameter
    ladarOffsetX          = getInt32Param("ladarOffsetX");
    ladarOffsetY          = getInt32Param("ladarOffsetY");

    // position
    if (positionInst >= 0)
    {
        ret = position->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on Position(%d), code = %d\n", positionInst, ret);
            return ret;
        }
    }

    return RackDataModule::moduleOn();  // has to be last command in moduleOn();
}

void ObjRecogRelayLadar::moduleOff(void)
{
    RackDataModule::moduleOff();          // has to be first command in moduleOff();
}

int  ObjRecogRelayLadar::moduleLoop(void)
{

    int             ret       = 0;
    int             i;
    message_info    msgInfo;
    position_data   positionData;
    obj_recog_data  *p_data   = (obj_recog_data *)getDataBufferWorkSpace();


    ret = dataMbx.recvDataMsgTimed(this->dataTimeOut, p_data,
                                   dataBufferMaxDataSize, &msgInfo);
    if (ret)
    {
        GDOS_ERROR("Can't receive data on objRecog MBX, code = %d\n", ret);
        return ret;
    }


    if (msgInfo.type != MSG_DATA)
    {
        GDOS_ERROR("Received unexpected message from %x to %x, type %d "
                   "on objRecogMbx\n", msgInfo.src, msgInfo.dest, msgInfo.type);
        return -ECOMM;
    }
    ObjRecogData::parse(&msgInfo);

    // add reference position
    if (positionInst >= 0)
    {
        ret = position->getData(&positionData, sizeof(position_data), 0);
        if (ret)
        {
            GDOS_ERROR("Can't get data from Position(%d), code = %d\n", positionInst, ret);
            return ret;
        }
        GDOS_DBG_DETAIL("Received position data, recordingTime %d, pos %d %d %a\n",
                        positionData.recordingTime, positionData.pos.x, positionData.pos.y,
                        positionData.pos.rho);
        memcpy(&p_data->refPos, &positionData.pos, sizeof(position_3d));
    }

    // add offsets
    for (i = 0; i < p_data->objectNum; i++)
    {
        p_data->object[i].pos.x += ladarOffsetX;
        p_data->object[i].pos.y += ladarOffsetY;
    }

    putDataBufferWorkSpace(msgInfo.datalen);
    return 0;
}

int  ObjRecogRelayLadar::moduleCommand(message_info *p_msginfo)
{
  // not for me -> ask RackDataModule
  return RackDataModule::moduleCommand(p_msginfo);
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

int  ObjRecogRelayLadar::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    // data mailbox
    ret = createMbx(&dataMbx, 1, dataBufferMaxDataSize,
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_DATA);

    // work mailbox
    ret = createMbx(&workMbx, 10, sizeof(position_data),
                    MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // position proxy
    if (positionInst >= 0)
    {
        position = new PositionProxy(&workMbx, 0, positionInst);
        if (!position)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_POSITION);
    }

    return 0;

init_error:
    // !!! call local cleanup function !!!
    ObjRecogRelayLadar::moduleCleanup();
    return ret;
}

void ObjRecogRelayLadar::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
        RackDataModule::moduleCleanup();

    if (positionInst >= 0)
    {
        if (initBits.testAndClearBit(INIT_BIT_PROXY_POSITION))
        {
            delete position;
        }
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_DATA))
    {
        destroyMbx(&dataMbx);
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }
}

ObjRecogRelayLadar::ObjRecogRelayLadar(void)
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    128,              // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    10,               // max buffer entries
                    10)               // data buffer listener


{
    // get static module parameter
    size_t maxDataSize     = getIntArg("maxDataSize", argTab);
    rack_time_t periodTime = getIntArg("periodTime", argTab);
    positionInst           = getIntArg("positionInst", argTab);

    this->inited = false;

    if ((maxDataSize < 0) || (maxDataSize > sizeof(obj_recog_data_msg)))
    {
           GDOS_ERROR("Invalid image size %d\n", maxDataSize);
           return;
    }

    if (periodTime < 0)
    {
           GDOS_ERROR("Invalid period time %dms\n", periodTime);
           return;
    }

    this->dataTimeOut = periodTime * 2;
    this->dataTimeOut *= RACK_TIME_FACTOR;

    dataBufferMaxDataSize   = maxDataSize;
    dataBufferPeriodTime    = periodTime;

    this->inited = true;
};

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "ObjRecogRelayLadar");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new ObjRecogRelayLadar
    p_inst = new ObjRecogRelayLadar();
    if (!p_inst || !p_inst->inited)
    {
        printf("Can't create new ObjRecogRelayLadar -> EXIT\n");
        return -ENOMEM;
    }

    ret = p_inst->moduleInit();
    if (ret)
        goto exit_error;

    p_inst->run();
    return 0;

exit_error:

    delete (p_inst);
    return ret;
}
