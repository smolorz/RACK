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
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#ifndef __CHASSIS_SIM_H__
#define __CHASSIS_SIM_H__

#include <main/rack_datamodule.h>
#include <drivers/chassis_proxy.h>

// define module class
#define MODULE_CLASS_ID     CHASSIS

//######################################################################
//# class ChassisSimModule
//######################################################################

class ChassisSim : public DataModule{
  private:
    chassis_move_data   commandData;
    uint32_t            activePilot;
    RackMutex           mtx;

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
    ChassisSim();
    ~ChassisSim() {};

    // -> non realtime context
    int  moduleInit(void);
};

#endif // __CHASSIS_SIM_H__
