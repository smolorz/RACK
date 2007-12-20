/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *      Oliver Wulf <oliver.wulf@web.de>
 *
 */
#include <main/rack_data_module.h>
#include <main/rack_proxy.h>

// init bits
#define INIT_BIT_RACK_MODULE                0
#define INIT_BIT_ENTRIES_CREATED            1
#define INIT_BIT_LISTENER_CREATED           2
#define INIT_BIT_BUFFER_CREATED             3
#define INIT_BIT_BUFFER_MTX_CREATED         4
#define INIT_BIT_LISTENER_MTX_CREATED       5

//######################################################################
//# class RackDataModule
//######################################################################

// non realtime context
RackDataModule::RackDataModule( uint32_t class_id,                // class ID
                        uint64_t dataTaskErrorTime_ns,    // datatask error sleep time
                        int32_t  cmdMbxMsgSlots,          // command mailbox slots
                        uint32_t cmdMbxMsgDataSize,       // command mailbox data size
                        uint32_t cmdMbxFlags,             // command mailbox create flags
                        uint32_t maxDataBufferEntries,    // max buffer entries
                        uint32_t maxDataBufferListener)   // data buffer listener
                        : RackModule(class_id,
                                     dataTaskErrorTime_ns,
                                     cmdMbxMsgSlots,
                                     cmdMbxMsgDataSize,
                                     cmdMbxFlags)
{
    dataBufferMaxEntries    = maxDataBufferEntries;
    dataBufferMaxListener   = maxDataBufferListener;
    dataBufferSendMbx       = 0;

    dataBufferMaxDataSize   = 0;

    dataBufferPeriodTime    = 1000;

    listenerNum             = 0;
    globalDataCount         = 0;
    index                   = 0;

    dataBuffer              = NULL;
    listener                = NULL;

    dataModuleInitBits.clearAllBits();
}

RackDataModule::~RackDataModule()
{
}

//
// private RackDataModule functions
//

// the time is the first data element afer the message head !
// realtime context
rack_time_t RackDataModule::getRecordingTime(void *p_data)
{
  return *(rack_time_t *)p_data;
}

// realtime context (cmdTask)
int         RackDataModule::addListener(rack_time_t periodTime, uint32_t getNextData, uint32_t destMbxAdr,
                                    message_info* msgInfo)
{
    unsigned int i, idx;

    listenerMtx.lock(RACK_INFINITE);

    idx = listenerNum;

    // check if listener is in table yet
    for (i = 0; i < listenerNum; i++)
    {
        if (destMbxAdr == listener[i].msgInfo.src) // in table
        {
            GDOS_WARNING("Listener %n (datambx: %x) is already in list\n",
                         destMbxAdr, destMbxAdr);
            idx = i;
            break;
        }
    }

    if (idx == listenerNum) // not in table
    {
        if (listenerNum >= dataBufferMaxListener)
        {
            listenerMtx.unlock();
            GDOS_ERROR("Can't add listener %n (datambx: %x). To many listener are "
                       "registered (max %d)\n", destMbxAdr, destMbxAdr,
                       dataBufferMaxListener);

            return -EBUSY;
        }

        memcpy(&listener[idx].msgInfo, msgInfo, sizeof(message_info));
        listener[idx].msgInfo.src = destMbxAdr;
        listenerNum++;
    }

    if(getNextData)
    {
        listener[idx].reduction     = 1;
        listener[idx].getNextData   = 1;

        //GDOS_DBG_DETAIL("Add nextData listener %n (datambx: %x) time %i\n",
        //                destMbxAdr, destMbxAdr, rackTime.get());
    }
    else
    {
        if ((periodTime < dataBufferPeriodTime) ||
            (periodTime == 0))
        {
            listener[idx].reduction     = 1; // sending data after every putDataBufferWorkspace()
        }
        else
        {
            listener[idx].reduction     = periodTime / dataBufferPeriodTime;
        }
        listener[idx].getNextData   = 0;

        //GDOS_DBG_DETAIL("Add listener %n (datambx: %x) reduction %d (self %d ms, request %d ms)\n",
        //                destMbxAdr, destMbxAdr, listener[idx].reduction, dataBufferPeriodTime, periodTime);
    }

    listenerMtx.unlock();
    return 0;
}

