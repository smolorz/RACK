/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#include "position.h"
#include <main/angle_tool.h>

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE            0
#define INIT_BIT_MBX_ODOMETRY           1
#define INIT_BIT_MBX_WORK               2
#define INIT_BIT_PROXY_ODOMETRY         3
#define INIT_BIT_MTX_CREATED            4

//
// data structures
//

Position *p_inst;

argTable_t argTab[] = {

    { ARGOPT_OPT, "odometryInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the odometry module", { 0 } },

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

int  Position::moduleOn(void)
{
    int       ret = 0;
    rack_time_t realPeriodTime = 0;

    ret = odometry->on();
    if (ret)
    {
        GDOS_ERROR("Can't switch on odometry(%i), code = %d\n", odometryInst, ret);
        return ret;
    }

    ret = odometry->getContData(getDataBufferPeriodTime(0), &odometryMbx, &realPeriodTime);
    if (ret)
    {
        GDOS_ERROR("Can't get continuous data from odometry(%i) module, "
                   "code = %d\n", odometryInst, ret);
        return ret;
    }

    setDataBufferPeriodTime(realPeriodTime);

    return DataModule::moduleOn();    // have to be last command in moduleOn();
}

void Position::moduleOff(void)
{
    DataModule::moduleOff();          // have to be first command in moduleOff();

    odometry->stopContData(&odometryMbx);
}


int  Position::moduleLoop(void)
{
    int             ret;
    position_data*  pPosition;
    odometry_data   odometryData;
    message_info     msgInfo;
    position_3d     relPos;

    pPosition = (position_data *)getDataBufferWorkSpace();

    ret = odometryMbx.recvDataMsgTimed(getDataBufferPeriodTime(0) * 2000000llu, &odometryData,
                                      sizeof(odometryData), &msgInfo);
    if (ret)
    {
        GDOS_ERROR("Can't read odometry(%i) data, code = %d\n", odometryInst, ret);
        return ret;
    }

    if (msgInfo.type == MSG_DATA &&
        msgInfo.src  == odometry->getDestAdr())
    {
        OdometryData::parse(&msgInfo);

        refPosMtx.lock(RACK_INFINITE);

        // calculate relative position to refference position
        odometryData.pos.x = odometryData.pos.x - refOdo.x;
        odometryData.pos.y = odometryData.pos.y - refOdo.y;
        relPos.x   = (int)(+ cosRefOdo * odometryData.pos.x + sinRefOdo * odometryData.pos.y);
        relPos.y   = (int)(- sinRefOdo * odometryData.pos.x + cosRefOdo * odometryData.pos.y);
        relPos.z   = odometryData.pos.z   - refOdo.z;
        relPos.rho = odometryData.pos.rho - refOdo.rho;

        GDOS_DBG_DETAIL("Relative position x %i y %i z %i rho %a\n",
                        relPos.x, relPos.y, relPos.z, relPos.rho);

        // calcualte absolute position
        pPosition->pos.x         = refPos.x   + (int)(cosRefPos * relPos.x - sinRefPos * relPos.y);
        pPosition->pos.y         = refPos.y   + (int)(sinRefPos * relPos.x + cosRefPos * relPos.y);
        pPosition->pos.z         = refPos.z   + relPos.z;
        pPosition->pos.phi       = odometryData.pos.phi;
        pPosition->pos.psi       = odometryData.pos.psi;
        pPosition->pos.rho       = refPos.rho + relPos.rho;

        pPosition->recordingTime = odometryData.recordingTime;

        GDOS_DBG_DETAIL("recordingTime %i x %i y %i z %i phi %a psi %a rho %a\n",
                        pPosition->recordingTime,
                        pPosition->pos.x, pPosition->pos.y, pPosition->pos.z,
                        pPosition->pos.phi, pPosition->pos.psi, pPosition->pos.rho);

        refPosMtx.unlock();

        putDataBufferWorkSpace(sizeof(position_data));
    }
    else
    {
        GDOS_ERROR("Received unexpected message from %n to %n, type %d\n",
                   msgInfo.src, msgInfo.dest, msgInfo.type);

        if (msgInfo.type > 0)
        {
            odometryMbx.sendMsgReply(MSG_ERROR, &msgInfo);
        }
        return -ECOMM;
    }
    return 0;
}

int  Position::moduleCommand(message_info *msgInfo)
{
    position_data *pUpdate;
    odometry_data odometryData;
    int ret;

    switch(msgInfo->type)
    {
        case MSG_POSITION_UPDATE:
            pUpdate = PositionData::parse(msgInfo);

            ret = odometry->getData(&odometryData,
                                    sizeof(odometry_data),
                                    pUpdate->recordingTime);
            if (ret)
            {
                cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
            }

            refPosMtx.lock(RACK_INFINITE);

            refOdo.x   = odometryData.pos.x;
            refOdo.y   = odometryData.pos.y;
            refOdo.z   = odometryData.pos.z;
            refOdo.phi = 0.0f;
            refOdo.psi = 0.0f;
            refOdo.rho = odometryData.pos.rho;
            sinRefOdo  = sin(refOdo.rho);
            cosRefOdo  = cos(refOdo.rho);

            refPos.x   = pUpdate->pos.x;
            refPos.y   = pUpdate->pos.y;
            refPos.z   = pUpdate->pos.z;
            refPos.phi = 0.0f;
            refPos.psi = 0.0f;
            refPos.rho = pUpdate->pos.rho;
            sinRefPos  = sin(refPos.rho);
            cosRefPos  = cos(refPos.rho);

            refTime    = pUpdate->recordingTime;

            refPosMtx.unlock();
            GDOS_DBG_INFO("recordingTime %i x %i y %i z %i phi %a psi %a rho %a\n",
                           refTime, refPos.x, refPos.y, refPos.z, 
                           refPos.phi, refPos.psi, refPos.rho);

            cmdMbx.sendMsgReply(MSG_OK, msgInfo);
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

int  Position::moduleInit(void)
{
    int ret;

    // call DataModule init function (first command in init)
    ret = DataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    // work mailbox
    ret = createMbx(&workMbx, 1, 128, MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // odometry mailbox
    ret = createMbx(&odometryMbx, 4, sizeof(odometry_data),
                    MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_ODOMETRY);

    // odometry
    odometry = new OdometryProxy(&workMbx, 0, odometryInst);
    if (!odometry)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_ODOMETRY);

    // create refPos mutex
    ret = refPosMtx.create();
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MTX_CREATED);

    return 0;

init_error:
    moduleCleanup();
    return ret;
}

void Position::moduleCleanup(void)
{
    // destroy mutex
    if (initBits.testAndClearBit(INIT_BIT_MTX_CREATED))
    {
        refPosMtx.destroy();
    }

    // free proxy
    if (initBits.testAndClearBit(INIT_BIT_PROXY_ODOMETRY))
    {
        delete odometry;
    }

    // delete odometry mailbox
    if (initBits.testAndClearBit(INIT_BIT_MBX_ODOMETRY))
    {
        destroyMbx(&odometryMbx);
    }

    // delete work mailbox
    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }

    // call DataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        DataModule::moduleCleanup();
    }
}

