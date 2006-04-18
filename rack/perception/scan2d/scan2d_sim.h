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
#ifndef __SCAN_2D_SIM_H__
#define __SCAN_2D_SIM_H__

#include <main/rack_datamodule.h>
#include <main/dxf_map.h>
#include <perception/scan2d_proxy.h>
#include <drivers/odometry_proxy.h>

#define MODULE_CLASS_ID             SCAN2D

//######################################################################
//# class Scan2DSim
//######################################################################

class Scan2DSim : public DataModule {
  	private:
	    uint32_t    odometryInst;
	    uint32_t    maxRange;

        DxfMap      dxfMap;

	    // additional mailboxes
	    RackMailbox workMbx;
	    RackMailbox odometryMbx;

	    // proxies
	    OdometryProxy  *odometry;

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
	    Scan2DSim();
	    ~Scan2DSim() {};

	    // -> non realtime context
	    int  moduleInit(void);
};

#endif // __SCAN_2D_SIM_H__
