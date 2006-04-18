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
 
#ifndef __CHASSIS_PROXY_H__
#define __CHASSIS_PROXY_H__

#include <main/rack_proxy.h>

#define CHASSIS_INVAL_PILOT            0xFFFFFFFF

//######################################################################
//# Chassis Message Types
//######################################################################

#define MSG_CHASSIS_MOVE               (RACK_PROXY_MSG_POS_OFFSET + 1)
#define MSG_CHASSIS_GET_PARAMETER      (RACK_PROXY_MSG_POS_OFFSET + 2)
#define MSG_CHASSIS_SET_ACTIVE_PILOT   (RACK_PROXY_MSG_POS_OFFSET + 3)

#define MSG_CHASSIS_PARAMETER          (RACK_PROXY_MSG_NEG_OFFSET - 1)

//######################################################################
//# Chassis Data (static size  - MESSAGE)
//######################################################################

typedef struct {
    RACK_TIME recordingTime;  // have to be first element
    float32_t deltaX;
    float32_t deltaY;
    float32_t deltaRho;
    float32_t vx;
    float32_t vy;
    float32_t omega;
    float32_t battery;
    uint32_t  activePilot;
} __attribute__((packed)) chassis_data;

class ChassisData
{
    public:
        static void le_to_cpu(chassis_data *data)
        {
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->deltaX        = __le32_float_to_cpu(data->deltaX);
            data->deltaY        = __le32_float_to_cpu(data->deltaY);
            data->deltaRho      = __le32_float_to_cpu(data->deltaRho);
            data->vx            = __le32_float_to_cpu(data->vx);
            data->vy            = __le32_float_to_cpu(data->vy);
            data->omega         = __le32_float_to_cpu(data->omega);
            data->battery       = __le32_float_to_cpu(data->battery);
            data->activePilot   = __le32_to_cpu(data->activePilot);
        }

        static void be_to_cpu(chassis_data *data)
        {
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->deltaX        = __be32_float_to_cpu(data->deltaX);
            data->deltaY        = __be32_float_to_cpu(data->deltaY);
            data->deltaRho      = __be32_float_to_cpu(data->deltaRho);
            data->vx            = __be32_float_to_cpu(data->vx);
            data->vy            = __be32_float_to_cpu(data->vy);
            data->omega         = __be32_float_to_cpu(data->omega);
            data->battery       = __be32_float_to_cpu(data->battery);
            data->activePilot   = __be32_to_cpu(data->activePilot);
        }

        static chassis_data* parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            chassis_data *p_data = (chassis_data *)msgInfo->p_data;

            if (msgInfo->flags & MSGINFO_DATA_LE) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->usedMbx->setDataByteorder(msgInfo);
            return p_data;
        }

};

//######################################################################
//# Chassis Move Data (static size - MESSAGE)
//######################################################################

typedef struct {
    int32_t   vx;
    int32_t   vy;
    float32_t omega;
} __attribute__((packed)) chassis_move_data;

class ChassisMoveData
{
    public:
        static void le_to_cpu(chassis_move_data *data)
        {
            data->vx      = __le32_to_cpu(data->vx);
            data->vy      = __le32_to_cpu(data->vy);
            data->omega   = __le32_float_to_cpu(data->omega);
        }

        static void be_to_cpu(chassis_move_data *data)
        {
            data->vx      = __be32_to_cpu(data->vx);
            data->vy      = __be32_to_cpu(data->vy);
            data->omega   = __be32_float_to_cpu(data->omega);
        }

        static chassis_move_data* parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            chassis_move_data *p_data = (chassis_move_data *)msgInfo->p_data;

            if (msgInfo->flags & MSGINFO_DATA_LE) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->usedMbx->setDataByteorder(msgInfo);
            return p_data;
        }

};

//######################################################################
//# Chassis Parameter Data (static size  - MESSAGE)
//######################################################################

