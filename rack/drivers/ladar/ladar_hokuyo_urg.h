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
#ifndef __LADAR_HOKUYO_URG_H__
#define __LADAR_HOKUYO_URG_H__

#include <main/rack_data_module.h>
#include <main/serial_port.h>

#include <drivers/ladar_proxy.h>

#define MODULE_CLASS_ID         LADAR

typedef struct {
    ladar_data    data;
    int32_t       distance[LADAR_DATA_MAX_DISTANCE_NUM];
} __attribute__((packed)) ladar_data_msg;

struct rtser_config urg_serial_config = {
    config_mask       : 0xFFFF,
    baud_rate         : 19200,
    parity            : RTSER_NO_PARITY,
    data_bits         : RTSER_8_BITS,
    stop_bits         : RTSER_1_STOPB,
    handshake         : RTSER_DEF_HAND,
    fifo_depth        : RTSER_DEF_FIFO_DEPTH,
    rx_timeout        : 200000000llu,
    tx_timeout        : RTSER_DEF_TIMEOUT,
    event_timeout     : 200000000llu,
    timestamp_history : RTSER_RX_TIMESTAMP_HISTORY,
    event_mask        : RTSER_EVENT_RXPEND
};

static char gCommand[] = {'G','0','4','4','7','2','5','0','2',10};
static char sCommand19200[]  = {'S','0','1','9','2','0','0','5','5','5','5','5','5','5',10};
static char sCommand115200[] = {'S','1','1','5','2','0','0','5','5','5','5','5','5','5',10};
static unsigned char serialBuffer[2048];

//######################################################################
//# class NewRackDataModule
//######################################################################

class LadarHokuyoUrg : public RackDataModule {
  private:
    SerialPort  serialPort;
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
    int  moduleCommand(message_info *msgInfo);

    // -> non realtime context
    void moduleCleanup(void);

  public:

    // constructor und destructor
    LadarHokuyoUrg();
    ~LadarHokuyoUrg() {};

    // -> non realtime context
    int  moduleInit(void);

};

#endif
