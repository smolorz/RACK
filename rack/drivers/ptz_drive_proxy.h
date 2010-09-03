/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Matthias Hentschel  <hentschel@rts.uni-hannover.de>
 *
 */
#ifndef __PTZ_DRIVE_PROXY_H__
#define __PTZ_DRIVE_PROXY_H__

#include <main/rack_proxy.h>

//######################################################################
//# PtzDrive Message Types
//######################################################################
#define MSG_PTZ_DRIVE_HOME                        (RACK_PROXY_MSG_POS_OFFSET + 1)
#define MSG_PTZ_DRIVE_MOVE_POS                    (RACK_PROXY_MSG_POS_OFFSET + 2)
#define MSG_PTZ_DRIVE_MOVE_VEL                    (RACK_PROXY_MSG_POS_OFFSET + 3)
#define MSG_PTZ_DRIVE_INFORM_REACHED              (RACK_PROXY_MSG_POS_OFFSET + 4)

#define MSG_PTZ_DRIVE_POSITION_REACHED            (RACK_PROXY_MSG_NEG_OFFSET - 1)

//######################################################################
//# PtzDrive Data (static size - MESSAGE)
//######################################################################

/**
 * pan-tilt-zoom drive data structure
 */
typedef struct {
    rack_time_t recordingTime;              /**< [ms] global timestamp (has to be first element)*/
    float       posPan;                     /**< [rad] pan position */
    float       posTilt;                    /**< [rad] tilt position */
    float       posZoom;                    /**< [pct] zoom position */
} __attribute__((packed)) ptz_drive_data;

class PtzDriveData
{
    public:
        static void le_to_cpu(ptz_drive_data *data)
        {
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->posPan        = __le32_float_to_cpu(data->posPan);
            data->posTilt       = __le32_float_to_cpu(data->posTilt);
            data->posZoom       = __le32_float_to_cpu(data->posZoom);
        }

        static void be_to_cpu(ptz_drive_data *data)
        {
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->posPan        = __be32_float_to_cpu(data->posPan);
            data->posTilt       = __be32_float_to_cpu(data->posTilt);
            data->posZoom       = __be32_float_to_cpu(data->posZoom);;
        }

        static ptz_drive_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            ptz_drive_data *p_data = (ptz_drive_data *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }
};


//######################################################################
//# PtzDrive Move Pos Data (static size - MESSAGE)
//######################################################################

/**
 * pan-tilt-zoom drive move velocity data structure
 */
typedef struct {
    float       posPan;                     /**< [rad]   pan set position for moving */
    float       posTilt;                    /**< [rad]   tilt set position for moving */
    float       posZoom;                    /**< [pct]   zoom set position for moving */
    float       velPan;                     /**< [rad/s] maximum velocity on moving to
                                                         pan set position */
    float       velTilt;                    /**< [rad/s] maximum velocity on moving to
                                                         tilt set position */
    float       velZoom;                    /**< [pct/s] maximum velcity on moving to
                                                         zoom set position */
    float       accPan;                     /**< [rad/s^2] maximum acceleration on moving to
                                                           pan set position */
    float       accTilt;                    /**< [rad/s^2] maximum acceleration on moving to
                                                           tilt set position */
    float       accZoom;                    /**< [pct/s^2] maximum acceleration on moving to
                                                           zoom set position */
    int32_t     replyPosReached;
} __attribute__((packed)) ptz_drive_move_pos_data;

class PtzDriveMovePosData
{
    public:
        static void le_to_cpu(ptz_drive_move_pos_data *data)
        {
            data->posPan            = __le32_float_to_cpu(data->posPan);
            data->posTilt           = __le32_float_to_cpu(data->posTilt);
            data->posZoom           = __le32_float_to_cpu(data->posZoom);
            data->velPan            = __le32_float_to_cpu(data->velPan);
            data->velTilt           = __le32_float_to_cpu(data->velTilt);
            data->velZoom           = __le32_float_to_cpu(data->velZoom);
            data->accPan            = __le32_float_to_cpu(data->accPan);
            data->accTilt           = __le32_float_to_cpu(data->accTilt);
            data->accZoom           = __le32_float_to_cpu(data->accZoom);
            data->replyPosReached   = __le32_to_cpu(data->replyPosReached);
        }

