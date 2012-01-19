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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */

#include "clock_system.h"

#define STATE_INIT                          0
#define STATE_RUN                           1

//
// data structures
//

arg_table_t argTab[] = {

    { ARGOPT_OPT, "clockSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system of the clock input module", { 0 } },

    { ARGOPT_OPT, "clockInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance of the clock input module", { -1 } },

    { ARGOPT_OPT, "systemClockUpdate", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Enable update of the system clock with clock input, 0=off, 1=on, default 1", { 1 } },

    { ARGOPT_OPT, "systemClockUpdateTime", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Time interval for updating the system clock in ms, default 60000 ", { 60000 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};

/*******************************************************************************
 *   !!! REALTIME CONTEXT !!!
 *
 *   moduleOn,
 *   moduleOff,
 *   moduleLoop,
 *   moduleCommand,
 *
 *   own realtime user functions
 ******************************************************************************/
int ClockSystem::moduleOn(void)
{
    int ret;

    // get dynamic module parameter
    systemClockUpdate     = getInt32Param("systemClockUpdate");
    systemClockUpdateTime = getInt32Param("systemClockUpdateTime");

    // use clock input for updating the system time
    if (clockInst >= 0)
    {
        GDOS_DBG_INFO("Turn on Clock(%d/%d)\n", clockSys, clockInst);
        ret = clock->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on Clock(%d/%d), code = %d\n",
                       clockSys, clockInst, ret);
            return ret;
        }
    }


    // init variables
    lastUpdateTime = rackTime.get() - systemClockUpdateTime;
    currSyncMode   = CLOCK_SYNC_MODE_NONE;

    return RackDataModule::moduleOn(); // has to be last command in moduleOn();
}

// realtime context
void ClockSystem::moduleOff(void)
{
    RackDataModule::moduleOff();       // has to be first command in moduleOff();
}

// realtime context
int ClockSystem::moduleLoop(void)
{
    int                 ret;
    clock_data*         p_data;
    timeval             currSystemTime;
    timeval             newSystemTime;
    time_t              rawtime;
    tm*                 ptm;

    // get datapointer from rackdatabuffer
    p_data = (clock_data *)getDataBufferWorkSpace();
    p_data->recordingTime = rackTime.get();

    // try to update system clock
    if ((systemClockUpdate == 1) && (clockInst >= 0))
    {
        // each systemClockUpdateTime
        if (((int)p_data->recordingTime - (int)lastUpdateTime) > systemClockUpdateTime)
        {
            // request clock input data
            ret = clock->getData(&clockData, sizeof(clock_data), 0);
            if (ret)
            {
                GDOS_ERROR("Can't get data from Clock(%i/%i), code = %d\n",
                           clockSys, clockInst, ret);
                return ret;
            }

            // update clock if input data is valid
            if (clockData.syncMode != CLOCK_SYNC_MODE_NONE)
            {
                RackTask::disableRealtimeMode();
                newSystemTime.tv_sec  = clockData.utcTime;
                newSystemTime.tv_usec = ((long)rackTime.get() - (long)clockData.recordingTime) * 1000;
                settimeofday(&newSystemTime, 0);
                RackTask::enableRealtimeMode();

                GDOS_DBG_INFO("update system clock at recordingtime %dms to utc time %ds\n",
                              p_data->recordingTime, clockData.utcTime);
                lastUpdateTime = p_data->recordingTime;
                currSyncMode   = clockData.syncMode;
            }
            else
            {
                currSyncMode = CLOCK_SYNC_MODE_NONE;
            }
        }
    }

    // get current system time
    RackTask::disableRealtimeMode();
    gettimeofday(&currSystemTime, 0);
    time(&rawtime);
    ptm = gmtime(&rawtime);
    RackTask::enableRealtimeMode();

    p_data->hour          = ptm->tm_hour;
    p_data->minute        = ptm->tm_min;
    p_data->second        = ptm->tm_sec;
    p_data->day           = ptm->tm_mday;
    p_data->month         = ptm->tm_mon + 1;
    p_data->year          = ptm->tm_year + 1900;
    p_data->utcTime       = currSystemTime.tv_sec;
    p_data->dayOfWeek     = ptm->tm_wday;
    p_data->syncMode      = currSyncMode;
    p_data->varT          = 0;

    // sunday correction
    if (p_data->dayOfWeek == 0)
    {
        p_data->dayOfWeek = 7;
    }

    GDOS_DBG_DETAIL("recordingtime %i, utcTime %d\n", p_data->recordingTime, p_data->utcTime);
    putDataBufferWorkSpace(sizeof(clock_data));

    sleepDataBufferPeriodTime();
    return 0;
}

int ClockSystem::moduleCommand(RackMessage *msgInfo)
{
    // not for me -> ask RackDataModule
    return RackDataModule::moduleCommand(msgInfo);
}

/*******************************************************************************
 *   !!! NON REALTIME CONTEXT !!!
 *
 *   moduleInit,
 *   moduleCleanup,
 *   Constructor,
 *   Destructor,
 *   main,
 *
 *   own non realtime user functions
 ******************************************************************************/

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE                0
#define INIT_BIT_MBX_WORK                   1
#define INIT_BIT_PROXY_CLOCK                2

int ClockSystem::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    //
    // create mailboxes
    //

    // work mailbox
    ret = createMbx(&workMbx, 10, sizeof(clock_data),
                    MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // clock proxy
    if (clockInst >= 0)
    {
        clock = new ClockProxy(&workMbx, clockSys, clockInst);
        if (!clock)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_CLOCK);
    }

    return 0;

init_error:
    moduleCleanup();
    return ret;
}

void ClockSystem::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    // delete clock proxy
    if (initBits.testAndClearBit(INIT_BIT_PROXY_CLOCK))
    {
        delete clock;
    }

    // delete work mailbox
    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }
}

ClockSystem::ClockSystem()
        : RackDataModule( MODULE_CLASS_ID,
                      5000000000llu,    // 5s datatask error sleep time
                      16,               // command mailbox slots
                      48,               // command mailbox data size per slot
                      MBX_IN_KERNELSPACE | MBX_SLOT, // command mailbox flags
                      5,                // max buffer entries
                      10)               // data buffer listener
{
    // get static module parameter
    clockSys  = getIntArg("clockSys", argTab);
    clockInst = getIntArg("clockInst", argTab);
    dataBufferMaxDataSize = sizeof(clock_data);
    dataBufferPeriodTime  = 5000;
}

int main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "ClockSystem");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new ClockSystem

    ClockSystem *pInst;

    pInst = new ClockSystem();
    if (!pInst)
    {
        printf("Can't create new ClockSystem -> EXIT\n");
        return -ENOMEM;
    }

    // init
    ret = pInst->moduleInit();
    if (ret)
        goto exit_error;

    pInst->run();

    return 0;

exit_error:

    delete (pInst);

    return ret;
}
