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

#define PROFILEFORMAT_PROFILESENT  (1 << 0)
#define PROFILEFORMAT_PROFILECOUNT (1 << 1)
#define PROFILEFORMAT_LAYERNUM     (1 << 2)
#define PROFILEFORMAT_SECTORNUM    (1 << 3)
#define PROFILEFORMAT_DIRSTEP      (1 << 4)
#define PROFILEFORMAT_POINTNUM     (1 << 5)
#define PROFILEFORMAT_TSTART       (1 << 6)
#define PROFILEFORMAT_STARTDIR     (1 << 7)
#define PROFILEFORMAT_DISTANCE     (1 << 8)
#define PROFILEFORMAT_TEND         (1 << 11)
#define PROFILEFORMAT_ENDDIR       (1 << 12)
#define PROFILEFORMAT_SENSTAT      (1 << 13)

#define IDLE_MODE    0x1
#define ROTATE_MODE  0x2
#define MEASURE_MODE 0x3

typedef struct {
    unsigned int senstat;
} STATUS_RESPONSE;

typedef struct {
    unsigned short rev;
} TRANS_ROT_REQUEST;

struct _TRANS_MEASURE_RESPONSE {
    unsigned int   senstat;
    unsigned short errorcode;
} __attribute__ ((packed));
typedef struct _TRANS_MEASURE_RESPONSE TRANS_MEASURE_RESPONSE;

typedef struct {
    unsigned short syncabs;
} SET_TIME_ABS;

typedef struct {
    unsigned short synctime;
} SET_TIME_ABS_RESPONSE;

typedef struct {
    unsigned short profilenum;
    unsigned short profileformat;
} GET_PROFILE_REQUEST;

typedef struct {
    uint16_t word[4];
} UINT16_PACKAGE;

//######################################################################
//# class NewDataModule
//######################################################################

class LadarIbeo : public DataModule{
  private:

    // your values
    CanPort     canPort;
    int         canDev;
    int         sensorId;
    uint8_t     profile[MAX_PROFILE_SIZE];

    int         canHostIdBase;
    int         canSensorIdBase;
    int         hostId;

    RACK_TIME   timeOffset;
    RACK_TIME   timeOffsetSector[8];

    int         sendRequestPackage(int requestCommand, int parameterLen, void* parameter);
    int         receiveResponsePackage(int responseCode, int maxParameterLen, void* parameter, RACK_TIME* recordingtime);
    int         decodeSensorStatus(unsigned int senstat);
    int         getSensorStatus(void);
    int         transIdle(void);
    int         transRot(void);
    int         transMeasure(void);
    int         getProfile(void);
    int         cancelProfile(void);
    int         setTimeAbs(void);
    int         byteorder_read_be_u16(void* u16);

  protected:

    // -> realtime context
    int  moduleOn(void);
    void moduleOff(void);
    int  moduleLoop(void);
    int  moduleCommand(MessageInfo *msgInfo);

	// -> non realtime context
	void moduleCleanup(void);

  public:

    // constructor und destructor
    LadarIbeo();
    ~LadarIbeo() {};

    // -> non realtime context
    int  moduleInit(void);

};

#endif // __NEW_DATA_MODULE_H__