typedef struct
{
  int32_t   vxMax;            // mm/s
  int32_t   vyMax;
  int32_t   vxMin;            // mm/s
  int32_t   vyMin;
  int32_t   axMax;            // mm/s
  int32_t   ayMax;

  float32_t omegaMax;         // rad/s
  float32_t omegaDotMax;      // rad/s
  int32_t   minTurningRadius; // mm

  float32_t breakConstant;    // mm/mm/s
  int32_t   safetyMargin;     // mm
  int32_t   safetyMarginMove; // mm
  int32_t   comfortMargin;    // mm

  int32_t   boundaryFront;    // mm
  int32_t   boundaryBack;
  int32_t   boundaryLeft;
  int32_t   boundaryRight;

  int32_t   wheelBase;        // mm
  int32_t   wheelRadius;      // mm
  int32_t   trackWidth;

  float32_t pilotParameterA;
  float32_t pilotParameterB;

} __attribute__((packed)) chassis_param_data;

class ChassisParamData
{
    public:
        static void le_to_cpu(chassis_param_data *data)
        {
            data->vxMax             = __le32_to_cpu(data->vxMax);
            data->vyMax             = __le32_to_cpu(data->vyMax);
            data->vxMin             = __le32_to_cpu(data->vxMin);
            data->vyMin             = __le32_to_cpu(data->vyMin);
            data->axMax             = __le32_to_cpu(data->axMax);
            data->ayMax             = __le32_to_cpu(data->ayMax);

            data->omegaMax          = __le32_float_to_cpu(data->omegaMax);
            data->omegaDotMax       = __le32_float_to_cpu(data->omegaDotMax);
            data->minTurningRadius  = __le32_to_cpu(data->minTurningRadius);

            data->breakConstant     = __le32_float_to_cpu(data->breakConstant);
            data->safetyMargin      = __le32_to_cpu(data->safetyMargin);
            data->safetyMarginMove  = __le32_to_cpu(data->safetyMarginMove);
            data->comfortMargin     = __le32_to_cpu(data->comfortMargin);

            data->boundaryFront     = __le32_to_cpu(data->boundaryFront);
            data->boundaryBack      = __le32_to_cpu(data->boundaryBack);
            data->boundaryLeft      = __le32_to_cpu(data->boundaryLeft);
            data->boundaryRight     = __le32_to_cpu(data->boundaryRight);

            data->wheelBase         = __le32_to_cpu(data->wheelBase);
            data->wheelRadius       = __le32_to_cpu(data->wheelRadius);
            data->trackWidth        = __le32_to_cpu(data->trackWidth);

            data->pilotParameterA   = __le32_float_to_cpu(data->pilotParameterA);
            data->pilotParameterB   = __le32_float_to_cpu(data->pilotParameterB);
        }

        static void be_to_cpu(chassis_param_data *data)
        {
            data->vxMax             = __be32_to_cpu(data->vxMax);
            data->vyMax             = __be32_to_cpu(data->vyMax);
            data->vxMin             = __be32_to_cpu(data->vxMin);
            data->vyMin             = __be32_to_cpu(data->vyMin);
            data->axMax             = __be32_to_cpu(data->axMax);
            data->ayMax             = __be32_to_cpu(data->ayMax);

            data->omegaMax          = __be32_float_to_cpu(data->omegaMax);
            data->omegaDotMax       = __be32_float_to_cpu(data->omegaDotMax);
            data->minTurningRadius  = __be32_to_cpu(data->minTurningRadius);

            data->breakConstant     = __be32_float_to_cpu(data->breakConstant);
            data->safetyMargin      = __be32_to_cpu(data->safetyMargin);
            data->safetyMarginMove  = __be32_to_cpu(data->safetyMarginMove);
            data->comfortMargin     = __be32_to_cpu(data->comfortMargin);

            data->boundaryFront     = __be32_to_cpu(data->boundaryFront);
            data->boundaryBack      = __be32_to_cpu(data->boundaryBack);
            data->boundaryLeft      = __be32_to_cpu(data->boundaryLeft);
            data->boundaryRight     = __be32_to_cpu(data->boundaryRight);

            data->wheelBase         = __be32_to_cpu(data->wheelBase);
            data->wheelRadius       = __be32_to_cpu(data->wheelRadius);
            data->trackWidth        = __be32_to_cpu(data->trackWidth);

            data->pilotParameterA   = __be32_float_to_cpu(data->pilotParameterA);
            data->pilotParameterB   = __be32_float_to_cpu(data->pilotParameterB);
        }

