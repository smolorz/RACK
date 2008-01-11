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
#define INIT_BIT_PT_CREATED             5

//
// data structures
//

Position *p_inst;

argTable_t argTab[] = {

    { ARGOPT_OPT, "odometryInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the odometry module", { 0 } },

    { ARGOPT_OPT, "updateInterpol", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Time constant for update interpolation (default 0)", { 0 } },

    { ARGOPT_OPT, "offsetLatitude", ARGOPT_REQVAL, ARGOPT_VAL_FLT,
      "Latitude Offset for the global cartesian coordinate system in deg, default 0", { 0 } },

    { ARGOPT_OPT, "offsetLongitude", ARGOPT_REQVAL, ARGOPT_VAL_FLT,
      "Longitude offset for the global cartesian coordinate system in deg, default 0", { 0 } },

    { ARGOPT_OPT, "scaleLatitude", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Latitude scale in m / deg, default 0", { 0 } },

    { ARGOPT_OPT, "scaleLongitude", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Longitude scale in m / deg, default 0", { 0 } },

    { ARGOPT_OPT, "offsetNorthing", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Northing offset for Gauss Krueger coordinates in in m, default 0", { 0 } },

    { ARGOPT_OPT, "offsetEasting", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Easting offset for Gauss Krueger coordinates in in m, default 0", { 0 } },

    { ARGOPT_OPT, "positionReference", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Position reference frame, 0 = wgs84, 1 = Gauss Krueger, default 0", { 0 } },

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
    int           ret;
    odometry_data odometryData;

    ret = odometry->on();
    if (ret)
    {
        GDOS_ERROR("Can't switch on odometry(%i), code = %d\n", odometryInst, ret);
        return ret;
    }

    //get actual odometry data
    ret = odometry->getData(&odometryData, sizeof(odometry_data), 0);
    if (ret)
    {
        GDOS_ERROR("Can't get data from Odometry(%i), code = %d\n", odometryInst, ret);
        return ret;
    }

    ret = odometry->getContData(0, &odometryMbx, &dataBufferPeriodTime);
    if (ret)
    {
        GDOS_ERROR("Can't get continuous data from odometry(%i) module, "
                   "code = %d\n", odometryInst, ret);
        return ret;
    }

    memcpy(&refOdo, &odometryData.pos, sizeof(position_3d));
    memcpy(&refPos, &oldPos, sizeof(position_3d));
    sinRefOdo = sin(refOdo.rho);
    cosRefOdo = cos(refOdo.rho);

    interpolDiff.x    = 0;
    interpolDiff.y    = 0;
    interpolDiff.z    = 0;
    interpolDiff.rho  = 0.0f;
    interpolStartTime = rackTime.get();

    return RackDataModule::moduleOn();    // has to be last command in moduleOn();
}

void Position::moduleOff(void)
{
    RackDataModule::moduleOff();          // has to be first command in moduleOff();

    odometry->stopContData(&odometryMbx);
}


int  Position::moduleLoop(void)
{
    int             ret;
    position_data*  pPosition;
    odometry_data   odometryData;
    message_info    msgInfo;
    position_3d     relPos;
    position_3d     refPosI;
    double          sinRefPosI, cosRefPosI;
    double          interpolFactor;

    pPosition = (position_data *)getDataBufferWorkSpace();

    ret = odometryMbx.recvDataMsgTimed(dataBufferPeriodTime * 3000000llu, &odometryData,
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

        odometryData.pos.phi = normaliseAngleSym0(odometryData.pos.phi);
        odometryData.pos.psi = normaliseAngleSym0(odometryData.pos.psi);
        odometryData.pos.rho = normaliseAngle(odometryData.pos.rho);

        GDOS_DBG_DETAIL("Odometry position x %i y %i z %i rho %a\n",
                        odometryData.pos.x, odometryData.pos.y, odometryData.pos.z, odometryData.pos.rho);

        refPosMtx.lock(RACK_INFINITE);

        // calculate relative position to refference position
        odometryData.pos.x = odometryData.pos.x - refOdo.x;
        odometryData.pos.y = odometryData.pos.y - refOdo.y;
        relPos.x   = (int)(+ cosRefOdo * odometryData.pos.x + sinRefOdo * odometryData.pos.y);
        relPos.y   = (int)(- sinRefOdo * odometryData.pos.x + cosRefOdo * odometryData.pos.y);
        relPos.z   = odometryData.pos.z   - refOdo.z;
        relPos.rho = normaliseAngleSym0(odometryData.pos.rho - refOdo.rho);

        GDOS_DBG_DETAIL("Relative position x %i y %i z %i rho %a\n",
                        relPos.x, relPos.y, relPos.z, relPos.rho);

        // calculate interpolated reference position refPosI
        if(updateInterpol != 0)
        {
            interpolFactor = 1.0 - (((double)odometryData.recordingTime - (double)interpolStartTime) / (double)updateInterpol);

            if(interpolFactor > 1.0)
            {
                interpolFactor = 1.0;
            }
            else if(interpolFactor < 0.0)
            {
                interpolFactor = 0.0;
            }

            refPosI.x   = refPos.x + (int)(interpolFactor * interpolDiff.x);
            refPosI.y   = refPos.y + (int)(interpolFactor * interpolDiff.y);
            refPosI.z   = refPos.z + (int)(interpolFactor * interpolDiff.z);
            refPosI.rho = refPos.rho + (interpolFactor * interpolDiff.rho);

            sinRefPosI  = sin(refPosI.rho);
            cosRefPosI  = cos(refPosI.rho);
        }
        else
        {
            refPosI.x   = refPos.x;
            refPosI.y   = refPos.y;
            refPosI.z   = refPos.z;
            refPosI.rho = refPos.rho;

            sinRefPosI  = sin(refPosI.rho);
            cosRefPosI  = cos(refPosI.rho);
        }

        // calculate absolute position
        pPosition->pos.x         = refPosI.x + (int)(cosRefPosI * relPos.x - sinRefPosI * relPos.y);
        pPosition->pos.y         = refPosI.y + (int)(sinRefPosI * relPos.x + cosRefPosI * relPos.y);
        pPosition->pos.z         = refPosI.z + relPos.z;
        pPosition->pos.phi       = odometryData.pos.phi;
        pPosition->pos.psi       = odometryData.pos.psi;
        pPosition->pos.rho       = normaliseAngle(refPosI.rho + relPos.rho);

        pPosition->recordingTime = odometryData.recordingTime;

        refPosMtx.unlock();

        GDOS_DBG_DETAIL("recordingTime %i x %i y %i z %i phi %a psi %a rho %a\n",
                        pPosition->recordingTime,
                        pPosition->pos.x, pPosition->pos.y, pPosition->pos.z,
                        pPosition->pos.phi, pPosition->pos.psi, pPosition->pos.rho);

        putDataBufferWorkSpace(sizeof(position_data));

        memcpy(&oldPos, &pPosition->pos, sizeof(position_3d));
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
    position_data       *pUpdate, *pPosOldRef;
    position_data       posData;
    position_data       *pPosData;
    position_wgs84_data posWgs84Data;
    position_wgs84_data *pPosWgs84Data;
    position_gk_data    posGk;
    odometry_data       odometryData;
    int                 ret, posOldRefIndex;
    double              diffLat, diffLon;

    switch(msgInfo->type)
    {
        case MSG_POSITION_UPDATE:
            pUpdate = PositionData::parse(msgInfo);

            if(pUpdate->recordingTime == 0)
            {
                pUpdate->recordingTime = rackTime.get();
            }

            ret = odometry->getData(&odometryData,
                                    sizeof(odometry_data),
                                    pUpdate->recordingTime);
            if (ret)
            {
                cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
                break;
            }

            refPosMtx.lock(RACK_INFINITE);

            // store difference between old and new reference position for interpolation
            if(updateInterpol != 0)
            {
                posOldRefIndex = getDataBufferIndex(pUpdate->recordingTime);

                if(posOldRefIndex >= 0)
                {
                    pPosOldRef = ((position_data*)dataBuffer[posOldRefIndex].pData);

                    interpolDiff.x = pPosOldRef->pos.x - pUpdate->pos.x;
                    interpolDiff.y = pPosOldRef->pos.y - pUpdate->pos.y;
                    interpolDiff.z = pPosOldRef->pos.z - pUpdate->pos.z;
                    interpolDiff.rho = normaliseAngleSym0(pPosOldRef->pos.rho - pUpdate->pos.rho);
                }
                else
                {
                    interpolDiff.x = 0;
                    interpolDiff.y = 0;
                    interpolDiff.z = 0;
                    interpolDiff.rho = 0.0f;
                }
                interpolStartTime = rackTime.get();
            }

            // store odometry position of new reference
            memcpy(&refOdo, &odometryData.pos, sizeof(position_3d));
            // store new reference position
            memcpy(&refPos, &pUpdate->pos, sizeof(position_3d));
            sinRefOdo = sin(refOdo.rho);
            cosRefOdo = cos(refOdo.rho);

            refPosMtx.unlock();

            GDOS_DBG_INFO("update recordingTime %i x %i y %i z %i phi %a psi %a rho %a\n",
                           (int)pUpdate->recordingTime, refPos.x, refPos.y, refPos.z,
                           refPos.phi, refPos.psi, refPos.rho);

            cmdMbx.sendMsgReply(MSG_OK, msgInfo);
            break;

        case MSG_POSITION_WGS84_TO_POS:
            pPosWgs84Data = PositionWgs84Data::parse(msgInfo);

            // position reference is Gauss-Krueger
            if (positionReference == POSITION_REFERENCE_GK)
            {
                positionTool->wgs84ToGk(pPosWgs84Data, &posGk);

                posData.pos.x   =  (int)rint(posGk.northing - offsetNorthing * 1000.0);
                posData.pos.y   =  (int)rint(posGk.easting - offsetEasting * 1000.0);
                posData.pos.z   = -(int)rint(posGk.altitude);
                posData.pos.phi = 0.0f;
                posData.pos.psi = 0.0f;
                posData.pos.rho = pPosWgs84Data->heading;

                GDOS_DBG_INFO("Wgs84 lat %a, lon %a, alt %d, Gauss-Krueger north %f, east %f, alt %f\n",
                               pPosWgs84Data->latitude, pPosWgs84Data->longitude, pPosWgs84Data->altitude,
                               posGk.northing, posGk.easting, posGk.altitude);

                cmdMbx.sendDataMsgReply(MSG_POSITION_POS, msgInfo, 1, &posData,
                                        sizeof(position_data));
            }

            // position reference is Wgs84
            else
            {
                diffLat  = pPosWgs84Data->latitude - offsetLatitude;
                diffLon  = pPosWgs84Data->longitude - offsetLongitude;

                posData.pos.x       = (int)rint((diffLat * 180.0 / M_PI) * scaleLatitude * 1000.0);
                posData.pos.y       = (int)rint((diffLon * 180.0 / M_PI) * scaleLongitude * 1000.0);
                posData.pos.z       = -pPosWgs84Data->altitude;
                posData.pos.phi     = 0.0f;
                posData.pos.psi     = 0.0f;
                posData.pos.rho     = pPosWgs84Data->heading;

                cmdMbx.sendDataMsgReply(MSG_POSITION_POS, msgInfo, 1, &posData,
                                        sizeof(position_data));
            }
            break;

        case MSG_POSITION_POS_TO_WGS84:
            pPosData = PositionData::parse(msgInfo);

            // position reference is Gauss-Krueger
            if (positionReference == POSITION_REFERENCE_GK)
            {
                posGk.northing         =  (double)pPosData->pos.x + offsetNorthing * 1000.0;
                posGk.easting          =  (double)pPosData->pos.y + offsetEasting * 1000.0;
                posGk.altitude         = -(double)pPosData->pos.z;

                positionTool->gkToWgs84(&posGk, &posWgs84Data);

                posWgs84Data.heading   = pPosData->pos.rho;

                GDOS_DBG_INFO("Gauss-Krueger north %d, east %d, alt %d, Wgs84 lat %a, lon %a, alt %d\n",
                               posGk.northing, posGk.easting, posGk.altitude, posWgs84Data.latitude,
                               posWgs84Data.longitude, posWgs84Data.altitude);

                cmdMbx.sendDataMsgReply(MSG_POSITION_WGS84, msgInfo, 1, &posWgs84Data,
                                        sizeof(position_wgs84_data));
            }

            // position reference is Wgs84
            else
            {
                if (scaleLatitude != 0)
                {
                    diffLat = (((double)pPosData->pos.x / scaleLatitude) / 1000.0) * M_PI / 180.0;
                }
                else
                {
                    diffLat = 0.0;
                }

                if (scaleLongitude != 0)
                {
                    diffLon = (((double)pPosData->pos.y / scaleLongitude) / 1000.0) * M_PI / 180.0;
                }
                else
                {
                    diffLon = 0;
                }

                posWgs84Data.latitude  = offsetLatitude + diffLat;
                posWgs84Data.longitude = offsetLongitude + diffLon;
                posWgs84Data.altitude  = -pPosData->pos.z;
                posWgs84Data.heading   = pPosData->pos.rho;

                cmdMbx.sendDataMsgReply(MSG_POSITION_WGS84, msgInfo, 1, &posWgs84Data,
                                        sizeof(position_wgs84_data));
            }
            break;

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

int  Position::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
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

    // init positionTool
    positionTool = new PositionTool(getCmdMbx(), gdosLevel);
    if (!positionTool)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PT_CREATED);

    return 0;

init_error:
    moduleCleanup();
    return ret;
}

void Position::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // destroy positionTool
    if (initBits.testAndClearBit(INIT_BIT_PT_CREATED))
    {
        delete positionTool;
    }

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
}

Position::Position()
      : RackDataModule( MODULE_CLASS_ID,
                    2000000000llu,    // 2s datatask error sleep time
                    8,                // command mailbox slots
                    64,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    1000,             // max buffer entries
                    10)               // data buffer listener
{
    // get value(s) out of your argument table
    odometryInst      = getIntArg("odometryInst", argTab);
    updateInterpol    = getIntArg("updateInterpol", argTab);
    offsetLatitude    = (double)getFltArg("offsetLatitude", argTab) * M_PI / 180.0;
    offsetLongitude   = (double)getFltArg("offsetLongitude", argTab) * M_PI / 180.0;
    scaleLatitude     = getIntArg("scaleLatitude", argTab);
    scaleLongitude    = getIntArg("scaleLongitude", argTab);
    offsetNorthing    = (double)getIntArg("offsetNorthing", argTab);
    offsetEasting     = (double)getIntArg("offsetEasting", argTab);
    positionReference = getIntArg("positionReference", argTab);

    oldPos.x   = 0;
    oldPos.y   = 0;
    oldPos.z   = 0;
    oldPos.rho = 0.0f;

    // set dataBuffer size
    dataBufferMaxDataSize   = sizeof(position_data);
}

int  main(int argc, char *argv[])
{
      int ret;

      // get args

      ret = RackModule::getArgs(argc, argv, argTab, "Position");
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
