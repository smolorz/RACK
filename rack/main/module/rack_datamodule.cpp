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
 *
 */
#include <main/rack_datamodule.h>
#include <main/rack_proxy.h>

// init bits
#define INIT_BIT_RACKMODULE                 0
#define INIT_BIT_ENTRIES_CREATED            1
#define INIT_BIT_LISTENER_CREATED           2
#define INIT_BIT_BUFFER_CREATED             3
#define INIT_BIT_BUFFER_MTX_CREATED         4
#define INIT_BIT_LISTENER_MTX_CREATED       5

//######################################################################
//# class DataModule
//######################################################################

// non realtime context
DataModule::DataModule( uint32_t class_id,                // class ID
                        uint64_t cmdTaskErrorTime_ns,     // cmdtask error sleep time
                        uint64_t dataTaskErrorTime_ns,    // datatask error sleep time
                        uint64_t dataTaskDisableTime_ns,  // datatask disable sleep time
                        int32_t  cmdMbxMsgSlots,          // command mailbox slots
                        uint32_t cmdMbxMsgDataSize,       // command mailbox data size
                        uint32_t cmdMbxFlags,             // command mailbox create flags
                        uint32_t maxDataBufferEntries,    // max buffer entries
                        uint32_t maxDataBufferListener)   // data buffer listener
                        : Module(class_id,
                                 cmdTaskErrorTime_ns,
                                 dataTaskErrorTime_ns,
                                 dataTaskDisableTime_ns,
                                 cmdMbxMsgSlots,
                                 cmdMbxMsgDataSize,
                                 cmdMbxFlags)
{
    dataBufferMaxEntries    = maxDataBufferEntries;
    dataBufferMaxListener   = maxDataBufferListener;
    dataBufferSendMbx       = 0;

    dataBufferMaxDataSize   = 0;

    dataBufferPeriodTime    = 0;

    listenerNum             = 0;
    globalDataCount         = 0;
    index                   = 0;

    entry                   = NULL;
    listener                = NULL;

    dataModBits.clearAllBits();
}

DataModule::~DataModule()
{
}

//
// private DataModule functions
//

// the time is the first data element afer the message head !
// realtime context
rack_time_t   DataModule::getRecTime(void *p_data)
{
  return *(rack_time_t *)p_data;
}

// realtime context (cmdTask)
int         DataModule::addListener(rack_time_t periodTime, uint32_t destMbxAdr,
                                    MessageInfo* msgInfo)
{
    unsigned int i, idx;
    unsigned int reduction;

    listenerMtx.lock(RACK_INFINITE);

    GDOS_DBG_INFO("Add listener %n (datambx: %x)\n", destMbxAdr, destMbxAdr);

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

    if ((periodTime < dataBufferPeriodTime) ||
        (periodTime == 0))
    {
        reduction = 1; // sending data after every putDataBufferWorkspace()
    }
    else
    {
        reduction = periodTime / dataBufferPeriodTime;
    }

    GDOS_DBG_DETAIL("Setting reduction to %d (self %d ms, request %d ms)\n",
                    reduction, dataBufferPeriodTime, periodTime);

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

        memcpy(&listener[idx].msgInfo, msgInfo, sizeof(MessageInfo));
        listener[idx].msgInfo.src = destMbxAdr;
        listenerNum++;
    }

    listener[idx].reduction = reduction;

    listenerMtx.unlock();
    return 0;
}

