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
 *      Marko Reimer <reimer@rts.uni-hannover.de>
 *
 */
#ifndef __CHASSIS_ER1_H__
#define __CHASSIS_ER1_H__

#include <main/rack_data_module.h>

//#include <main/serial_port.h>
#include <drivers/chassis_proxy.h>

#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <linux/serial.h>

// define module class
#define MODULE_CLASS_ID     CHASSIS

#define ClearInterrupt 0xAC
#define AdjustActualPostion 0xF5
#define ClearPositionError 0x47
#define GetActivityStatus 0xA6
#define GetActualVelocity 0xAD
#define GetCaptureValue 0x36
#define GetChecksum 0xF8
#define GetCommandedAcceleration 0xA7
#define GetCommandedPostion 0x1D
#define GetCommandedVelocity 0x1E
#define GetCurrentMotorCommand 0x3A
#define GetDerivative 0x9B
#define GetEventStatus 0x31
#define GetHostIOError 0xA5
#define GetIntegral 0x9A
#define GetInterruptAxis 0xE1
#define GetPhaseCommand 0xEA
#define GetPositionError 0x99
#define GetSignalStatus 0xA4
#define GetTime 0x3E
#define GetTraceCount 0xBB
#define GetTraceStatus 0xBA
#define GetVersion 0x8F
#define InitializPhase 0x7A
#define NoOperation 0x00
#define ReadAnalog 0xEF
#define ReadBuffer 0xC9
#define ReadIO 0x83
#define Reset 0x39
#define ResetEventStatus 0x34
#define SetAcceleration 0x90
#define GetAcceleration 0x4C
#define SetActualPostion 0x4D
#define GetActualPostion 0x37
#define SetActualPostionUnits 0xBE
#define GetActualPostionUnits 0xBF
#define SetAutoStopMode 0xD2
#define GetAutoStopMode 0xD3
#define SetAxisMode 0x87
#define GetAxisMode 0x88
#define SetAxisOutSource 0xED
#define GetAxisOutSource 0xEE
#define SetBreakpoint 0xD4
#define GetBreakpoint 0xD5
#define SetBreakpointValue 0xD6
#define GetBreakpointValue 0xD7
#define SetBufferFunction 0xCA
#define GetBufferFunction 0xCB
#define SetbufferLength 0xC2
#define GetBufferLength 0xC3
#define SetBufferReadIndex 0xC6
#define GetBufferReadIndex 0xC7
#define SetBufferStart 0xC0
#define GetBufferStart 0xC1
#define SetBufferWriteIndex 0xC4
#define GetBufferWriteIndex 0xC5
#define SetCaptureSource 0xD8
#define GetCaptureSource 0xD9
#define SetCommutationMode 0xE2
#define GetCommutationMode 0xE3
#define SetDeceleration 0x91
#define GetDeceleration 0x92
#define SetDerivativeTime 0x9C
#define GetDerivativeTime 0x9D
#define SetDiagnosticPortMode 0x89
#define GetDiagnosticPortMode 0x8A
#define SetEncoderModulus 0x8D
#define GetEncoderModulus 0x8E
#define SetEncoderSource 0xDA
#define GetEncoderSource 0xDB
#define SetEncoderToStepRatio 0xDE
#define GetEncoderToStepRatio 0xDF
#define SetIntegrationLimit 0x95
#define GetIntegrationLimit 0x96
#define SetInterruptMask 0x2F
#define GetInterruptMask 0x56
#define SetJerk 0x13
#define GetJerk 0x58
#define SetKaff 0x93
#define GetKaff 0x94
#define SetKd 0x27
#define GetKd 0x52
#define SetKi 0x26
#define GetKi 0x51
#define SetKout 0x9E
#define GetKout 0x9F
#define SetKp 0x25
#define GetKp 0x50
#define SetKvff 0x2B
#define GetKvff 0x54
#define SetLimitSwitchMode 0x80
#define GetLimitSwitchMode 0x81
#define SetMotionCompleteMode 0xEB
#define GetMotionCompleteMode 0xEC
#define SetMotorBias 0x0F
#define GetMotorBias 0x2D
#define SetMotorCommand 0x77
#define GetMotorCommand 0x69
#define SetMotorLimit 0x06
#define GetMotorLimit 0x07
#define SetMotorMode 0xDC
#define GetMotorMode 0xDD
#define SetNumberPhases 0x85
#define GetNumberPhases 0x86
#define SetOutputMode 0xE0
#define GetOutputMode 0x6E
#define SetPhaseAngle 0x84
#define GetPhaseAngle 0x2C
#define SetPhaseCorrectionMode 0xE8
#define GetPhaseCorrectionMode 0xE9
#define SetPhaseCounts 0x75
#define GetPhaseCounts 0x7D
#define SetPhaseInitializeMode 0xE4
#define GetPhaseInitializeMode 0xE5
#define SetPhaseInitializeTime 0x72
#define GetPhaseInitializeTime 0x7C
#define SetPhaseOffset 0x76
#define GetPhaseOffset 0x7B
#define SetPhasePrescale 0xE6
#define GetPhasePrescale 0xE7
#define SetPostion 0x10
#define GetPostion 0x4A
#define SetPostionErrorLimit 0x97
#define GetPostionErrorLimit 0x98
#define SetProfileMode 0xA0
#define GetProfileMode 0xA1
#define SetSampleTime 0x38
#define GetSampleTime 0x61
#define SetSerialPortMode 0x8B
#define GetSerialPortMode 0x8C
#define SetSettleTime 0xAA
#define GetSettleTime 0xAB
#define SetSettleWindow 0xBC
#define GetSettleWindow 0xBD
#define SetSignalSense 0xA2
#define GetSignalSense 0xA3
#define SetStartMode 0xCC
#define GetStartMode 0xCD
#define SetStartVelocity 0x6A
#define GetStartVelocity 0x6B
#define SetStopMode 0xD0
#define GetStopMode 0xD1
#define SetSynchronizationMode 0xF2
#define GetSynchronizationMode 0xF3
#define SetTraceMode 0xB0
#define GetTraceMode 0xB1
#define SetTracePeriod 0xB8
#define GetTracePeriod 0xB9
#define SetTraceStart 0xB2
#define GetTraceStart 0xB3
#define SetTraceStop 0xB4
#define GetTraceStop 0xB5
#define SetTraceVariable 0xB6
#define GetTraceVariable 0xB7
#define SetTrackingWindow 0xA8
#define GetTrackingWindow 0xA9
#define SetVelocity 0x11
#define GetVelocity 0xAB
#define Update 0x1A
#define WriteBuffer 0xC8
#define WriteIO 0x82
/* End: All Pilot Mition Processor are defined here */
/* Begin: Function error code are defined here */
#define ERR_RCM_NO_ERROR 0
#define ERR_RCM_INVALID_PORT 1
#define ERR_RCM_OPENPORT 2
/* End: Function error code are defined here */
/* Begin: Define constant for chassis_er1_reply */
#define STATUS 0
#define CHECKSUM 1
#define DATA_START 2
#define DATA_END 7
#define CHECKSUMVALID 0
/* END: Define constant for chassis_er1_reply */
#define PWMREPLYSIZE 8

