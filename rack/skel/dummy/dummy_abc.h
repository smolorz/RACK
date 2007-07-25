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
#ifndef __DUMMY_ABC_H__
#define __DUMMY_ABC_H__

#include <main/rack_data_module.h>
#include "skel/dummy_proxy.h"

// define module class
#define MODULE_CLASS_ID     TEST

typedef struct {
    dummy_data  data;
    int32_t     value[DUMMY_MAX_VALUE_NUM];
} __attribute__((packed)) dummy_data_msg;

//######################################################################
//# class NewRackDataModule
//######################################################################

class DummyAbc : public RackDataModule {
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
    int  moduleCommand(message_info *msgInfo);

    // -> non realtime context
    void moduleCleanup(void);

  public:

    // constructor und destructor
    DummyAbc();
    ~DummyAbc() {};

    // -> non realtime context
    int  moduleInit(void);

};

#endif // __DUMMY_ABC_H__
