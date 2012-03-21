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
 *      Thomas Wittmann  <wittmann@rts.uni-hannover.de>
 *
 */
#ifndef __LADAR_HOKUYO_URG_USB_H__
#define __LADAR_HOKUYO_URG_USB_H__

#include <main/rack_data_module.h>
#include <main/serial_port.h>
#include <main/angle_tool.h>

#include <drivers/ladar_proxy.h>

#define MODULE_CLASS_ID         LADAR

typedef struct {
    ladar_data    data;
    ladar_point   point[LADAR_DATA_MAX_POINT_NUM];
} __attribute__((packed)) ladar_data_msg;

static char gCommand[] = {'G','0','4','4','7','2','5','0','2',10};
static char sCommand19200[]  = {'S','0','1','9','2','0','0','5','5','5','5','5','5','5',10};
static char sCommand115200[] = {'S','1','1','5','2','0','0','5','5','5','5','5','5','5',10};
static unsigned char serialBuffer[2048];



/**
 * Ladar Hokuyo URG (via USB)
 *
 * @ingroup modules_ladar
 */
class LadarHokuyoUrgUsb : public RackDataModule {
  private:
    int serialPort;
    int serialDev;
    int start;
    int end;
    int cluster;
    int startAngle;

  protected:

    // -> realtime context
    int  moduleOn(void);
    void moduleOff(void);
    int  moduleLoop(void);
    int  moduleCommand(RackMessage *msgInfo);

    // -> non realtime context
    void moduleCleanup(void);

  public:

    // constructor und destructor
    LadarHokuyoUrgUsb();
    ~LadarHokuyoUrgUsb() {};

    // -> non realtime context
    int  moduleInit(void);

};

#endif