// realtime context (cmdTask)
void        RackDataModule::removeListener(uint32_t destMbxAdr)
{
    uint32_t read = 0;
    uint32_t write = 0;

    //GDOS_DBG_DETAIL("Remove listener %n (datambx: %x)\n", destMbxAdr, destMbxAdr);

    for(; read < listenerNum; read++)
    {
        if (listener[read].msgInfo.src != destMbxAdr)
        {
            if (read != write)
            {
                memcpy(&listener[write], &listener[read], sizeof(ListenerEntry));
            }
            write++;
        }
    }

    listenerNum = write;
}

// realtime context
void        RackDataModule::removeAllListener(void)
{
    uint32_t i;

    for (i = 0; i < listenerNum; i++)
    {
        GDOS_DBG_INFO("Remove listener %n (datambx: %x)\n", listener[i].msgInfo.src, listener[i].msgInfo.src);
        dataBufferSendMbx->sendMsgReply(MSG_ERROR, &listener[i].msgInfo);
    }

    listenerNum = 0;
}

rack_time_t   RackDataModule::getListenerPeriodTime(uint32_t dataMbx)
{
    uint32_t i;

    listenerMtx.lock(RACK_INFINITE);

    for (i=0; i<listenerNum; i++)
    {
        if (listener[i].msgInfo.src == dataMbx)
        {
            listenerMtx.unlock();
            return listener[i].reduction * dataBufferPeriodTime;
        }
    }

    listenerMtx.unlock();
    return 0;
}

// realtime context (cmdTask)
int         RackDataModule::getDataBufferIndex(rack_time_t time)
{
    int         n;
    rack_time_t timeDiff;
    rack_time_t minTimeDiff = RACK_TIME_MAX;

    int         old_index;
    int         new_index;

    rack_time_t new_rectime;
    rack_time_t old_rectime;

    if (!globalDataCount) // no data available
    {
        GDOS_WARNING("DataBuffer: No data in buffer. Try it again \n");
        return -EFAULT;
    }

    new_index   = index;
    new_rectime = getRecordingTime(dataBuffer[new_index].pData);

    n = globalDataCount > dataBufferMaxEntries ?
        dataBufferMaxEntries : globalDataCount;

    if (time != 0)
    {
        // globalDataCount > 0

        if ((index == globalDataCount) &&
            (globalDataCount < (dataBufferMaxEntries -1)))
        {
            old_index = 1; // in slot 1 are the oldest data
        }
        else
        {
            old_index = (index+2) % dataBufferMaxEntries;
        }

        old_rectime = getRecordingTime(dataBuffer[old_index].pData);

        if (time > ( new_rectime + 2 * dataBufferPeriodTime))
        {
            GDOS_ERROR("DataBuffer: Requested time %d is newer than newest "
                       "data message %d\n", time, new_rectime);
            return -EINVAL;
        }
        else if (time < old_rectime)
        {
            GDOS_ERROR("DataBuffer: Requested time %d is older than oldest "
                       "data message %d\n", time, old_rectime);
            return -EINVAL;
        }

        // loop from oldest to newest data package and find package with
        // minimum timetimeDiff to requested time

        while (n)
        {
            old_rectime = getRecordingTime(dataBuffer[old_index].pData);

            timeDiff = abs(old_rectime - time);
            if (timeDiff <= minTimeDiff)
            {
                minTimeDiff = timeDiff;
                new_index   = old_index;
            }
            else
            {
                // passed local minimum
                break;
            }
            old_index = (old_index + 1) % dataBufferMaxEntries;
            n--;
        }

        if (new_rectime == 0)
        {
            GDOS_ERROR("DataBuffer: Requested time %d is older than oldest "
                       "data message (Buffer is not completely filled)\n", time);
            return -EINVAL;
        }
    }

    return new_index;
}

int         RackDataModule::sendDataReply(rack_time_t time, message_info *msgInfo)
{
    int index, ret;

    if (!msgInfo)
        return -EINVAL;

    bufferMtx.lock(RACK_INFINITE);

    index = getDataBufferIndex(time);

    if(index >= 0)
    {
        ret = dataBufferSendMbx->sendDataMsgReply(MSG_DATA, msgInfo, 1, dataBuffer[index].pData,
                                                  dataBuffer[index].dataSize);
        if(ret)
        {
            GDOS_ERROR("DataBuffer: Can't send data msg (code %d)\n", ret);
        }
        else
        {
        /*
            GDOS_PRINT("Sending Data: %n -> %n, buffer[%d/%d] @ %p, time %d, size %d \n",
                        msgInfo->dest, msgInfo->src, new_index, dataBufferMaxEntries,
                        new_data, getRecordingTime(new_data), dataBuffer[new_index].dataSize);
        */
        }
    }
    else
    {
        ret = index;
    }

    bufferMtx.unlock();
    return ret;
}

