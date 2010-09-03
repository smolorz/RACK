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
 *      Joerg Langenberg  <joerg.langenberg@gmx.net>
 *
 */
#ifndef __SERVO_DRIVE_PROXY_H__
#define __SERVO_DRIVE_PROXY_H__

#include <main/rack_proxy.h>

#define SERVO_DRIVE_MAX 8

//######################################################################
//# ServoDrive Message Types
//######################################################################

#define MSG_SERVO_DRIVE_HOME                (RACK_PROXY_MSG_POS_OFFSET + 1)
#define MSG_SERVO_DRIVE_MOVE_POS            (RACK_PROXY_MSG_POS_OFFSET + 2)
#define MSG_SERVO_DRIVE_MOVE_VEL            (RACK_PROXY_MSG_POS_OFFSET + 3)
#define MSG_SERVO_DRIVE_INFORM_REACHED      (RACK_PROXY_MSG_POS_OFFSET + 4)

#define MSG_SERVO_DRIVE_POSITION_REACHED    (RACK_PROXY_MSG_NEG_OFFSET - 1)

//######################################################################
//# ServoDrive Data (static size - MESSAGE)
//######################################################################

/**
 * servo drive data structure
 */
typedef struct {
    rack_time_t recordingTime;              /**< [ms] global timestamp (has to be first element)*/
    float       position;                   /**< [rad|mm] position of the servo drive */
} __attribute__((packed)) servo_drive_data;

class ServoDriveData
{
    public:
        static void le_to_cpu(servo_drive_data *data)
        {
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->position      = __le32_float_to_cpu(data->position);
        }

        static void be_to_cpu(servo_drive_data *data)
        {
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->position      = __le32_float_to_cpu(data->position);
        }

        static servo_drive_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            servo_drive_data *p_data = (servo_drive_data *)msgInfo->p_data;

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
//# ServoDrive Move Pos Data (static size - MESSAGE)
//######################################################################

/**
 * servo drive move velocity data structure
 */
typedef struct {
    float       position;                   /**< [rad|mm]   set position for moving */
    float       vel;                        /**< [rad/s|mm/s] maximum velocity on moving to
                                                              set position */
    float       acc;                        /**< [rad/s2|mm/s^2] maximum acceleration on moving to
                                                                 set position */
    int32_t     replyPositionReached;
} __attribute__((packed)) servo_drive_move_pos_data;

class ServoDriveMovePosData
{
    public:
        static void le_to_cpu(servo_drive_move_pos_data *data)
        {
            data->position      = __le32_float_to_cpu(data->position);
            data->vel           = __le32_float_to_cpu(data->vel);
            data->acc           = __le32_float_to_cpu(data->acc);
            data->replyPositionReached =
                                  __le32_to_cpu(data->replyPositionReached);
        }

        static void be_to_cpu(servo_drive_move_pos_data *data)
        {
            data->position      = __be32_float_to_cpu(data->position);
            data->vel           = __be32_float_to_cpu(data->vel);
            data->acc           = __be32_float_to_cpu(data->acc);
            data->replyPositionReached =
                                  __be32_to_cpu(data->replyPositionReached);
        }

        static servo_drive_move_pos_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            servo_drive_move_pos_data *p_data =
                                   (servo_drive_move_pos_data *)msgInfo->p_data;

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
//# ServoDrive Move Vel Data (static size - MESSAGE)
//######################################################################

/**
 * servo drive move velocity data structure
 */
typedef struct {
    float       vel;                        /**< [rad/s|mm/s] set velocity for moving */
} __attribute__((packed)) servo_drive_move_vel_data;

class ServoDriveMoveVelData
{
    public:
        static void le_to_cpu(servo_drive_move_vel_data *data)
        {
            data->vel              = __le32_float_to_cpu(data->vel);
        }

        static void be_to_cpu(servo_drive_move_vel_data *data)
        {
            data->vel              = __be32_float_to_cpu(data->vel);
        }

        static servo_drive_move_vel_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            servo_drive_move_vel_data *p_data =
                                   (servo_drive_move_vel_data *)msgInfo->p_data;

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
 * Hardware abstraction for servo drives and motors.
 *
 * @ingroup proxies_drivers
 */
class ServoDriveProxy : public RackDataProxy {

    public:

        ServoDriveProxy(RackMailbox *workMbx, uint32_t sys_id,
                        uint32_t instance)
                : RackDataProxy(workMbx, sys_id, SERVO_DRIVE, instance)
        {
        };

        ~ServoDriveProxy()
        {
        };

//
// get data
//

        int getData(servo_drive_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp)
        {
            return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
        }

        int getData(servo_drive_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp, uint64_t reply_timeout_ns);

//
// move pos
//

        int movePos(float position, float vel, float acc,
                    uint32_t replyPositionReached,
                    int32_t replyPositionReachedId)
        {
            return movePos(position, vel, acc, replyPositionReached,
                           replyPositionReachedId, dataTimeout);
        }

        int movePos(float position, float vel, float acc,
                    uint32_t replyPositionReached,
                    int32_t replyPositionReachedId, uint64_t reply_timeout_ns);

//
// move vel
//

        int moveVel(float vel)
        {
            return moveVel(vel, dataTimeout);
        }

        int moveVel(float vel, uint64_t reply_timeout_ns);

};

#endif
