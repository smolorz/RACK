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
#ifndef __RACK_DATAMODULE_H__
#define __RACK_DATAMODULE_H__

/*!
 * \ingroup module
 *
 * @{*/

#include <main/rack_module.h>

#include <math.h>

//######################################################################
//# class DataBufferEntry
//######################################################################

class DataBufferEntry {
    public:
        void*       p_data;
        uint32_t    dataSize;

        // Konstruktor
        DataBufferEntry()
        {
            p_data      = NULL;
            dataSize    = 0;
        }

        // Destruktor
        ~DataBufferEntry()
        {
            p_data = NULL;
        };
};

//######################################################################
//# class ListenerEntry
//######################################################################

class ListenerEntry {
    public:
        uint32_t        reduction;
        MessageInfo     msgInfo;

        // Konstruktor
        ListenerEntry()
        {
            reduction = 0;
            memset(&msgInfo, 0, sizeof(MessageInfo));
        };

        // Destruktor
        ~ListenerEntry()
        { };
};

//######################################################################
//# class DataModule
//######################################################################

class DataModule : public Module {
    private:
        RackBits            dataModBits;   // rack datamodule init bits
        uint32_t            index;
        uint32_t            globalDataCount;
        uint32_t            listenerNum;

        DataBufferEntry*    entry;
        ListenerEntry*      listener;

        RackMutex           bufferMtx;
        char                bufferMtxName[30];

        RackMutex           listenerMtx;
        char                listenerMtxName[30];

        uint32_t            dataBufferMaxEntries;
        uint32_t            dataBufferMaxDataSize;  // per slot !!!
        uint32_t            dataBufferMaxListener;
        int16_t             dataBufferSendType;
        RackMailbox*        dataBufferSendMbx;
        RACK_TIME           dataBufferPeriodTime;

        friend void         cmd_task_proc(void* arg);

        RACK_TIME           getRecTime(void *p_data);
        int                 addListener(RACK_TIME periodTime, uint32_t destMbxAdr,
                                        MessageInfo* msgInfo);
        void                removeListener(uint32_t destMbxAdr);
        void                removeAllListener(void);
        int                 sendDataReply(RACK_TIME time, MessageInfo *msgInfo);


  public:

    DataModule(uint32_t class_id,               // class ID
               uint64_t cmdTaskErrorTime_ns,    // cmdtask error sleep time
               uint64_t dataTaskErrorTime_ns,   // datatask error sleep time
               uint64_t dataTaskDisableTime_ns, // datatask disable sleep time
               int32_t  cmdMbxMsgSlots,         // command mailbox slots
               uint32_t cmdMbxMsgDataSize,      // command mailbox data size
               uint32_t cmdMbxFlags,            // command mailbox create flags
               uint32_t maxDataBufferEntries,   // max buffer entries
               uint32_t maxDataBufferListener); // data buffer listener

    ~DataModule();

    uint32_t  getDataBufferMaxDataSize(void);
    void      setDataBufferMaxDataSize(uint32_t dataSize);

//    float     getDataBufferSampleRate(void);
//    int       setDataBufferSampleRate(float sampleRate);

    RACK_TIME getDataBufferPeriodTime(uint32_t dataMbx);
    void      setDataBufferPeriodTime(RACK_TIME periodTime);

    void*     getDataBufferWorkSpace(void);
    void      putDataBufferWorkSpace(uint32_t datalength);

    //
    // virtual module functions
    //

    int   moduleInit(void);
    void  moduleCleanup(void);

    int   moduleOn(void);
    void  moduleOff(void);

    int   moduleLoop(void) { return 0; };
    int   moduleCommand(MessageInfo* p_msginfo);
};

/*@}*/

#endif // __RACK_DATAMODULE_H_