//
// public RackDataModule functions
//

// realtime context (dataTask)
void*       RackDataModule::getDataBufferWorkSpace(void)
{
    return dataBuffer[(index+1) % dataBufferMaxEntries].pData;
}

// realtime context (dataTask)
void        RackDataModule::putDataBufferWorkSpace(uint32_t datalength)
{
    uint32_t        i;
    int             ret;

    if ((datalength < 0) || (datalength > dataBufferMaxDataSize))
    {
        GDOS_WARNING("datalen %i doesn't fit dataBufferMaxDataSize %i", datalength, dataBufferMaxDataSize);
        return;
    }

    bufferMtx.lock(RACK_INFINITE);
    listenerMtx.lock(RACK_INFINITE);

    index = (index + 1) % dataBufferMaxEntries;
    globalDataCount += 1;

    if(globalDataCount == 0)  // handle uint32 overflow
        globalDataCount = 1;

/*
    GDOS_PRINT("Put DataBuffer: buffer[%d/%d] @ %p, time %d, size %d \n",
               index, dataBufferMaxEntries, dataBuffer[index].pData,
               getRecordingTime(dataBuffer[index].pData), dataBuffer[index].dataSize);
*/

    dataBuffer[index].dataSize = datalength;

    for (i=0; i<listenerNum; i++)
    {
        if ((globalDataCount % listener[i].reduction) == 0)
        {
/*
            GDOS_ERROR("Sending continuous data to listener[%d] %n, listenernum: %d, "
                       "reduction %d\n", i, listener[i].msgInfo.src,
                       listenerNum, listener[i].reduction);
*/
            ret = dataBufferSendMbx->sendDataMsgReply(MSG_DATA,
                                                      &listener[i].msgInfo,
                                                      1,
                                                      dataBuffer[index].pData,
                                                      dataBuffer[index].dataSize);
            if (ret)
            {
                GDOS_ERROR("DataBuffer: Can't send continuous data "
                           "to listener %n, code = %d\n",
                           listener[i].msgInfo.src, ret);

                removeListener(listener[i].msgInfo.src);
            }
            else if (listener[i].getNextData)
            {
                GDOS_DBG_DETAIL("DataBuffer: Remove nextData listener %n time %i\n",
                                listener[i].msgInfo.src, rackTime.get());

                removeListener(listener[i].msgInfo.src);
            }
        }
    }

    listenerMtx.unlock();
    bufferMtx.unlock();
}

//
// virtual Module functions
//

// non realtime context (linux)
int         RackDataModule::moduleInit(void)
{
    int ret;
    unsigned int i, k;

    // first init module
    ret = RackModule::moduleInit();
    if (ret)
    {
          return ret;
    }
    dataModuleInitBits.setBit(INIT_BIT_RACK_MODULE);

    // now the command mailbox exists
    dataBufferSendMbx = getCmdMbx();

    GDOS_DBG_DETAIL("RackDataModule::moduleInit ... \n");

    // check needed databuffer values
    if (!dataBufferMaxEntries  ||
        !dataBufferMaxDataSize ||
        !dataBufferMaxListener )
    {
        GDOS_ERROR("RackDataModule: Invalid init values\n");
        goto init_error;
    }

    if (dataBuffer || listener)
    {
        GDOS_ERROR("RackDataModule: DataBuffer exists !\n");
        return -EBUSY;
    }

    // create data buffer structures

    dataBuffer = new DataBufferEntry[dataBufferMaxEntries];
    if (!dataBuffer)
    {
        GDOS_ERROR("RackDataModule: DataBuffer entries not created !\n");
        goto init_error;
    }
    dataModuleInitBits.setBit(INIT_BIT_ENTRIES_CREATED);
    GDOS_DBG_DETAIL("DataBuffer dataBuffer table created @ %p\n", dataBuffer);

    for (i=0; i<dataBufferMaxEntries; i++)
    {
        dataBuffer[i].pData = malloc(dataBufferMaxDataSize);
        if (!dataBuffer[i].pData)
        {
            GDOS_ERROR("Error while allocating databuffer dataBuffer[%d] \n", i);
            for (k=i-1; i>=0; k--)
            {
                free(dataBuffer[k].pData);
            }
            goto init_error;
        }
        else
        {
            memset(dataBuffer[i].pData, 0, dataBufferMaxDataSize);
            //GDOS_DBG_DETAIL("DataBuffer dataBuffer @ %p (%d bytes) created\n",
            //                dataBuffer[i].pData, dataBufferMaxDataSize);
        }
        dataBuffer[i].dataSize = 0;
    }
    dataModuleInitBits.setBit(INIT_BIT_BUFFER_CREATED);
    GDOS_DBG_DETAIL("Memory for DataBuffer entries allocated\n");

    // create listener data structures

    listener = new ListenerEntry[dataBufferMaxListener];
    if (!listener)
    {
        GDOS_ERROR("RackDataModule: listener table not created !\n");
        goto init_error;
    }
    dataModuleInitBits.setBit(INIT_BIT_LISTENER_CREATED);
    GDOS_DBG_DETAIL("DataBuffer listener table created @ %p\n", listener);


    ret = bufferMtx.create();
    if (ret) {
        GDOS_ERROR("Error while creating bufferMtx, code = %d \n", ret);
        goto init_error;
    }
    dataModuleInitBits.setBit(INIT_BIT_BUFFER_MTX_CREATED);
    GDOS_DBG_DETAIL("DataBuffer buffer mutex created\n");

    ret = listenerMtx.create();
    if (ret) {
        GDOS_ERROR("Error while creating listenerMtx, code = %d \n", ret);
        goto init_error;
    }
    dataModuleInitBits.setBit(INIT_BIT_LISTENER_MTX_CREATED);
    GDOS_DBG_DETAIL("DataBuffer listener mutex created\n");

    return 0;

init_error:
    // !!! call local cleanup function !!!
    RackDataModule::moduleCleanup();

    return ret;
}

