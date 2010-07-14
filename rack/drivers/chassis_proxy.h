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

#define CHASSIS_INVAL_PILOT            0xFFFFFF00   /**< preset define for an invalid pilot */

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

/**
 * chassis data structure
 */
typedef struct {
    rack_time_t recordingTime;              /**< [ms] global timestamp (has to be first element)*/
    float       deltaX;                     /**< [mm] longitudinal movement since the last chassis
                                                      data update, main driving direction positive*/
    float       deltaY;                     /**< [mm] lateral movement since the last chassis
                                                      data update, dexter to main driving direction
                                                      positive */
    float       deltaRho;                   /**< [rad] angular movement since the last chassis data
                                                       update, positive clockwise */
    float       vx;                         /**< [mm/s] longitudinal velocity,
                                                        main driving direction positive*/
    float       vy;                         /**< [mm/s] lateral velocity,
                                                        dexter to main driving direction positive */
    float       omega;                      /**< [rad/s] angular velocity, positive clockwise */
    float       battery;                    /**< [v] battery voltage */
    uint32_t    activePilot;                /**< command mailbox adress of active pilot which is
                                                 allowed to send move commands */
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

/**
 * chassis move data structure
 */
typedef struct {
    int32_t   vx;                           /**< [mm/s] longitudinal velocity,
                                                        main driving direction positive*/
    int32_t   vy;                           /**< [mm/s] lateral velocity,
                                                        dexter to main driving direction positive */
    float     omega;                        /**< [rad/s] angular velocity, positive clockwise */
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

/**
 * chassis parameter data structure
 */
typedef struct
{
  int32_t   vxMax;                          /**< [mm/s] maximum longitudinal velocity in main
                                                        driving direction */
  int32_t   vyMax;                          /**< [mm/s] maximum lateral velocity dexter to main
                                                        driving direction */
  int32_t   vxMin;                          /**< [mm/s] minimum longitudinal velocitiy in main
                                                        driving direction */
  int32_t   vyMin;                          /**< [mm/s] minimum lateral velocity dexter to main
                                                        driving direction */

  int32_t   accMax;                         /**< [mm/s^2] maximum acceleration in main driving
                                                          direction */
  int32_t   decMax;                         /**< [mm/s^2] maximum deceleration in main driving
                                                          direction */

  float     omegaMax;                       /**< [rad/s] absolute value of maximum angular
                                                         velocity */
  int32_t   minTurningRadius;               /**< [mm] absolute value of minimum turning radius */

  float     breakConstant;                  /**< [mm/mm/s] constant for adjusting the speed
                                                           reduction in the proximity of obstacles*/
  int32_t   safetyMargin;                   /**< [mm] additional safety margin added around the
                                                      robot bounding box */
  int32_t   safetyMarginMove;               /**< [mm] additional safety margin added to the robot
                                                      bounding box in the current driving
                                                      direction */
  int32_t   comfortMargin;                  /**< [mm] additional comfort margin added laterally to
                                                      the robot bounding, should be maintained to
                                                      obstacles if possible */
  int32_t   boundaryFront;                  /**< [mm] robot bounding box length, distance from the
                                                      robot reference frame to the foremost point */
  int32_t   boundaryBack;                   /**< [mm] robot bounding box length, distance from the
                                                      robot reference frame to the sternmost point*/
  int32_t   boundaryLeft;                   /**< [mm] robot bounding box width, distance from the
                                                      robot reference frame to the leftmost point */
  int32_t   boundaryRight;                  /**< [mm] robot bounding box width, distance from the
                                                      robot reference frame to the rightmost point */

  int32_t   wheelBase;                      /**< [mm] wheel base */
  int32_t   wheelRadius;                    /**< [mm] radius of the drive wheels */
  int32_t   trackWidth;                     /**< [mm] track width */

  float     pilotParameterA;                /**< [rad/mm] control gain for lateral position
                                                          controller */
  float     pilotParameterB;                /**< [1/s]  control gain for angular position
                                                        controller */
  int32_t   pilotVTransMax;                 /**< [mm/s] maximum transversal velocity */

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

/**
 * chassis set active pilot data structure
 */
typedef struct {
    uint32_t  activePilot;                  /**< command mailbox adress of active pilot which is
                                                 allowed to send move commands */
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
