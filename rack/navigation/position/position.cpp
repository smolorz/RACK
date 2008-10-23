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
      "Northing offset for Gauss Krueger/UTM coordinates in in m, default 0", { 0 } },

    { ARGOPT_OPT, "offsetEasting", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Easting offset for Gauss Krueger/UTM coordinates in in m, default 0", { 0 } },

    { ARGOPT_OPT, "utmZone", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Number of UTM zone, default 32", { 32 } },

    { ARGOPT_OPT, "positionReference", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Position reference frame, 0 = wgs84, 1 = Gauss Krueger, 2 = Utm, default 0", { 0 } },

    { ARGOPT_OPT, "autoOffset", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Automatic offset estimation, 0 = off, 1 = on, default 1", { 1 } },

    { ARGOPT_OPT, "odometryStdDevX", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Odometry standard deviation X, mm/m, default 50", { 50 } },
      
    { ARGOPT_OPT, "odometryStdDevY", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Odometry standard deviation Y, mm/m, 1 = on, default 100", { 50 } },
      
    { ARGOPT_OPT, "odometryStdDevRho", ARGOPT_REQVAL, ARGOPT_VAL_FLT,
      "Odometry standard deviation Rho, deg/deg, default 0.1", { 0 } },

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

    // get parameter
    updateInterpol    = getInt32Param("updateInterpol");
    offsetLatitude    = (double)getFloatParam("offsetLatitude") * M_PI / 180.0;
    offsetLongitude   = (double)getFloatParam("offsetLongitude") * M_PI / 180.0;
    scaleLatitude     = getInt32Param("scaleLatitude");
    scaleLongitude    = getInt32Param("scaleLongitude");
    offsetNorthing    = (double)getInt32Param("offsetNorthing");
    offsetEasting     = (double)getInt32Param("offsetEasting");
    utmZone           = getInt32Param("utmZone");
    positionReference = getInt32Param("positionReference");
    autoOffset        = getInt32Param("autoOffset");
    odometryStdDevX   = getInt32Param("odometryStdDevX");
    odometryStdDevY   = getInt32Param("odometryStdDevY");
    odometryStdDevRho = (double)getFloatParam("odometryStdDevRho") * M_PI / 180.0;

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
    memcpy(&oldOdometryPos, &odometryData.pos, sizeof(position_3d));
    sinRefOdo = sin(refOdo.rho);
    cosRefOdo = cos(refOdo.rho);

    memcpy(&refPos, &oldPos, sizeof(position_3d));
    memcpy(&refPosI, &refPos, sizeof(position_3d));
    sinRefPosI = sin(refPosI.rho);
    cosRefPosI = cos(refPosI.rho);

    interpolDiff.x    = 0;
    interpolDiff.y    = 0;
    interpolDiff.z    = 0;
    interpolDiff.rho  = 0.0f;
    interpolStartTime = odometryData.recordingTime;
    
    positionStdDeviation.x = 0;
    positionStdDeviation.x = 0;
    positionStdDeviation.x = 0;
    positionStdDeviation.rho = 0.0f;
    positionStdDeviation.phi = 0.0f;
    positionStdDeviation.psi = 0.0f;

    offset = 0;

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

        //GDOS_DBG_DETAIL("Odometry position x %i y %i z %i rho %a\n",
        //                odometryData.pos.x, odometryData.pos.y, odometryData.pos.z, odometryData.pos.rho);

        refPosMtx.lock(RACK_INFINITE);

        // update interpolated reference position refPosI
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

        // calculate absolute position
        getPosition(&odometryData.pos, &pPosition->pos);
        pPosition->recordingTime = odometryData.recordingTime;

        if (stdDeviationUpdate == 1)
        {
            stdDevX = (double)positionStdDeviation.x;
            stdDevY = (double)positionStdDeviation.y;
            stdDevRho = positionStdDeviation.rho;
            stdDeviationUpdate = 0;
        }

        stdDevX      += ((double)odometryStdDevX * fabs((double)(odometryData.pos.x - oldOdometryPos.x)) / 1000.0 +
                        (double)odometryStdDevY * fabs((double)(odometryData.pos.y - oldOdometryPos.y)) / 5000.0);
        stdDevY      += ((double)odometryStdDevX * fabs((double)(odometryData.pos.x - oldOdometryPos.x)) / 5000.0 +
                        (double)odometryStdDevY * fabs((double)(odometryData.pos.y - oldOdometryPos.y)) / 1000.0);
        stdDevRho    += 0.1 * fabs(odometryData.pos.rho - oldOdometryPos.rho) +
                            fabs((double)(odometryData.pos.x - oldOdometryPos.x)) / 100000.0 +
                            fabs((double)(odometryData.pos.y - oldOdometryPos.y)) / 100000.0;

        pPosition->var.x = (int)stdDevX;
        pPosition->var.y = (int)stdDevY;
        pPosition->var.rho = stdDevRho;

        memcpy(&oldOdometryPos, &odometryData.pos, sizeof(position_3d));
    
        refPosMtx.unlock();

        GDOS_DBG_DETAIL("recordingTime %i x %i y %i z %i phi %a psi %a rho %a varX %i varY %i varRho %a\n",
                        pPosition->recordingTime,
                        pPosition->pos.x, pPosition->pos.y, pPosition->pos.z,
                        pPosition->pos.phi, pPosition->pos.psi, pPosition->pos.rho,pPosition->var.x, pPosition->var.y,
                        pPosition->var.rho);


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
    position_data       *pUpdate;
    position_3d         posOldRefI;
    odometry_data       odometryData;

    position_data       posData;
    position_data       *pPosData;
    position_wgs84_data *pPosWgs84Data;
    position_utm_data   *pPosUtmData;
    position_gk_data    *pPosGkData;
    position_wgs84_data posWgs84Data;
    position_utm_data   posUtmData;
    position_gk_data    posGkData;
    int                 ret;

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

            // store new reference position
            memcpy(&refPos, &pUpdate->pos, sizeof(position_3d));

            // store new standard deviation
            memcpy(&positionStdDeviation, &pUpdate->var, sizeof(position_3d));
            stdDeviationUpdate = 1;

            // store difference between old and new reference position for interpolation
            if(updateInterpol != 0)
            {
                getPosition(&odometryData.pos, &posOldRefI);

                interpolDiff.x = posOldRefI.x - pUpdate->pos.x;
                interpolDiff.y = posOldRefI.y - pUpdate->pos.y;
                interpolDiff.z = posOldRefI.z - pUpdate->pos.z;
                interpolDiff.rho = normaliseAngleSym0(posOldRefI.rho - pUpdate->pos.rho);

                interpolStartTime = rackTime.get();

                memcpy(&refPosI, &posOldRefI, sizeof(position_3d));
                sinRefPosI = sin(refPosI.rho);
                cosRefPosI = cos(refPosI.rho);
            }
            else
            {
                interpolDiff.x = 0;
                interpolDiff.y = 0;
                interpolDiff.z = 0;
                interpolDiff.rho = 0.0f;

                interpolStartTime = rackTime.get();

                memcpy(&refPosI, &refPos, sizeof(position_3d));
                sinRefPosI  = sin(refPosI.rho);
                cosRefPosI  = cos(refPosI.rho);
            }

            // store odometry position of new reference
            memcpy(&refOdo, &odometryData.pos, sizeof(position_3d));
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
            wgs84ToPos(pPosWgs84Data, &posData);

            GDOS_DBG_INFO("WGS84 to POS: WGS84 lat %adeg, lon %adeg, alt %dmm, head %adeg, "
                          "Pos x %dmm, y %dmm, z %dmm, rho %adeg\n",
                          pPosWgs84Data->latitude, pPosWgs84Data->longitude, pPosWgs84Data->altitude,
                          pPosWgs84Data->heading, posData.pos.x, posData.pos.y, 
                          posData.pos.z, posData.pos.rho);
            cmdMbx.sendDataMsgReply(MSG_POSITION_POS, msgInfo, 1, &posData,
                                    sizeof(position_data));
            break;

        case MSG_POSITION_POS_TO_WGS84:
            pPosData = PositionData::parse(msgInfo);
            posToWgs84(pPosData, &posWgs84Data);

            GDOS_DBG_INFO("POS to WGS84: Pos x %dmm, y %dmm, z %dmm, rho %adeg, "
                          "WGS84 lat %adeg, lon %adeg, alt %dmm, head %adeg\n",
                          posData.pos.x, posData.pos.y, posData.pos.z, posData.pos.rho,
                          posWgs84Data.latitude, posWgs84Data.longitude,
                          posWgs84Data.altitude, posWgs84Data.heading);
            cmdMbx.sendDataMsgReply(MSG_POSITION_WGS84, msgInfo, 1, &posWgs84Data,
                                    sizeof(position_wgs84_data));
            break;

        case MSG_POSITION_UTM_TO_POS:
            pPosUtmData = PositionUtmData::parse(msgInfo);

            if (positionReference == 2)
            {
                posData.pos.x   =  (int)rint(pPosUtmData->northing - offsetNorthing * 1000.0);
                posData.pos.y   =  (int)rint(pPosUtmData->easting - offsetEasting * 1000.0);
                posData.pos.z   = -pPosUtmData->altitude;
                posData.pos.phi = 0.0f;
                posData.pos.psi = 0.0f;
                posData.pos.rho = pPosUtmData->heading;
            }
            else
            {
                positionTool->utmToWgs84(pPosUtmData, &posWgs84Data);
                wgs84ToPos(&posWgs84Data, &posData);
            }

            GDOS_DBG_INFO("UTM to POS: UTM zone %d, north %fmm, east %fmm, alt %dmm, head %adeg, "
                          "Pos x %dmm, y %dmm, z %dmm, rho %adeg\n",
                          pPosUtmData->zone, pPosUtmData->northing, pPosUtmData->easting,
                          pPosUtmData->altitude, pPosUtmData->heading, posData.pos.x,
                          posData.pos.y, posData.pos.z, posData.pos.rho);
            cmdMbx.sendDataMsgReply(MSG_POSITION_POS, msgInfo, 1, &posData,
                                    sizeof(position_data));
            break;

        case MSG_POSITION_POS_TO_UTM:
            pPosData = PositionData::parse(msgInfo);

            if (positionReference == 2)
            {
                posUtmData.zone     =  utmZone;
                posUtmData.northing =  (double)pPosData->pos.x + offsetNorthing * 1000.0;
                posUtmData.easting  =  (double)pPosData->pos.y + offsetEasting * 1000.0;
                posUtmData.altitude = -pPosData->pos.z;
                posUtmData.heading  =  pPosData->pos.rho;
            }
            else
            {
                posToWgs84(pPosData, &posWgs84Data);
                positionTool->wgs84ToUtm(&posWgs84Data, &posUtmData);
            }

            GDOS_DBG_INFO("POS to UTM: Pos x %dmm, y %dmm, z %dmm, rho %adeg, "
                          "UTM zone %d, north %fmm, east %fmm, alt %dmm, head %adeg\n",
                          posData.pos.x, posData.pos.y, posData.pos.z, posData.pos.rho,
                          posUtmData.zone, posUtmData.northing, posUtmData.easting,
                          posUtmData.altitude, posUtmData.heading);
            cmdMbx.sendDataMsgReply(MSG_POSITION_UTM, msgInfo, 1, &posUtmData,
                                    sizeof(position_utm_data));
            break;

        case MSG_POSITION_GK_TO_POS:
            pPosGkData = PositionGkData::parse(msgInfo);

            if (positionReference == 1)
            {
                posData.pos.x   =  (int)rint(pPosGkData->northing - offsetNorthing * 1000.0);
                posData.pos.y   =  (int)rint(pPosGkData->easting - offsetEasting * 1000.0);
                posData.pos.z   = -pPosGkData->altitude;
                posData.pos.phi = 0.0f;
                posData.pos.psi = 0.0f;
                posData.pos.rho = pPosGkData->heading;
            }
            else
            {
                positionTool->gkToWgs84(pPosGkData, &posWgs84Data);
                wgs84ToPos(&posWgs84Data, &posData);
            }

            GDOS_DBG_INFO("GK to POS: GK north %fmm, east %fmm, alt %dmm, head %adeg, "
                          "Pos x %dmm, y %dmm, z %dmm, rho %adeg\n",
                            pPosGkData->northing, pPosGkData->easting, pPosGkData->altitude,
                            pPosGkData->heading, posData.pos.x, posData.pos.y, posData.pos.z,
                            posData.pos.rho);
            cmdMbx.sendDataMsgReply(MSG_POSITION_POS, msgInfo, 1, &posData,
                                    sizeof(position_data));
            break;

        case MSG_POSITION_POS_TO_GK:
            pPosData = PositionData::parse(msgInfo);

            if (positionReference == 1)
            {
                posGkData.northing  =  (double)pPosData->pos.x + offsetNorthing * 1000.0;
                posGkData.easting   =  (double)pPosData->pos.y + offsetEasting * 1000.0;
                posGkData.altitude  = -pPosData->pos.z;
                posGkData.heading   =  pPosData->pos.rho;
            }
            else
            {
                posToWgs84(pPosData, &posWgs84Data);
                positionTool->wgs84ToGk(&posWgs84Data, &posGkData);
            }

            GDOS_DBG_INFO("POS to GK: Pos x %dmm, y %dmm, z %dmm, rho %adeg, "
                          "GK north %fmm, east %fmm, alt %dmm, head %adeg\n",
                          pPosData->pos.x, pPosData->pos.y, pPosData->pos.z, 
                          pPosData->pos.rho, posGkData.northing, posGkData.easting,
                          posGkData.altitude, posGkData.heading);
            cmdMbx.sendDataMsgReply(MSG_POSITION_GK, msgInfo, 1, &posGkData,
                                    sizeof(position_gk_data));
            break;


        default:
            // not for me -> ask RackDataModule
            return RackDataModule::moduleCommand(msgInfo);
      }
      return 0;
}

void    Position::wgs84ToPos(position_wgs84_data *posWgs84Data, position_data *posData)
{
    position_gk_data    posGk;
    position_utm_data   posUtm;
    double              diffLat, diffLon;

    switch (positionReference)
    {
        // position reference is Gauss-Krueger
        case POSITION_REFERENCE_GK:
            positionTool->wgs84ToGk(posWgs84Data, &posGk);

            // auto offset calculation
            if ((autoOffset == 1) && (offset != 1))
            {
               offset         = 1;
               offsetNorthing = rint(posGk.northing / (100.0 * 1000.0)) * 100.0;
               offsetEasting  = rint(posGk.easting / (100.0 * 1000.0)) * 100.0;
               GDOS_PRINT("Set new GK position offset to north %fm, east %fm\n", 
                          offsetNorthing, offsetEasting);
            }

            posData->pos.x   =  (int)rint(posGk.northing - offsetNorthing * 1000.0);
            posData->pos.y   =  (int)rint(posGk.easting - offsetEasting * 1000.0);
            posData->pos.z   = -posGk.altitude;
            posData->pos.phi = 0.0f;
            posData->pos.psi = 0.0f;
            posData->pos.rho = posGk.heading;

            GDOS_DBG_INFO("Wgs84 lat %a, lon %a, alt %d, head %a"
                        "Gauss-Krueger north %f, east %f, alt %d, head %a\n",
                        posWgs84Data->latitude, posWgs84Data->longitude, 
                        posWgs84Data->altitude, posWgs84Data->heading,
                        posGk.northing, posGk.easting, posGk.altitude, posGk.heading);
            break;

        // position reference is Utm
        case POSITION_REFERENCE_UTM:
            positionTool->wgs84ToUtm(posWgs84Data, &posUtm);

            // auto offset calculation
            if ((autoOffset == 1) && (offset != 1))
            {
               offset         = 1;
               offsetNorthing = rint(posUtm.northing / (100.0 * 1000.0)) * 100.0;
               offsetEasting  = rint(posUtm.easting / (100.0 * 1000.0)) * 100.0;
               GDOS_PRINT("Set new UTM position offset to north %fm, east %fm\n", 
                          offsetNorthing, offsetEasting);
            }

            posData->pos.x   =  (int)rint(posUtm.northing - offsetNorthing * 1000.0);
            posData->pos.y   =  (int)rint(posUtm.easting - offsetEasting * 1000.0);
            posData->pos.z   = -posUtm.altitude;
            posData->pos.phi = 0.0f;
            posData->pos.psi = 0.0f;
            posData->pos.rho = posUtm.heading;

            GDOS_DBG_INFO("Wgs84 lat %a, lon %a, alt %d, head %a"
                          "UTM zone %d, north %f, east %f, alt %d, head %a\n",
                        posWgs84Data->latitude, posWgs84Data->longitude, 
                        posWgs84Data->altitude, posWgs84Data->heading,
                        posUtm.zone, posUtm.northing, posUtm.easting, 
                        posUtm.altitude, posUtm.heading);
            break;

            // position reference is Wgs84
            default:
                diffLat          = posWgs84Data->latitude - offsetLatitude;
                diffLon          = posWgs84Data->longitude - offsetLongitude;
                posData->pos.x   = (int)rint((diffLat * 180.0 / M_PI) * scaleLatitude * 1000.0);
                posData->pos.y   = (int)rint((diffLon * 180.0 / M_PI) * scaleLongitude * 1000.0);
                posData->pos.z   = -posWgs84Data->altitude;
                posData->pos.phi = 0.0f;
                posData->pos.psi = 0.0f;
                posData->pos.rho = posWgs84Data->heading;
                break;
        }
}


void    Position::posToWgs84(position_data *posData, position_wgs84_data *posWgs84Data)
{
    position_gk_data    posGk;
    position_utm_data   posUtm;
    double              diffLat, diffLon;

    switch (positionReference)
    {
        // position reference is Gauss-Krueger
        case POSITION_REFERENCE_GK:
            posGk.northing  =  (double)posData->pos.x + offsetNorthing * 1000.0;
            posGk.easting   =  (double)posData->pos.y + offsetEasting * 1000.0;
            posGk.altitude  = -posData->pos.z;
            posGk.heading   =  posData->pos.rho;
            positionTool->gkToWgs84(&posGk, posWgs84Data);

            GDOS_DBG_INFO("Gauss-Krueger north %f, east %f, alt %d, head %a, "
                          "Wgs84 lat %a, lon %a, alt %d, head %a\n",
                          posGk.northing, posGk.easting, posGk.altitude, posGk.heading,
                          posWgs84Data->latitude, posWgs84Data->longitude, 
                          posWgs84Data->altitude, posWgs84Data->heading);
            break;

            // position reference is Utm
            case POSITION_REFERENCE_UTM:
                posUtm.zone     =  utmZone;
                posUtm.northing =  (double)posData->pos.x + offsetNorthing * 1000.0;
                posUtm.easting  =  (double)posData->pos.y + offsetEasting * 1000.0;
                posUtm.altitude = -posData->pos.z;
                posUtm.heading  =  posData->pos.rho;
                positionTool->utmToWgs84(&posUtm, posWgs84Data);

                GDOS_DBG_INFO("Utm zone %d, north %f, east %f, alt %d, head %a, "
                              "Wgs84 lat %a, lon %a, alt %d, head %a\n",
                              posUtm. zone, posUtm.northing, posUtm.easting, 
                              posUtm.altitude, posUtm.heading,
                              posWgs84Data->latitude, posWgs84Data->longitude, 
                              posWgs84Data->altitude, posWgs84Data->heading);
                break;

            // position reference is Wgs84
            default:
                if (scaleLatitude != 0)
                {
                    diffLat = (((double)posData->pos.x / scaleLatitude) / 1000.0) * M_PI / 180.0;
                }
                else
                {
                    diffLat = 0.0;
                }

                if (scaleLongitude != 0)
                {
                    diffLon = (((double)posData->pos.y / scaleLongitude) / 1000.0) * M_PI / 180.0;
                }
                else
                {
                    diffLon = 0;
                }

                posWgs84Data->latitude  = offsetLatitude + diffLat;
                posWgs84Data->longitude = offsetLongitude + diffLon;
                posWgs84Data->altitude  = -posData->pos.z;
                posWgs84Data->heading   = posData->pos.rho;
                break;
    }
}


void    Position::getPosition(position_3d* odo, position_3d* pos)
{
    position_3d odoDiff, relPos;

    // calculate relative position to reference position
    odoDiff.x   = odo->x - refOdo.x;
    odoDiff.y   = odo->y - refOdo.y;
    odoDiff.z   = odo->z - refOdo.z;
    odoDiff.rho = odo->rho - refOdo.rho;

    relPos.x    = (int)(+ cosRefOdo * odoDiff.x + sinRefOdo * odoDiff.y);
    relPos.y    = (int)(- sinRefOdo * odoDiff.x + cosRefOdo * odoDiff.y);
    relPos.z    = odoDiff.z;
    relPos.rho  = normaliseAngleSym0(odoDiff.rho);

    //GDOS_DBG_DETAIL("Relative position x %i y %i z %i rho %a\n",
    //                relPos.x, relPos.y, relPos.z, relPos.rho);

    // calculate absolute position
    pos->x      = refPosI.x + (int)(cosRefPosI * relPos.x - sinRefPosI * relPos.y);
    pos->y      = refPosI.y + (int)(sinRefPosI * relPos.x + cosRefPosI * relPos.y);
    pos->z      = refPosI.z + relPos.z;
    pos->phi    = normaliseAngleSym0(odo->phi);
    pos->psi    = normaliseAngleSym0(odo->psi);
    pos->rho    = normaliseAngle(refPosI.rho + relPos.rho);
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

    // get static parameter
    odometryInst      = getInt32Param("odometryInst");

    oldPos.x    = 0;
    oldPos.y    = 0;
    oldPos.z    = 0;
    oldPos.phi  = 0.0f;
    oldPos.psi  = 0.0f;
    oldPos.rho  = 0.0f;

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
                    240,              // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    1000,             // max buffer entries
                    10)               // data buffer listener
{
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
