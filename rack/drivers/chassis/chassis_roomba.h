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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
#ifndef __CHASSIS_ROOMBA_H__
#define __CHASSIS_ROOMBA_H__

#include <main/rack_data_module.h>

#include <main/serial_port.h>
#include <drivers/chassis_proxy.h>

// define module class
#define MODULE_CLASS_ID     CHASSIS

#define CHASSIS_ROOMBA_ROI_START       0x80
#define CHASSIS_ROOMBA_ROI_CONTROL     0x82
#define CHASSIS_ROOMBA_ROI_FULL        0x84
#define CHASSIS_ROOMBA_ROI_DRIVE       0x89
#define CHASSIS_ROOMBA_ROI_MOTORS      0x8A
#define CHASSIS_ROOMBA_ROI_SENSORS     0x8E

#define CHASSIS_ROOMBA_RADIUS_MAX      2000                     // mm
#define CHASSIS_ROOMBA_DEGREES_PER_MM  360.0 / (258.0 * M_PI)

typedef struct {
    rack_time_t recordingTime;
    int         bumpAndWheeldrop;
    int         wall;
    int         cliffLeft;
    int         cliffFrontLeft;
    int         cliffFrontRight;
    int         cliffRight;
    int         virtualWall;
    int         motorOvercurrents;
    int         dirtDetectorLeft;
    int         dirtDetectorRight;
    int         remoteControl;
    int         buttons;
    int         distance;
    int         angle;
    int         chargingState;
    int         batteryVoltage;
    int         batteryCurrent;
    int         batteryTemperature;
    int         batteryCharge;
    int         batteryCapacity;
} __attribute__((packed)) chassis_roomba_sensor_data;

//######################################################################
//# class ChassisRoombaModule
//######################################################################
class ChassisRoomba : public RackDataModule {
  private:
    int                         motorMainBrush;
    int                         motorVacuum;
    int                         motorSideBrush;

    // your values
    int                         serialDev;
    SerialPort                  serialPort;
    


    char                        serialBuffer[30];
    rack_time_t                 recordingTimeOld;
    chassis_roomba_sensor_data  sensorData;

    RackMutex                   hwMtx;

    uint32_t                    activePilot;
    int                         speed;
    float                       omega;

  protected:

    // -> non realtime context
    int  moduleOn(void);
    void moduleOff(void);
    int  moduleLoop(void);
    int  moduleCommand(message_info *msgInfo);

    // -> non realtime context
    void moduleCleanup(void);

    int setCleaningMotor(int mainBrush, int vacuum, int sideBrush);
    int sendMoveCommand(int speed, float omega);
    int readSensorData(chassis_roomba_sensor_data *sensor);

  public:

    // constructor und destructor
    ChassisRoomba();
    ~ChassisRoomba() {};

    // -> non realtime context
    int  moduleInit(void);
};

#endif // __CHASSIS_ROOMBA_H__