// non realtime context (linux)
void        RackDataModule::moduleCleanup(void)
{
    uint32_t i;

    GDOS_DBG_DETAIL("RackDataModule::moduleCleanup ... \n");

    if (dataModuleInitBits.testAndClearBit(INIT_BIT_RACK_MODULE))
    {
        RackModule::moduleCleanup();
    }

    if (dataModuleInitBits.testAndClearBit(INIT_BIT_LISTENER_MTX_CREATED))
    {
        GDOS_DBG_DETAIL("Deleting dataBuffer listener mutex\n");
        listenerMtx.destroy();
    }

    if (dataModuleInitBits.testAndClearBit(INIT_BIT_BUFFER_MTX_CREATED))
    {
        GDOS_DBG_DETAIL("Deleting dataBuffer buffer mutex\n");
        bufferMtx.destroy();
    }

    if (dataModuleInitBits.testAndClearBit(INIT_BIT_LISTENER_CREATED))
    {
        GDOS_DBG_DETAIL("Deleting dataBuffer listener table @ %p\n", listener);
        delete[] listener;
        listener = NULL;
    }

    if (dataModuleInitBits.testAndClearBit(INIT_BIT_BUFFER_CREATED))
    {
       for (i=0; i<dataBufferMaxEntries; i++)
        {
            //GDOS_DBG_DETAIL("Deleting dataBuffer dataBuffer @ %p (%d bytes)\n",
            //                dataBuffer[i].pData, dataBufferMaxDataSize);
            free(dataBuffer[i].pData);
        }
    }

    if (dataModuleInitBits.testAndClearBit(INIT_BIT_ENTRIES_CREATED))
    {
        GDOS_DBG_DETAIL("Deleting dataBuffer dataBuffer table @ %p\n", dataBuffer);
        delete[] dataBuffer;
        dataBuffer = NULL;
    }
}

// realtime context (dataTask)
int         RackDataModule::moduleOn(void)
{
    int ret;

    if (!dataBufferPeriodTime)
    {
        GDOS_ERROR("Local dataBuffer period time was not set \n");
        GDOS_ERROR("You have to set this value before calling RackDataModule::moduleOn()\n");
        return -EINVAL;
    }

    listenerNum     = 0;
    globalDataCount = 0;
    index           = 0;

    // do moduleLoop until first dataMsg is available
    while(globalDataCount == 0)
    {
        GDOS_DBG_DETAIL("looping for first dataMsg\n");
        ret = moduleLoop();
        if(ret)
        {
            return ret;
        }
    }

    return RackModule::moduleOn();
}

// realtime context (dataTask)
void        RackDataModule::moduleOff(void)
{
    RackModule::moduleOff();          // has to be first command

    listenerMtx.lock(RACK_INFINITE);

    removeAllListener();

    listenerMtx.unlock();
}

