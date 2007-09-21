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

#include <navigation/pilot_proxy.h>
#include <main/pilot_tool.h>

// define module class
#define MODULE_CLASS_ID             PILOT


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

        int32_t             chassisInst;
        int32_t             maxSpeed;

        // mailboxes
        RackMailbox         workMbx;                // communication

        // proxies
        ChassisProxy*       chassis;

        // buffer
        chassis_param_data  chasParData;

      protected:
        // -> realtime context
        int      moduleOn(void);
        int      moduleLoop(void);
        void     moduleOff(void);
        int      moduleCommand(message_info *msgInfo);

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
