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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
#ifndef __CHASSIS_PIONEER_H__
#define __CHASSIS_PIONEER_H__

#include <main/rack_data_module.h>

#include <main/serial_port.h>
#include <drivers/chassis_proxy.h>

// define module class
#define MODULE_CLASS_ID     CHASSIS

const unsigned char moveTemplate[]     = {0xFA, 0xFB, 6, 11, 0, 0, 0, 0, 0};
const unsigned char turnTemplate[]     = {0xFA, 0xFB, 6, 21, 0, 0, 0, 0, 0};

//######################################################################
//# class ChassisPioneerModule
//######################################################################

class ChassisPioneer : public RackDataModule {
  private:

    // your values
    int             serialDev;
    SerialPort      serialPort;

    int             sonar;

    unsigned char   moveCommand[sizeof(moveTemplate)];
    unsigned char   turnCommand[sizeof(turnTemplate)];

    int             watchdogCounter;
    RackMutex       hwMtx;

    int             leftEncoderOld, rightEncoderOld;
    rack_time_t     oldTimestamp;

    float           battery;
    uint32_t        activePilot;

    int  calculate_checksum(unsigned char *ptr);
    int  receivePackage(unsigned char *sipBuffer, rack_time_t *timestamp);
    int  sendPackage(const unsigned char *package, int packageSize);
    int  sendMovePackage(int vx, float omega);

  protected:

    // -> non realtime context
    int  moduleOn(void);
    void moduleOff(void);
    int  moduleLoop(void);
    int  moduleCommand(message_info *msgInfo);

    // -> non realtime context
    void moduleCleanup(void);

  public:

    // constructor und destructor
    ChassisPioneer();
    ~ChassisPioneer() {};

    // -> non realtime context
    int  moduleInit(void);
};

#endif // __CHASSIS_PIONEER_H__