// realtime context (cmdTask)
void        DataModule::removeListener(uint32_t destMbxAdr)
{
    uint32_t read = 0;
    uint32_t write = 0;

    GDOS_DBG_INFO("Remove listener %n (datambx: %x)\n", destMbxAdr, destMbxAdr);

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
void        DataModule::removeAllListener(void)
{
    uint32_t i;

    for (i = 0; i < listenerNum; i++)
    {
        GDOS_DBG_INFO("Remove listener %n (datambx: %x)\n", listener[i].msgInfo.src, listener[i].msgInfo.src);
        dataBufferSendMbx->sendMsgReply(MSG_ERROR, &listener[i].msgInfo);
    }

    listenerNum = 0;
}

// realtime context (cmdTask)
int         DataModule::sendDataReply(rack_time_t time, MessageInfo *msgInfo)
{
    uint32_t    n;
    int         ret;
    rack_time_t   difference;
    rack_time_t   minDifference = RACK_TIME_MAX;

    uint32_t    old_index;
    uint32_t    new_index;

    rack_time_t   new_rectime;
    rack_time_t   old_rectime;

    void*       new_data    = NULL;
    void*       old_data    = NULL;

    if (!msgInfo)
        return -EINVAL;

    bufferMtx.lock(RACK_INFINITE);

    if (!globalDataCount) // no data available
    {
        bufferMtx.unlock();
        GDOS_WARNING("DataBuffer: No data in buffer. Try it again \n");
        return -EFAULT;
    }

    new_index   = index;
    new_data    = entry[new_index].p_data;
    new_rectime = getRecTime(new_data);

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

        old_data    = entry[old_index].p_data;
        old_rectime = getRecTime(old_data);

        if (time > ( new_rectime + 2 * dataBufferPeriodTime))
        {
            bufferMtx.unlock();
            GDOS_ERROR("DataBuffer: Requested time %d is newer than newest "
                       "data message %d\n", time, new_rectime);
            return -EINVAL;
        }
        else if (time < old_rectime)
        {
            bufferMtx.unlock();
            GDOS_ERROR("DataBuffer: Requested time %d is older than oldest "
                       "data message %d\n", time, old_rectime);
            return -EINVAL;
        }

        // loop from oldest to newest data package and find package with
        // minimum timedifference to requested time

        while (n)
        {
            old_data    = entry[old_index].p_data;
            old_rectime = getRecTime(old_data);

            difference = abs(old_rectime - time);
            if (difference <= minDifference)
            {
                minDifference = difference;
                new_index     = old_index;
                new_data      = old_data;
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
            bufferMtx.unlock();
            GDOS_ERROR("DataBuffer: Requested time %d is older than oldest "
                       "data message (Buffer is not completely filled)\n", time);
            return -EINVAL;
        }
    }

/*
    GDOS_PRINT("Sending Data: %n -> %n, buffer[%d/%d] @ %p, time %d, size %d \n",
                msgInfo->dest, msgInfo->src, new_index, dataBufferMaxEntries,
                new_data, getRecTime(new_data), entry[new_index].dataSize);
*/

    ret = dataBufferSendMbx->sendDataMsgReply(MSG_DATA, msgInfo, 1, new_data,
                                              entry[new_index].dataSize);

    bufferMtx.unlock();
    return ret;
}

//
// public DataModule functions
//

uint32_t    DataModule::getDataBufferMaxDataSize(void)
{
    return dataBufferMaxDataSize;
}

void        DataModule::setDataBufferMaxDataSize(uint32_t dataSize)
{
    if (dataModBits.testBit(INIT_BIT_BUFFER_CREATED))
    {
        return;
    }
    dataBufferMaxDataSize = dataSize;
}

rack_time_t   DataModule::getDataBufferPeriodTime(uint32_t dataMbx)
{
    uint32_t i;

    if (!dataMbx)
        return dataBufferPeriodTime;

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

void DataModule::setDataBufferPeriodTime(rack_time_t periodTime)
{
    GDOS_PRINT("Setting local period time to %d ms \n", periodTime);
    dataBufferPeriodTime = periodTime;
}

// realtime context (dataTask)
void*       DataModule::getDataBufferWorkSpace(void)
{
    return entry[(index+1) % dataBufferMaxEntries].p_data;
}

// realtime context (dataTask)
void        DataModule::putDataBufferWorkSpace(uint32_t datalength)
{
    uint32_t        i;
    int             ret;

    if (datalength > dataBufferMaxDataSize)
        return;

    bufferMtx.lock(RACK_INFINITE);
    listenerMtx.lock(RACK_INFINITE);

    index = (index + 1) % dataBufferMaxEntries;
    globalDataCount += 1;

    if(globalDataCount == 0)  // handle uint32 overflow
        globalDataCount = 1;

/*
    GDOS_PRINT("Put DataBuffer: buffer[%d/%d] @ %p, time %d, size %d \n",
               index, dataBufferMaxEntries, entry[index].p_data,
               getRecTime(entry[index].p_data), entry[index].dataSize);
*/

    entry[index].dataSize = datalength;

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
                                                      entry[index].p_data,
                                                      entry[index].dataSize);
            if (ret)
            {
                GDOS_ERROR("DataBuffer: Can't send continuous data "
                           "to listener %n, code = %d\n",
                           listener[i].msgInfo.src, ret);

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
int         DataModule::moduleInit(void)
{
    int ret;
    unsigned int i, k;

    // first init module
    ret = Module::moduleInit();
    if (ret)
    {
          return ret;
    }
    dataModBits.setBit(INIT_BIT_RACKMODULE);

    // now the command mailbox exists
    dataBufferSendMbx = getCmdMbx();

    GDOS_DBG_DETAIL("DataModule::moduleInit ... \n");

    // check needed databuffer values
    if (!dataBufferMaxEntries  ||
        !dataBufferMaxDataSize ||
        !dataBufferMaxListener )
    {
        GDOS_ERROR("DataModule: Invalid init values\n");
        goto init_error;
    }

    if (entry || listener)
    {
        GDOS_ERROR("DataModule: DataBuffer exists !\n");
        return -EBUSY;
    }

    // create data buffer structures

    entry = new DataBufferEntry[dataBufferMaxEntries];
    if (!entry)
    {
        GDOS_ERROR("DataModule: DataBuffer entries not created !\n");
        goto init_error;
    }
    dataModBits.setBit(INIT_BIT_ENTRIES_CREATED);
    GDOS_DBG_DETAIL("DataBuffer entry table created @ %p\n", entry);

    for (i=0; i<dataBufferMaxEntries; i++)
    {
        entry[i].p_data = malloc(dataBufferMaxDataSize);
        if (!entry[i].p_data)
        {
            GDOS_ERROR("Error while allocating databuffer entry[%d] \n", i);
            for (k=i-1; i>=0; k--)
            {
                free(entry[k].p_data);
            }
            goto init_error;
        }
        else
        {
            memset(entry[i].p_data, 0, dataBufferMaxDataSize);
            GDOS_DBG_DETAIL("DataBuffer entry @ %p (%d bytes) created\n",
                            entry[i].p_data, dataBufferMaxDataSize);
        }
        entry[i].dataSize = 0;
    }
    dataModBits.setBit(INIT_BIT_BUFFER_CREATED);
    GDOS_DBG_DETAIL("Memory for DataBuffer entries allocated\n");

    // create listener data structures

    listener = new ListenerEntry[dataBufferMaxListener];
    if (!listener)
    {
        GDOS_ERROR("DataModule: listener table not created !\n");
        goto init_error;
    }
    dataModBits.setBit(INIT_BIT_LISTENER_CREATED);
    GDOS_DBG_DETAIL("DataBuffer listener table created @ %p\n", listener);


    ret = bufferMtx.create();
    if (ret) {
        GDOS_ERROR("Error while creating bufferMtx, code = %d \n", ret);
        goto init_error;
    }
    dataModBits.setBit(INIT_BIT_BUFFER_MTX_CREATED);
    GDOS_DBG_DETAIL("DataBuffer buffer mutex created\n");

    ret = listenerMtx.create();
    if (ret) {
        GDOS_ERROR("Error while creating listenerMtx, code = %d \n", ret);
        goto init_error;
    }
    dataModBits.setBit(INIT_BIT_LISTENER_MTX_CREATED);
    GDOS_DBG_DETAIL("DataBuffer listener mutex created\n");

    return 0;

init_error:
    // !!! call local cleanup function !!!
    DataModule::moduleCleanup();

    return ret;
}

// non realtime context (linux)
void        DataModule::moduleCleanup(void)
{
    uint32_t i;

    GDOS_DBG_DETAIL("DataModule::moduleCleanup ... \n");

    if (dataModBits.testAndClearBit(INIT_BIT_LISTENER_MTX_CREATED))
    {
        GDOS_DBG_DETAIL("Deleting dataBuffer listener mutex\n");
        listenerMtx.destroy();
    }

    if (dataModBits.testAndClearBit(INIT_BIT_BUFFER_MTX_CREATED))
    {
        GDOS_DBG_DETAIL("Deleting dataBuffer buffer mutex\n");
        bufferMtx.destroy();
    }

    if (dataModBits.testAndClearBit(INIT_BIT_LISTENER_CREATED))
    {
        GDOS_DBG_DETAIL("Deleting dataBuffer listener table @ %p\n", listener);
        delete[] listener;
        listener = NULL;
    }

    if (dataModBits.testAndClearBit(INIT_BIT_BUFFER_CREATED))
    {
        for (i=0; i<dataBufferMaxEntries; i++)
        {
            GDOS_DBG_DETAIL("Deleting dataBuffer entry @ %p (%d bytes)\n",
                            entry[i].p_data, dataBufferMaxDataSize);
            free(entry[i].p_data);
        }
    }

    if (dataModBits.testAndClearBit(INIT_BIT_ENTRIES_CREATED))
    {
        GDOS_DBG_DETAIL("Deleting dataBuffer entry table @ %p\n", entry);
        delete[] entry;
        entry = NULL;
    }

    // cleanunp module (last command)
    Module::moduleCleanup();
}

// realtime context (dataTask)
int         DataModule::moduleOn(void)
{
    int ret;
    
    if (!dataBufferPeriodTime)
    {
        GDOS_ERROR("Local dataBuffer period time was not set \n");
        GDOS_ERROR("You have to set this value before calling DataModule::moduleOn()\n");
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
        
    return Module::moduleOn();
}

// realtime context (dataTask)
void        DataModule::moduleOff(void)
{
    Module::moduleOff();          // have to be first command

    listenerMtx.lock(RACK_INFINITE);

    removeAllListener();

    listenerMtx.unlock();
}

// realtime context (cmdTask)
int         DataModule::moduleCommand(MessageInfo *msgInfo)
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
                ret = addListener(p_data->periodTime, p_data->dataMbxAdr, msgInfo);
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
                    contData.periodTime = getDataBufferPeriodTime(p_data->dataMbxAdr);
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

        default:
            // not for me -> ask Module
            return Module::moduleCommand(msgInfo);
    }
}