        static chassis_param_data *parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            chassis_param_data *p_data = (chassis_param_data *)msgInfo->p_data;

            if (msgInfo->flags & MSGINFO_DATA_LE) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->usedMbx->setDataByteorder(msgInfo);
            return p_data;
        }

};

//######################################################################
//# Chassis Set Active Pilot Data (static size  - MESSAGE)
//######################################################################

typedef struct {
    uint32_t  activePilot;    // Command MBX Number of the active pilot (control)
} __attribute__((packed)) chassis_set_active_pilot_data;

class ChassisSetActivePilotData
{
    public:
        static void le_to_cpu(chassis_set_active_pilot_data *data)
        {
            data->activePilot = __le32_to_cpu(data->activePilot);
        }

        static void be_to_cpu(chassis_set_active_pilot_data *data)
        {
            data->activePilot   = __be32_to_cpu(data->activePilot);
        }

        static chassis_set_active_pilot_data* parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            chassis_set_active_pilot_data *p_data =
                               (chassis_set_active_pilot_data *)msgInfo->p_data;

            if (msgInfo->flags & MSGINFO_DATA_LE) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->usedMbx->setDataByteorder(msgInfo);
            return p_data;
        }

};

//######################################################################
//# Chassis Proxy Functions
//######################################################################

class ChassisProxy : public RackDataProxy {

  public:

//
// constructor / destructor
// WARNING -> look at module class id in constuctor
//

    ChassisProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
            : RackDataProxy(workMbx, sys_id, CHASSIS, instance)
    {
    };

    ~ChassisProxy()
    {
    };


//
// overwriting getData proxy function
// (includes parsing and type conversion)
//

    int getData(chassis_data *recv_data, ssize_t recv_datalen, RACK_TIME timeStamp,
                uint64_t reply_timeout_ns, MessageInfo *msgInfo);

    int getData(chassis_data *recv_data, ssize_t recv_datalen, RACK_TIME timeStamp,
                              MessageInfo *msgInfo)
    {
        return getData(recv_data, recv_datalen, timeStamp, dataTimeout, msgInfo);
    }


// move

    int move(int vx, int vy, float omega, uint64_t reply_timeout_ns);
    int move(int vx, int vy, float omega)
    {
        return move(vx, vy, omega, dataTimeout);
    }

    int moveCurve(int speed, float curve, uint64_t reply_timeout_ns)
    {
        float omega = curve * (float)speed;

        return move(speed, 0 , omega, reply_timeout_ns);
    }

    int moveCurve(int speed, float curve)
    {
        return moveCurve(speed, curve, dataTimeout);
    }

    int moveRadius(int speed, int radius, uint64_t reply_timeout_ns)
    {
        float omega;

        if(radius == 0)
        {
            omega = 0.0f;
        }
        else
        {
            omega = (float)speed / (float)radius;
        }

        return move(speed, 0 , omega, reply_timeout_ns);
    }

    int moveRadius(int speed, int radius)
    {
        return moveRadius(speed, radius, dataTimeout);
    }


// getParam


    int getParam(chassis_param_data *recv_data, ssize_t recv_datalen,
                 MessageInfo *msgInfo)
    {
        return getParam(recv_data, recv_datalen, dataTimeout, msgInfo);
    }

    int getParam(chassis_param_data *recv_data, ssize_t recv_datalen,
                 uint64_t reply_timeout_ns, MessageInfo *msgInfo);


// setActivePilot


    int setActivePilot(int pilotMbx)
    {
        return setActivePilot(pilotMbx, dataTimeout);
    }

    int setActivePilot(uint pilotMbxAdr, uint64_t reply_timeout_ns);

};

#endif // __ODOMETRY_PROXY_H__
