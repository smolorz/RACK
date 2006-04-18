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
 *      YourName <YourMailAddress>
 *
 */
#ifndef __NEW_DATA_MODULE_H__
#define __NEW_DATA_MODULE_H__

#include <main/rack_datamodule.h>
#include "new_data_module_proxy.h"   // own Proxy header file

// define module class
#define MODULE_CLASS_ID     TEST

typedef struct {
    new_data    data;
    int32_t 	value[NEWDATAMODULE_VALUE_MAX];
} __attribute__((packed)) new_data_msg;

//######################################################################
//# class NewDataModule
//######################################################################

class NewDataModule : public DataModule{
  private:

    // your values
    int reqVal;

    // your mailboxes
    // ...

    // your proxies
    // ...

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
    NewDataModule();
    ~NewDataModule() {};

    // -> non realtime context
    int  moduleInit(void);

};

#endif // __NEW_DATA_MODULE_H__
