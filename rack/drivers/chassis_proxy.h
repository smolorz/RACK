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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */

#ifndef __CHASSIS_PROXY_H__
#define __CHASSIS_PROXY_H__

/*!
 * @ingroup drivers
 * @defgroup chassis Chassis
 *
 * Hardware abstraction for mobile robot chassis.
 *
 * @{
 */

#include <main/rack_proxy.h>

#define CHASSIS_INVAL_PILOT            0xFFFFFF00

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

typedef struct chassis_data_s {
    rack_time_t recordingTime;  // has to be first element
    float       deltaX;
    float       deltaY;
    float       deltaRho;
    float       vx;
    float       vy;
    float       omega;
    float       battery;
    uint32_t    activePilot;
} __attribute__((packed)) chassis_data;

class ChassisData : public RackDataMessage
{
    public:
        static chassis_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            chassis_data *data = (chassis_data *)msgInfo->p_data;

            data->recordingTime = msgInfo->data32ToCpu(data->recordingTime);
            data->deltaX        = msgInfo->data32FloatToCpu(data->deltaX);
            data->deltaY        = msgInfo->data32FloatToCpu(data->deltaY);
            data->deltaRho      = msgInfo->data32FloatToCpu(data->deltaRho);
            data->vx            = msgInfo->data32FloatToCpu(data->vx);
            data->vy            = msgInfo->data32FloatToCpu(data->vy);
            data->omega         = msgInfo->data32FloatToCpu(data->omega);
            data->battery       = msgInfo->data32FloatToCpu(data->battery);
            data->activePilot   = msgInfo->data32ToCpu(data->activePilot);

            msgInfo->setDataByteorder();

            return data;
        }
};

//######################################################################
//# Chassis Move Data (static size - MESSAGE)
//######################################################################

typedef struct chassis_move_data_s {
    int32_t   vx;
    int32_t   vy;
    float     omega;
} __attribute__((packed)) chassis_move_data;

class ChassisMoveData : public RackMessage
{
    public:
        static chassis_move_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            chassis_move_data *data = (chassis_move_data *)msgInfo->p_data;

            data->vx      = msgInfo->data32ToCpu(data->vx);
            data->vy      = msgInfo->data32ToCpu(data->vy);
            data->omega   = msgInfo->data32FloatToCpu(data->omega);

            msgInfo->setDataByteorder();

            return data;
        }
};

//######################################################################
//# Chassis Parameter Data (static size  - MESSAGE)
//######################################################################

typedef struct chassis_param_data_s
{
  int32_t   vxMax;            /**< mm/s */
  int32_t   vyMax;
  int32_t   vxMin;            /**< mm/s */
  int32_t   vyMin;

  int32_t   accMax;           /**< mm/s/s */
  int32_t   decMax;

  float     omegaMax;         /**< rad/s */
  int32_t   minTurningRadius; /**< mm */

  float     breakConstant;    /**< mm/mm/s */
  int32_t   safetyMargin;     /**< mm */
  int32_t   safetyMarginMove; /**< mm */
  int32_t   comfortMargin;    /**< mm */

  int32_t   boundaryFront;    /**< Boundary in front of the robot [mm] */
  int32_t   boundaryBack;
  int32_t   boundaryLeft;
  int32_t   boundaryRight;

  int32_t   wheelBase;        /**< mm */
  int32_t   wheelRadius;      /**< mm */
  int32_t   trackWidth;

  float     pilotParameterA;
  float     pilotParameterB;
  int32_t   pilotVTransMax;   /**< Maximal transversal velocity in mm /s */

} __attribute__((packed)) chassis_param_data;

class ChassisParamData : public RackMessage
{
    public:
        static chassis_param_data *parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            chassis_param_data *data = (chassis_param_data *)msgInfo->p_data;

            data->vxMax             = msgInfo->data32ToCpu(data->vxMax);
            data->vyMax             = msgInfo->data32ToCpu(data->vyMax);
            data->vxMin             = msgInfo->data32ToCpu(data->vxMin);
            data->vyMin             = msgInfo->data32ToCpu(data->vyMin);