Position::Position()
      : DataModule( MODULE_CLASS_ID,
                    2000000000llu,    // 2s cmdtask error sleep time
                    2000000000llu,    // 2s datatask error sleep time
                     100000000llu,    // 100ms datatask disable sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    1000,             // max buffer entries
                    10)               // data buffer listener
{
    // get value(s) out of your argument table
    odometryInst   = getIntArg("odometryInst", argTab);

    refPos.x   = 0;
    refPos.y   = 0;
    refPos.z   = 0;
    refPos.phi = 0;
    refPos.psi = 0;
    refPos.rho = 0;
    sinRefPos  = 0.0;
    cosRefPos  = 1.0;

    refOdo.x   = 0;
    refOdo.y   = 0;
    refOdo.z   = 0;
    refOdo.phi = 0;
    refOdo.psi = 0;
    refOdo.rho = 0;
    sinRefOdo  = 0.0;
    cosRefOdo  = 1.0;

    refTime    = 0;

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(position_data));
}

int  main(int argc, char *argv[])
{
      int ret;

      // get args

      ret = Module::getArgs(argc, argv, argTab, "Position");
      if (ret)
      {
        printf("Invalid arguments -> EXIT \n");
        return ret;
      }

      // create new Position

      p_inst = new Position();
      if (!p_inst)
      {
        printf("Can't create new Position -> EXIT\n");
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
