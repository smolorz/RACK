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
#ifndef __VEHICLE_PROXY_H__
#define __VEHICLE_PROXY_H__

#include <main/rack_proxy.h>

//######################################################################
//# Vehicle Message Types
//######################################################################
#define MSG_VEHICLE_SET_VALUE          (RACK_PROXY_MSG_POS_OFFSET + 1)

#define VEHICLE_GEAR_INVALID            -255   /**< preset define for an invalid gear */

//######################################################################
//# Vehicle Data (static size - MESSAGE)
//######################################################################

/**
 * vehicle data structure
 */
typedef struct {
    rack_time_t recordingTime;              /**< [ms] global timestamp (has to be first element)*/
    int32_t     speed;                      /**< [mm/s] longitudinal velocity,
                                                        main driving direction positive*/
    float       omega;                      /**< [rad/s] angular velocity, positive clockwise */
    float       throttle;                   /**< [pct] throttle value,
                                                       0 <= throttle <= 100 pct */
    float       brake;                      /**< [pct] brake value,
                                                       0 <= brake <= 100 pct */
    float       clutch;                     /**< [pct] clutch value,
                                                       0 <= clutch <= 100 pct */
    float       steering;                   /**< [pct] steering value,
                                                       -100 <= steering <= 100 pct */
    int32_t     gear;                       /**< gear,
                                                 <0 = reverse, 0 = neutral, >0 = forward */
    int32_t     engine;                     /**< state of the engine, 1 = running, 0 = off */
    int32_t     parkBrake;                  /**< state of the parking brake,
                                                 1 = activated, 0 = off */
    int32_t     vehicleProtect;             /**< state of the vehicle protection, 1 = on, 0 = off */
    uint32_t    activeController;           /**< id of the actual controller */
} __attribute__((packed)) vehicle_data;

class VehicleData
{
    public:
        static void le_to_cpu(vehicle_data *data)
        {
            data->recordingTime     = __le32_to_cpu(data->recordingTime);
            data->speed             = __le32_to_cpu(data->speed);
            data->omega             = __le32_float_to_cpu(data->omega);
            data->throttle          = __le32_float_to_cpu(data->throttle);
            data->brake             = __le32_float_to_cpu(data->brake);
            data->clutch            = __le32_float_to_cpu(data->clutch);
            data->steering          = __le32_float_to_cpu(data->steering);
            data->gear              = __le32_to_cpu(data->gear);
            data->engine            = __le32_to_cpu(data->engine);
            data->parkBrake         = __le32_to_cpu(data->parkBrake);
            data->vehicleProtect    = __le32_to_cpu(data->vehicleProtect);
            data->activeController  = __le32_to_cpu(data->activeController);
        }

        static void be_to_cpu(vehicle_data *data)
        {
            data->recordingTime     = __be32_to_cpu(data->recordingTime);
            data->speed             = __be32_to_cpu(data->speed);
            data->omega             = __be32_float_to_cpu(data->omega);
            data->throttle          = __be32_float_to_cpu(data->throttle);
            data->brake             = __be32_float_to_cpu(data->brake);
            data->clutch            = __be32_float_to_cpu(data->clutch);
            data->steering          = __be32_float_to_cpu(data->steering);
            data->gear              = __be32_to_cpu(data->gear);
            data->engine            = __be32_to_cpu(data->engine);
            data->parkBrake         = __be32_to_cpu(data->parkBrake);
            data->vehicleProtect    = __be32_to_cpu(data->vehicleProtect);
            data->activeController  = __be32_to_cpu(data->activeController);
        }

        static vehicle_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            vehicle_data *p_data = (vehicle_data *)msgInfo->p_data;

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
//# Vehicle Set Value Data (static size - MESSAGE)
//######################################################################

/**
 * vehicle set value data structure
 */
typedef struct {
    float       throttle;                   /** [pct] throttle, 0 <= throttle <= 100 pct */
    float       brake;                      /** [pct] brake, 0 <= brake <= 100 pct */
    float       clutch;                     /** [pct] clutch, 0 <= clutch <= 100 pct */
    float       steering;                   /** [pct] steering, -100 <= steering <= 100 pct */
    int32_t     gear;                       /** [pct] <0 = reverse, 0 = neutral, >0 = forward */
    int32_t     engine;                     /** state of the engine, 1 = running, 0 = off */
    int32_t     parkBrake;                  /** state of the parking brake,
                                                1 = activated, 0 = off */
    int32_t     vehicleProtect;             /** state of the vehicle protection, 1 = on, 0 = off */
    uint32_t    activeController;           /** id of the actual controller */
} __attribute__((packed)) vehicle_set_value_data;

class VehicleSetValueData
{
    public:
        static void le_to_cpu(vehicle_set_value_data *data)
        {
            data->throttle          = __le32_float_to_cpu(data->throttle);
            data->brake             = __le32_float_to_cpu(data->brake);
            data->clutch            = __le32_float_to_cpu(data->clutch);
            data->steering          = __le32_float_to_cpu(data->steering);
            data->gear              = __le32_to_cpu(data->gear);
            data->engine            = __le32_to_cpu(data->engine);
            data->parkBrake         = __le32_to_cpu(data->parkBrake);
            data->vehicleProtect    = __le32_to_cpu(data->vehicleProtect);
            data->activeController  = __le32_to_cpu(data->activeController);
        }

        static void be_to_cpu(vehicle_set_value_data *data)
        {
            data->throttle          = __be32_float_to_cpu(data->throttle);
            data->brake             = __be32_float_to_cpu(data->brake);
            data->clutch            = __be32_float_to_cpu(data->clutch);
            data->steering          = __be32_float_to_cpu(data->steering);
            data->gear              = __be32_to_cpu(data->gear);
            data->engine            = __be32_to_cpu(data->engine);
            data->parkBrake         = __be32_to_cpu(data->parkBrake);
            data->vehicleProtect    = __be32_to_cpu(data->vehicleProtect);
            data->activeController  = __be32_to_cpu(data->activeController);
        }

        static vehicle_set_value_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            vehicle_set_value_data *p_data = (vehicle_set_value_data *)msgInfo->p_data;

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
 * Hardware abstraction for a nonholonomic vehicle controller.
 *
 * @ingroup proxies_drivers
 */
class VehicleProxy : public RackDataProxy {

 public:

    VehicleProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
          : RackDataProxy(workMbx, sys_id, VEHICLE, instance)
    {
    };

    ~VehicleProxy()
    {
    };

//
// vehicle data
//

    int getData(vehicle_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp)
    {
          return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(vehicle_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp,
                uint64_t reply_timeout_ns);

// set_value
    int setValue(vehicle_set_value_data *recv_data, ssize_t recv_datalen)
    {
        return setValue(recv_data, recv_datalen, dataTimeout);
    }

    int setValue(vehicle_set_value_data *recv_data, ssize_t recv_datalen,
                uint64_t reply_timeout_ns);
};

#endif // __VEHICLE_PROXY_H__