            data->accMax            = msgInfo->data32ToCpu(data->accMax);
            data->decMax            = msgInfo->data32ToCpu(data->decMax);

            data->omegaMax          = msgInfo->data32FloatToCpu(data->omegaMax);
            data->minTurningRadius  = msgInfo->data32ToCpu(data->minTurningRadius);

            data->breakConstant     = msgInfo->data32FloatToCpu(data->breakConstant);
            data->safetyMargin      = msgInfo->data32ToCpu(data->safetyMargin);
            data->safetyMarginMove  = msgInfo->data32ToCpu(data->safetyMarginMove);
            data->comfortMargin     = msgInfo->data32ToCpu(data->comfortMargin);

            data->boundaryFront     = msgInfo->data32ToCpu(data->boundaryFront);
            data->boundaryBack      = msgInfo->data32ToCpu(data->boundaryBack);
            data->boundaryLeft      = msgInfo->data32ToCpu(data->boundaryLeft);
            data->boundaryRight     = msgInfo->data32ToCpu(data->boundaryRight);

            data->wheelBase         = msgInfo->data32ToCpu(data->wheelBase);
            data->wheelRadius       = msgInfo->data32ToCpu(data->wheelRadius);
            data->trackWidth        = msgInfo->data32ToCpu(data->trackWidth);

            data->pilotParameterA   = msgInfo->data32FloatToCpu(data->pilotParameterA);
            data->pilotParameterB   = msgInfo->data32FloatToCpu(data->pilotParameterB);
            data->pilotVTransMax    = msgInfo->data32ToCpu(data->pilotVTransMax);

            msgInfo->setDataByteorder();

            return data;
        }

};

//######################################################################
//# Chassis Set Active Pilot Data (static size  - MESSAGE)
//######################################################################

typedef struct chassis_set_active_pilot_data_s
{
    uint32_t  activePilot;    /**< Command MBX Number of the active pilot (control) */

} __attribute__((packed)) chassis_set_active_pilot_data;

class ChassisSetActivePilotData : public RackMessage
{
    public:
        static chassis_set_active_pilot_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            chassis_set_active_pilot_data *data = (chassis_set_active_pilot_data *)msgInfo->p_data;

            data->activePilot = msgInfo->data32ToCpu(data->activePilot);

            msgInfo->setDataByteorder();

            return data;
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

    int getData(chassis_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp,
                uint64_t reply_timeout_ns);

    int getData(chassis_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp)
    {
        return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }


// move

    int move(int32_t vx, int32_t vy, float omega, uint64_t reply_timeout_ns);
    int move(int32_t vx, int32_t vy, float omega)
    {
        return move(vx, vy, omega, dataTimeout);
    }

    int moveCurve(int32_t speed, float curve, uint64_t reply_timeout_ns)
    {
        float omega = curve * (float)speed;

        return move(speed, 0 , omega, reply_timeout_ns);
    }

    int moveCurve(int32_t speed, float curve)
    {
        return moveCurve(speed, curve, dataTimeout);
    }

    int moveRadius(int32_t speed, int32_t radius, uint64_t reply_timeout_ns)
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

    int moveRadius(int32_t speed, int32_t radius)
    {
        return moveRadius(speed, radius, dataTimeout);
    }


// getParam


    int getParam(chassis_param_data *recv_data, ssize_t recv_datalen)
    {
        return getParam(recv_data, recv_datalen, dataTimeout);
    }

    int getParam(chassis_param_data *recv_data, ssize_t recv_datalen,
                 uint64_t reply_timeout_ns);


// setActivePilot


    int setActivePilot(uint32_t pilotMbx)
    {
        return setActivePilot(pilotMbx, dataTimeout);
    }

    int setActivePilot(uint32_t pilotMbxAdr, uint64_t reply_timeout_ns);

};

/*@}*/

#endif // __CHASSIS_PROXY_H__
