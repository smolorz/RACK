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
 *      Oliver Wulf        <oliver.wulf@gmx.de>
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 */

#ifndef __DATALOG_REC_H__
#define __DATALOG_REC_H__

#include <main/rack_datamodule.h>
#include <tools/datalog_proxy.h>

#include <drivers/camera_proxy.h>
#include <drivers/chassis_proxy.h>
#include <drivers/gps_proxy.h>
#include <drivers/ladar_proxy.h>
#include <drivers/odometry_proxy.h>
#include <navigation/pilot_proxy.h>
#include <navigation/position_proxy.h>
#include <perception/scan2d_proxy.h>


#define MODULE_CLASS_ID             DATALOG

#define DATALOG_SMALL_MBX_SIZE_MAX            20*1024  //20Kb
#define DATALOG_LARGE_MBX_SIZE_MAX        1*1024*1024  //1Mb

typedef struct {
    datalog_data         data;
    datalog_log_info     logInfo[DATALOG_LOGNUM_MAX];
} __attribute__((packed)) datalog_data_msg;

//######################################################################
//# class DatalogRec
//######################################################################
class DatalogRec : public RackDataModule {
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

        int         initLog;
        float       angleMinFloat;
        float       angleMaxFloat;
        float       ladarOffsetRhoFloat;

        void*       smallContDataPtr;
        void*       largeContDataPtr;

        RackMutex   datalogMtx;

        // additional mailboxes
        RackMailbox workMbx;
        RackMailbox smallContDataMbx;
        RackMailbox largeContDataMbx;

    protected:
        // -> realtime context
        int  moduleOn(void);
        void moduleOff(void);
        int  moduleLoop(void);
        int  moduleCommand(message_info *msgInfo);

        int  getStatus(uint32_t destMbxAdr, RackMailbox *replyMbx,
                       uint64_t reply_timeout_ns);

        int  moduleOn(uint32_t destMbxAdr, RackMailbox *replyMbx,
                      uint64_t reply_timeout_ns);

        int  getContData(uint32_t destMbxAdr, rack_time_t requestPeriodTime,
                         RackMailbox *dataMbx, RackMailbox *replyMbx,
                         rack_time_t *realPeriodTime, uint64_t reply_timeout_ns);

        int  stopContData(uint32_t destMbxAdr, RackMailbox *dataMbx,
                          RackMailbox *replyMbx, uint64_t reply_timeout_ns);

        int  logInfoCurrentModules(datalog_log_info *logInfoAll, int num,
                                   datalog_log_info *logInfoCurrent, RackMailbox *replyMbx,
                                   uint64_t reply_timeout_ns);

        // -> non realtime context
        void moduleCleanup(void);

    public:
        FILE*              fileptr[DATALOG_LOGNUM_MAX];
        datalog_data_msg   datalogInfoMsg;

        virtual void logInfoAllModules(datalog_data *data);
        virtual int  initLogFile();
        virtual int  logData(message_info *msgInfo);

        // constructor und destructor
        DatalogRec();
        ~DatalogRec() {};

        // -> non realtime context
        int  moduleInit(void);
};

#endif // __DATALOG_REC_H__
