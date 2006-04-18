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
#ifndef __SCAN_2D_H__
#define __SCAN_2D_H__

#include <main/rack_datamodule.h>
#include <perception/scan2d_proxy.h>
#include <drivers/ladar_proxy.h>

#define MODULE_CLASS_ID             SCAN2D

//######################################################################
//# class Scan2d
//######################################################################

class Scan2d : public DataModule {
  	private:

	    // own vars
	    uint32_t    ladarInst;

	    int         ladarOffsetX;
	    int         ladarOffsetY;
	    int         ladarOffsetRho;
	    int         maxRange;
	    int         reduce;
	    int         angleMin;
	    int         angleMax;
	    uint32_t    dataSrcMbxAdr;

	    float       angleMinFloat;
	    float       angleMaxFloat;
	    float       ladarOffsetRhoFloat;

	    // additional mailboxes
	    RackMailbox workMbx;
	    RackMailbox ladarMbx;

	    // proxies
	    LadarProxy  *ladar;

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
	    Scan2d();
	    ~Scan2d() {};

	    // -> non realtime context
	    int  moduleInit(void);
};

#endif // __SCAN_2D_H__
