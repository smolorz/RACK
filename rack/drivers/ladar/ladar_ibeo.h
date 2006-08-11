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
 *      Oliver Wulf      <wulf@rts.uni-hannover.de>
 *      Daniel Lecking   <lecking@rts.uni-hannover.de>
 *
 */
#ifndef __LADAR_IBEO_H__
#define __LADAR_IBEO_H__

#include <main/rack_datamodule.h>
#include <main/can_port.h>

#include <drivers/ladar_proxy.h>

#define MODULE_CLASS_ID         LADAR

#define MAX_PROFILE_SIZE 2048


//######################################################################
//# class NewRackDataModule
//######################################################################

class LadarIbeo : public RackDataModule {
    private:

        CanPort canPort;
        int canDev;
        int sensorId;
        uint8_t profile[MAX_PROFILE_SIZE];

        int canHostIdBase;
        int canSensorIdBase;
        int hostId;

        rack_time_t   timeOffset;
        rack_time_t   timeOffsetSector[8];

        int sendRequestPackage(int requestCommand, int parameterLen, void* parameter);
        int receiveResponsePackage(int responseCode, int maxParameterLen, void* parameter, rack_time_t* recordingtime);
        int decodeSensorStatus(unsigned int senstat);
        int getSensorStatus(void);
        int transIdle(void);
        int transRot(void);
        int transMeasure(void);
        int getProfile(void);
        int cancelProfile(void);
        int setTimeAbs(void);
        int byteorder_read_be_u16(void* u16);

    protected:

        // -> realtime context
        int  moduleOn(void);
        void moduleOff(void);
        int  moduleLoop(void);
        int  moduleCommand(message_info *msgInfo);

        // -> non realtime context
        void moduleCleanup(void);

    public:

        // constructor und destructor
        LadarIbeo();
        ~LadarIbeo() {};

        // -> non realtime context
        int moduleInit(void);

};

#endif // __NEW_DATA_MODULE_H__