        static void be_to_cpu(ptz_drive_move_pos_data *data)
        {
            data->posPan            = __be32_float_to_cpu(data->posPan);
            data->posTilt           = __be32_float_to_cpu(data->posTilt);
            data->posZoom           = __be32_float_to_cpu(data->posZoom);
            data->velPan            = __be32_float_to_cpu(data->velPan);
            data->velTilt           = __be32_float_to_cpu(data->velTilt);
            data->velZoom           = __be32_float_to_cpu(data->velZoom);
            data->accPan            = __be32_float_to_cpu(data->accPan);
            data->accTilt           = __be32_float_to_cpu(data->accTilt);
            data->accZoom           = __be32_float_to_cpu(data->accZoom);
            data->replyPosReached   = __be32_to_cpu(data->replyPosReached);
        }

        static ptz_drive_move_pos_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            ptz_drive_move_pos_data *p_data = (ptz_drive_move_pos_data *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }
};

//######################################################################
//# PtzDrive Move Vel Data (static size - MESSAGE)
//######################################################################

/**
 * pan-tilt-zoom drive move velocity data structure
 */
typedef struct {
    float       velPan;                     /**< [rad/s] set velocity for pan moving */
    float       velTilt;                    /**< [rad/s] set velocity for tilt moving */
    float       velZoom;                    /**< [pct/s] set velocity for zoom moving */
} __attribute__((packed)) ptz_drive_move_vel_data;

class PtzDriveMoveVelData
{
    public:
        static void le_to_cpu(ptz_drive_move_vel_data *data)
        {
            data->velPan    = __le32_float_to_cpu(data->velPan);
            data->velTilt   = __le32_float_to_cpu(data->velTilt);
            data->velZoom   = __le32_float_to_cpu(data->velZoom);
       }

        static void be_to_cpu(ptz_drive_move_vel_data *data)
        {
            data->velPan    = __be32_float_to_cpu(data->velPan);
            data->velTilt   = __be32_float_to_cpu(data->velTilt);
            data->velZoom   = __be32_float_to_cpu(data->velZoom);
        }

        static ptz_drive_move_vel_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            ptz_drive_move_vel_data *p_data =
                                   (ptz_drive_move_vel_data *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }
};

/**
 * Hardware abstraction for pan-tilt-zoom (ptz) drives.
 *
 * @ingroup proxies_drivers
 */
class PtzDriveProxy : public RackDataProxy {

    public:
        PtzDriveProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
                    : RackDataProxy(workMbx, sys_id, PTZ_DRIVE, instance)
        {
        };

        ~PtzDriveProxy()
        {
        };

//
// get data
//

        int getData(ptz_drive_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp)
        {
            return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
        }

        int getData(ptz_drive_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp, uint64_t reply_timeout_ns);

//
// move pos
//

        int movePos(ptz_drive_move_pos_data *recv_data, ssize_t recv_datalen)
        {
            return movePos(recv_data, recv_datalen, dataTimeout);
        }

        int movePos(ptz_drive_move_pos_data *recv_data, ssize_t recv_datalen,
                    uint64_t reply_timeout_ns);


//
// move vel
//
        int moveVel(ptz_drive_move_vel_data *recv_data, ssize_t recv_datalen)
        {
            return moveVel(recv_data, recv_datalen, dataTimeout);
        }

        int moveVel(ptz_drive_move_vel_data *recv_data, ssize_t recv_datalen,
                    uint64_t reply_timeout_ns);
};

/*@}*/

#endif