// realtime context (cmdTask)
int         RackDataModule::moduleCommand(message_info *msgInfo)
{
    int ret = 0;

    switch (msgInfo->type)
    {

        case MSG_GET_DATA:
        {
            rack_get_data *p_data = RackGetData::parse(msgInfo);

            //GDOS_DBG_DETAIL("CmdTask: GET_DATA: from %n -> %n, recTime: %d\n",
            //                msgInfo->src, msgInfo->dest, p_data->recordingTime);

            if (status == MODULE_STATE_ENABLED)
            {
                ret = sendDataReply(p_data->recordingTime, msgInfo);
                if (ret)
                {
                    ret = cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
                    if (ret)
                    {
                        GDOS_ERROR("CmdTask: Can't send error reply, "
                                   "code = %d\n", ret);
                        return ret;
                    }
                }
            }
            else
            {
                ret = cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
                if (ret)
                {
                    GDOS_ERROR("CmdTask: Can't send error reply, "
                               "code = %d\n", ret);
                    return ret;
                }
            }
            return 0;
        }

        case MSG_GET_CONT_DATA:
        {
            rack_get_cont_data *p_data = RackGetContData::parse(msgInfo);

            GDOS_DBG_DETAIL("CmdTask: GET_CONT_DATA: %n -> %n,type: %d, Prio: %d, "
                            " seq: %d, len: %d, dataMbx: %x, periodTime: %d\n",
                            msgInfo->src, msgInfo->dest, msgInfo->type,
                            msgInfo->priority, msgInfo->seq_nr,
                            msgInfo->datalen, p_data->dataMbxAdr,
                            p_data->periodTime);

            if (status == MODULE_STATE_ENABLED)
            {
                ret = addListener(p_data->periodTime, 0, p_data->dataMbxAdr, msgInfo);
                if (ret)
                {
                    ret = cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
                    if (ret)
                    {
                        GDOS_ERROR("CmdTask: Can't send error reply, "
                                "code = %d\n", ret);
                        return ret;
                    }
                }
                else
                {
                    rack_cont_data contData;
                    contData.periodTime = getListenerPeriodTime(p_data->dataMbxAdr);
                    ret = cmdMbx.sendDataMsgReply(MSG_CONT_DATA, msgInfo, 1,
                                                  &contData,
                                                  sizeof(rack_cont_data));
                    if (ret)
                    {
                        GDOS_ERROR("CmdTask: Can't send get_cont_data reply, "
                                   "code = %d\n", ret);
                        return ret;
                    }
                }
            }
            else // status != MODULE_STATE_ENABLED
            {
                ret = cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
                if (ret)
                {
                    GDOS_ERROR("CmdTask: Can't send error reply, "
                               "code = %d\n", ret);
                    return ret;
                }
            }
            return 0;
        }

        case MSG_STOP_CONT_DATA:
        {
            rack_stop_cont_data *p_data = RackStopContData::parse(msgInfo);

            GDOS_DBG_DETAIL("CmdTask: STOP_CONT_DATA: from %n, to %n, dataMbx: %x\n",
                            msgInfo->src, msgInfo->dest, p_data->dataMbxAdr);

            listenerMtx.lock(RACK_INFINITE);

            removeListener(p_data->dataMbxAdr);

            listenerMtx.unlock();

            ret = cmdMbx.sendMsgReply(MSG_OK, msgInfo);
            if (ret)
            {
                GDOS_ERROR("CmdTask: Can't send ok reply, code = %d\n", ret);
                return ret;
            }
            return 0;
        }

        case MSG_GET_NEXT_DATA:
        {
            GDOS_DBG_DETAIL("CmdTask: GET_NEXT_DATA: %n -> %n, type: %d\n",
                            msgInfo->src, msgInfo->dest, msgInfo->type);

            if (status == MODULE_STATE_ENABLED)
            {
                ret = addListener(0, 1, msgInfo->src, msgInfo);
                if (ret)
                {
                    ret = cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
                    if (ret)
                    {
                        GDOS_ERROR("CmdTask: Can't send error reply, "
                                "code = %d\n", ret);
                        return ret;
                    }
                }
            }
            else // status != MODULE_STATE_ENABLED
            {
                ret = cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
                if (ret)
                {
                    GDOS_ERROR("CmdTask: Can't send error reply, "
                               "code = %d\n", ret);
                    return ret;
                }
            }
            return 0;
        }

        default:
            // not for me -> ask Module
            return RackModule::moduleCommand(msgInfo);
    }
}
