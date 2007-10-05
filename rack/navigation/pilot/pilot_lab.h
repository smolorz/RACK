/*
 * RACK-RTS - Robotics Application Construction Kit (RTS internal)
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * All rights reserved.
 *
 * Authors
 *
 */

#ifndef __PILOT_LAB_H__
#define __PILOT_LAB_H__

#include <main/rack_data_module.h>

#include <drivers/chassis_proxy.h>
#include <navigation/pilot_proxy.h>
#include <navigation/position_proxy.h>
#include <main/pilot_tool.h>
#include <perception/scan2d_proxy.h>

// define module class
#define MODULE_CLASS_ID             PILOT

// scan_2d data message (use max message size)
typedef struct {
    scan2d_data   data;
    scan_point    point[SCAN2D_POINT_MAX];
} __attribute__((packed)) scan2d_data_msg;

// pilot data message (use no splines)
typedef struct {
     pilot_data        data;
     polar_spline      spline[0];
} __attribute__((packed)) pilot_data_msg;


//######################################################################
//# class PilotLab
//######################################################################

class PilotLab : public RackDataModule {
      private:

        // external module parameter
        int                 chassisInst;
        int                 positionInst;
        int                 scan2dInst;
        int                 speedMax;
        float               omegaMax;
        int                 varDistance;
        float               varRho;
        int                 distanceMin;

        // mailboxes
        RackMailbox         workMbx;                // communication
        RackMailbox         scan2dDataMbx;          // scan2d data

        // proxies
        ChassisProxy*       chassis;
        PositionProxy*      position;
        Scan2dProxy*        scan2d;

        // data structures
        chassis_param_data  chasParData;
        position_data       positionData;
        scan2d_data_msg     scan2dMsg;
        pilot_dest_data     pilotDest;

        // variables
        int                 comfortDistance;
        int                 zMin;
        int                 pilotState;
        int                 speed;
        float               omega;

      protected:
        // -> realtime context
        int      moduleOn(void);
        int      moduleLoop(void);
        void     moduleOff(void);
        int      moduleCommand(message_info *msgInfo);

        float controlOmega(int speed, int dCurr, int dSet, float rhoCurr, float rhoSet,
                           chassis_param_data *chassisParam);

        // -> non realtime context
        void     moduleCleanup(void);

      public:
        // constructor und destructor
        PilotLab();
        ~PilotLab() {};

        // -> non realtime context
        int  moduleInit(void);
};

#endif // __PILOT_LAB_H__
