/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
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
#include "obj_recog_ibeo_lux.h"


//
// CAN IDs
//

#define CANID_EGO_VEL               (0x303)
#define CANID_EGO_CROSS_ACC         (0x304)
#define CANID_EGO_STEER_WHEEL_ANGLE (0x305)
#define CANID_EGO_YAW_RATE          (0x306)

arg_table_t argTab[] = {

    { ARGOPT_REQ, "canDev", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "CAN device number", { -1 } },

    { ARGOPT_OPT, "baseCanId", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Base CAN id, default 0x500", { 0x500 } },

    { ARGOPT_OPT, "chassisSys", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The system number of the chassis module for ego motion data", { 0 } },

    { ARGOPT_OPT, "chassisInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the chassis module for ego motion data", { -1 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};


/*******************************************************************************
 *   !!! REALTIME CONTEXT !!!
 *
 *   moduleOn,
 *      moduleOff,
 *      moduleLoop,
 *   moduleCommand,
 *
 *   own realtime user functions
 ******************************************************************************/
int ObjRecogIbeoLux::moduleOn(void)
{
    int ret;

    // enable ego motion data
    if (chassisInst >= 0)
    {
        GDOS_DBG_INFO("Turn on Chassis(%d/%d)...\n", chassisSys, chassisInst);
        ret = chassis->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on Chassis(%d/%d), code = %d\n", chassisSys, chassisInst, ret);
            return ret;
        }

        GDOS_DBG_DETAIL("Request chassis parameter\n");
        ret = chassis->getParam(&chassisParam, sizeof(chassisParam));
        if (ret)
        {
            GDOS_ERROR("Can't get Chassis parameter, code = %d\n", ret);
            return ret;
        }

        GDOS_DBG_INFO("Requesting continuous data from Chassis(%d/%d)...\n", chassisSys, chassisInst);
        ret = chassis->getContData(0, &workMbx, &dataBufferPeriodTime);
        if (ret)
        {
            GDOS_ERROR("Can't get continuous data from Chassis(%d/%d), "
                       "code = %d \n", chassisSys, chassisInst, ret);
            return ret;
        }
    }

    return RackDataModule::moduleOn(); // has to be last command in moduleOn();
}

void ObjRecogIbeoLux::moduleOff(void)
{
    RackDataModule::moduleOff();       // has to be first command in moduleOff();

    if (chassisInst >= 0)
    {
        chassis->stopContData(&workMbx);
    }
}

int ObjRecogIbeoLux::moduleLoop(void)
{
    int             ret;
    int16_t         value;
    RackMessage     msgInfo;
    can_frame_t     canFrame;
    obj_recog_data* data = NULL;

    // get datapointer from rackdatabuffer
    data = (obj_recog_data *)getDataBufferWorkSpace();

    data->recordingTime = rackTime.get();
    data->objectNum     = 0;

    putDataBufferWorkSpace(sizeof(obj_recog_data));


    // ego motion data
    if (chassisInst >= 0)
    {
        // get chassis data
        ret = workMbx.recvDataMsgTimed(rackTime.toNano(2 * dataBufferPeriodTime), &chassisData,
                                       sizeof(chassis_data), &msgInfo);
        if (ret)
        {
            GDOS_ERROR("Can't receive chassis data on workMbx, "
                       "code = %d \n", ret);
            return ret;
        }

        if ((msgInfo.getType() == MSG_DATA) ||
            (msgInfo.getSrc()  == chassis->getDestAdr()))
        {
            ChassisData::parse(&msgInfo);

            GDOS_DBG_DETAIL("received chassis data, recordingTime %d, vx %f, omega %a\n",
                            chassisData.recordingTime, chassisData.vx, chassisData.omega);

            // set vehicle speed
            value = (int16_t)(chassisData.vx / 10);
            canFrame.can_id  = CANID_EGO_VEL;
            canFrame.can_dlc = 3;
            canFrame.data[0] = 2;
            canFrame.data[1] = (value >> 8) & 0xff;
            canFrame.data[2] = (value)      & 0xff;

            // send can message
            ret = canPort.send(&canFrame);
            if (ret)
            {
                GDOS_ERROR("Can't send CAN data, code = %d\n", ret);
                return ret;
            }

            // set cross acceleration (v*v / r) = (v / omega)
            if (chassisData.omega != 0)
            {
                value = (int16_t)((-abs(chassisData.vx)) / chassisData.omega);
            }
            else
            {
                value = 0;
            }
            canFrame.can_id  = CANID_EGO_CROSS_ACC;
            canFrame.can_dlc = 3;
            canFrame.data[0] = 2;
            canFrame.data[1] = (value >> 8) & 0xff;
            canFrame.data[2] = (value)      & 0xff;

            // send can message
            ret = canPort.send(&canFrame);
            if (ret)
            {
                GDOS_ERROR("Can't send CAN data, code = %d\n", ret);
                return ret;
            }

            // set steer wheel angle
            if ((int)chassisData.vx != 0)
            {
                value = (int16_t)(-1000.0 * atan((float)chassisParam.trackWidth * chassisData.omega /
                                                       fabs(chassisData.vx)));
            }
            else
            {
                value = 0;
            }
            canFrame.can_id  = CANID_EGO_STEER_WHEEL_ANGLE;
            canFrame.can_dlc = 3;
            canFrame.data[0] = 2;
            canFrame.data[1] = (value >> 8) & 0xff;
            canFrame.data[2] = (value)      & 0xff;

            // send can message
            ret = canPort.send(&canFrame);
            if (ret)
            {
                GDOS_ERROR("Can't send CAN data, code = %d\n", ret);
                return ret;
            }

            // set yaw rate
            value = (int16_t)(-chassisData.omega * 10000.0f);
            canFrame.can_id  = CANID_EGO_YAW_RATE;
            canFrame.can_dlc = 3;
            canFrame.data[0] = 2;
            canFrame.data[1] = (value >> 8) & 0xff;
            canFrame.data[2] = (value)      & 0xff;

            // send can message
            ret = canPort.send(&canFrame);
            if (ret)
            {
                GDOS_ERROR("Can't send CAN data, code = %d\n", ret);
                return ret;
            }
        }
        else
        {
            GDOS_ERROR("Received unexpected message from %n to %n type %d on "
                       "work mailbox\n", msgInfo.getSrc(), msgInfo.getDest(), msgInfo.getType());
            return -EINVAL;
        }
    }

    else
    {
        RackTask::sleep(rackTime.toNano(dataBufferPeriodTime));
    }

    return 0;
}

int ObjRecogIbeoLux::moduleCommand(RackMessage *msgInfo)
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
#define INIT_BIT_CAN_OPEN                   1
#define INIT_BIT_MBX_WORK                   2
#define INIT_BIT_PROXY_CHASSIS              3

int ObjRecogIbeoLux::moduleInit(void)
{
    uint64_t timeout;
    int      ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    // Open CAN port
    ret = canPort.open(canDev, NULL, 0, this);
    if (ret)
    {
        GDOS_ERROR("Can't open can port, code = %d\n", ret);
        goto init_error;
    }
    initBits.setBit(INIT_BIT_CAN_OPEN);
    GDOS_DBG_INFO("Can port has been bound\n");

    // take timestamps
    ret = canPort.getTimestamps();
    if (ret)
    {
        GDOS_ERROR("Can't enable timestamp mode, code = %d\n", ret);
        goto init_error;
    }
    GDOS_DBG_INFO("Timestamp mode has been enabled\n");

    // setting receive timeout
    timeout = 200000000ll; // 200ms
    ret = canPort.setRxTimeout(timeout);
    if (ret)
    {
        GDOS_WARNING("Can't set receive timeout to %d ms, code = %d\n",
                     timeout / 1000000, ret);
        goto init_error;
    }
    GDOS_DBG_INFO("Receive timeout has been set to %d ms\n",
                  timeout / 1000000);

    // setting send timeout
    timeout = 1000000000llu; // 1000ms
    ret = canPort.setTxTimeout(timeout);
    if (ret)
    {
        GDOS_WARNING("Can't set send timeout to %d ms, code = %d\n",
                     timeout / 1000000, ret);
        goto init_error;
    }
    GDOS_DBG_INFO("Send timeout has been set to %d ms\n",
                  timeout / 1000000);

    // work mailbox
    ret = createMbx(&workMbx, 2, sizeof(chassis_param_data),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // create Scan2d Proxy
    if (chassisInst >= 0)
    {
        chassis = new ChassisProxy(&workMbx, chassisSys, chassisInst);
        if (!chassis)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_CHASSIS);
    }

    return 0;

init_error:
    moduleCleanup();
    return ret;
}

void ObjRecogIbeoLux::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    if (initBits.testAndClearBit(INIT_BIT_PROXY_CHASSIS))
    {
        delete chassis;
    }

    // delete mailboxes
    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }

    if (initBits.testAndClearBit(INIT_BIT_CAN_OPEN))
    {
        canPort.close();
    }
}

ObjRecogIbeoLux::ObjRecogIbeoLux()
        : RackDataModule( MODULE_CLASS_ID,
                      5000000000llu,    // 5s datatask error sleep time
                      16,               // command mailbox slots
                      48,               // command mailbox data size per slot
                      MBX_IN_KERNELSPACE | MBX_SLOT, // command mailbox flags
                      5,                // max buffer entries
                      10)               // data buffer listener
{
    // get static module parameter
    canDev      = getIntArg("canDev", argTab);
    baseCanId   = getIntArg("baseCanId", argTab);
    chassisSys  = getIntArg("chassisSys", argTab);
    chassisInst = getIntArg("chassisInst", argTab);

    dataBufferMaxDataSize   = sizeof(obj_recog_data_msg);
    dataBufferPeriodTime    = 1000;
}

int main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "ObjRecogIbeoLux");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new ObjRecogIbeoLux
    ObjRecogIbeoLux *pInst;

    pInst = new ObjRecogIbeoLux();
    if (!pInst)
    {
        printf("Can't create new ObjRecogIbeoLux -> EXIT\n");
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