#define LOBYTE(w) ((unsigned char)(w))
#define HIBYTE(w) ((unsigned char)(((int)(w) >> 8) & 0xFF))
#define HIWORD(w) ((int) (((long int)(w) >> 8) & 0xffff))
#define LOWORD(w) ((int)(w))


typedef struct chassis_er1_cmd_struct {
    unsigned char address;
    unsigned char checksum;
    unsigned char axis; // initialized by constructor, always 0
    unsigned char code;
    unsigned char data[6];
    unsigned char size; // meta data, should be not be send.
} chassis_er1_cmd;

typedef struct chassis_er1_reply_struct {
    unsigned char data[PWMREPLYSIZE];
    long dwSize;
} chassis_er1_reply;

typedef struct chassis_er1_rcm_struct {
    int handle;
    int nosend ,debug;
} chassis_er1_rcm;


//######################################################################
//# class ChassisEr1Module
//######################################################################

class ChassisEr1 : public RackDataModule {
  private:

    // your values
    char           *serialDev;
    int             axisWidth;

    int             watchdogCounter;
    float           omegaMin;
    RackMutex       hwMtx;
    float           velFactor;
    float           odoFactorA;
    float           odoFactorB;
    float           battery;
    uint32_t        activePilot;
    float           speedLeft, speedRight; //in mm per sec
    rack_time_t     oldTime;

    chassis_er1_rcm rcm;

    int  getPositionPackage(int *leftPos, int *rightPos);
    long pwdreply_status(chassis_er1_reply *r);
    long pwdreply_checksum(chassis_er1_reply *r);
    void pwdreply_print(chassis_er1_reply *r);
    int  pwdreply_data(chassis_er1_reply *r);
    int  sendMovePackage(int vx, float omega);

    void rcm_WheelUpdate();
    void rcm_WheelSetPower(long);
    void rcm_WheelInit();
    void rcm_WheelSetVelocity(long);
    void rcm_WheelSetVelocity_LR(long,long);
    void rcm_WheelSetTurn(long);

    chassis_er1_cmd   SetCmd(unsigned char  paddress, unsigned char pcode);
    chassis_er1_cmd   SetCmd_with_arg(unsigned char  paddress, unsigned char  pcode, int  wData1);
    chassis_er1_cmd   SetCmd_with_arg2(unsigned char paddress, unsigned char pcode, int wData1, int wData2);
    chassis_er1_reply _SendCmd(chassis_er1_cmd *cmd);
    chassis_er1_reply SendCmd_with_arg2(unsigned char paddress, unsigned char pcode, int wData1, int wData2);
    chassis_er1_reply SendCmd_with_arg(unsigned char paddress, unsigned char pcode, int wData1);
    chassis_er1_reply SendCmd(unsigned char paddress, unsigned char pcode);
    chassis_er1_reply SendCmd_long(unsigned char paddress, unsigned char pcode, long dwData);

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
    ChassisEr1();
    ~ChassisEr1() {};

    // -> non realtime context
    int  moduleInit(void);
};

#endif // __CHASSIS_ER1_H__
