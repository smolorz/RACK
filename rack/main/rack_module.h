/**
 * @file
 * RACK - Robotics Application Construction Kit
 *
 * @note Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * @n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#ifndef __RACK_MODULE_H__
#define __RACK_MODULE_H__

#include <signal.h>
#include <execinfo.h>

#include <main/rack_debug.h>
#include <main/rack_proxy.h>
#include <main/rack_mailbox.h>
#include <main/rack_mutex.h>
#include <main/rack_name.h>
#include <main/rack_task.h>
#include <main/rack_time.h>

#include <main/argopts.h>

using std::string;

/* The command task has been received the ON-Command.
 * Data task shall enable the Module. */
#define MODULE_TSTATE_ON                MSG_ON

/* The command task has been received the OFF-Command.
 *  Data task shall disable the Module. */
#define MODULE_TSTATE_OFF               MSG_OFF

/* Module is enabled - Data task is working */
#define MODULE_STATE_ENABLED            MSG_ENABLED

/* Module is disabled - Command task is waiting for ON-Command */
#define MODULE_STATE_DISABLED           MSG_DISABLED

/* Module is in error state */
#define MODULE_STATE_ERROR              MSG_ERROR

//
// save argument table and name of module
//
void save_argTab(argTable_t* p_tab, const char* name);

extern argTable_t module_argTab[];

//
// some additional functions
//

/*!
 * \ingroup rackservices
 * \defgroup module Rack Module
 *
 * RackModule services.
 *
 * The class RackModule is the higest class of RACK components. All
 * public or protected Module functions can be used by any component which
 * inherits data structures or functions of this class.
 *
 * The public or protected data structures and functions are described in this
 * section.
 *
 * @{*/

// module mailbox create flags
#define MBX_IN_KERNELSPACE              0x0001
#define MBX_IN_USERSPACE                0x0002
#define MBX_SLOT                        0x0004
#define MBX_FIFO                        0x0008

//######################################################################
//# class RackModule
//######################################################################

class RackModule {

//
// init bits
//
    protected:
        RackBits  modBits;      // internal rack module init bits
    public:
        /** Bits to save the state of the initialisation */
        RackBits  initBits;     // high level driver init bits

//
// command task
//
    protected:
        RackTask  cmdTask;
        int8_t    cmdTaskRunning;
        int8_t    cmdTaskPrio;
        char      cmdTaskName[50];
        uint64_t  cmdTaskErrorTime_ns;

        int       cmdTaskJoin();

    public:
        /** Get the priority of the command task */
        int8_t    getCmdTaskPrio(void)
        {
            return cmdTaskPrio;
        }

    friend void   cmd_task_proc(void *arg);

//
// data task
//
    protected:
        RackTask  dataTask;
        int8_t    dataTaskRunning;
        int8_t    dataTaskPrio;
        char      dataTaskName[50];
        uint64_t  dataTaskErrorTime_ns;
        uint64_t  dataTaskDisableTime_ns;

        int       dataTaskJoin();

    public:
        /** Get the priority of the data task */
        int8_t    getDataTaskPrio(void)
        {
            return dataTaskPrio;
        }

    friend void   data_task_proc(void *arg);
    friend void   notify(int8_t type, RackModule *p_mod);

//
// common task values
//
    protected:
        int terminate;      // to stop the tasks
        int targetStatus;   // next module state
        int initializing;   // =1 if this module is still loading

        /** Message info to reply a message */
        message_info  replyMsgInfo;

        /** Module state */
        int status;

        /** Get the terminate value.
         * If this value is not zero all tasks will terminate */
        int term(void)
        {
            return terminate;
        };

//
// mailboxes
//
    protected:
        uint32_t  mailboxBaseAdr;
        uint32_t  mailboxFreeAdr;

        int32_t   cmdMbxMsgSlots;
        uint32_t  cmdMbxMsgDataSize;
        uint32_t  cmdMbxFlags;

        void      mailboxList(void);
        uint32_t  mailbox_getNextFreeAdr(void);
        int       mailbox_putLastAdr(void);
        int       mailboxCreateCmdMbx(void);
        int       mailboxCreate(RackMailbox *p_mbx, uint32_t adr, int slots,
                                size_t data_size, uint32_t flags, int8_t prio);

        /** Command mailbox of the module */
        RackMailbox   cmdMbx;

        int           createMbx(RackMailbox *p_mbx, int slots, size_t data_size,
                                uint32_t flags);
        void          destroyMbx(RackMailbox *p_mbx);
        int           initCmdMbx(int slots, size_t data_size, uint32_t flags);
        int           createCmdMbx(void);

        /** Get the pointer to the command mailbox */
        RackMailbox*  getCmdMbx(void)
        {
            return &cmdMbx;
        };

//
// Debugging
//
    protected:
        /** Debugging mailbox */
        GdosMailbox*  gdos;

        /** Debugging level */
        char          gdosLevel;

    public:
        /** Get the pointer of the debugging mailbox */
        GdosMailbox* getGdosMbx(void)
        {
            return gdos;
        }

        /** Set the debugging level */
        void setGdosLevel(int8_t newLevel)
        {
            if (gdos) // mailbox exists
                gdos->setGdosLevel(newLevel);
            else
                gdosLevel = newLevel;
        }

        /** Delete the debugging mailbox */
        void deleteGdosMbx();

//
// Rack time
//
    public:
        /** RackTime instance of the module */
        RackTime rackTime;

//
// module values
//
    protected:
        uint32_t  inst;      // instance number
        uint32_t  classID;   // class-id (LADAR)
        uint32_t  name;      // module name (12345678) == cmdMbxAdr

    public:
        /** Get instance number of the module */
        uint32_t getInstNo(void)
        {
            return inst;
        }

        /** Get class id of the module */
        uint32_t getClassId(void)
        {
            return classID;
        }

        /** Get the name of the module */
        uint32_t getName(void)
        {
            return name;
        }

        RackModule(uint32_t class_id,
                   uint64_t cmdTaskErrorTime_ns,
                   uint64_t dataTaskErrorTime_ns,
                   uint64_t dataTaskDisableTime_ns,
                   int32_t  cmdMbxMsgSlots,         // command mailbox slots
                   uint32_t cmdMbxMsgDataSize,      // command mailbox data size
                   uint32_t cmdMbxFlags);           // command mailbox create flags

        virtual ~RackModule();

//
// virtual module functions
//
    public:
        /** The init function of the module */
        virtual int   moduleInit(void);

        /** The cleanup function of the module */
        virtual void  moduleCleanup(void);

    protected:
        /** The on function of the module */
        virtual int   moduleOn(void)
        {
            return 0;
        }

        /** The off function of the module */
        virtual void  moduleOff(void)
        {
        }

        /** The loop function of the module */
        virtual int   moduleLoop(void)
        {
            return 0;
        }

        /** The moduleCommand function of the module */
        virtual int   moduleCommand(message_info* p_msginfo);

//
// signal handler shutdown function
//
    public:
        void moduleShutdown()
        {
            terminate = 1;

            if (status == MODULE_STATE_ENABLED)
                moduleOff();

            moduleCleanup();
        }

//
// module start arguments
//
    public:

        static int getArgs(int argc, char *argv[], argTable_t* p_tab,
                           const char *classname)
        {
            argDescriptor_t module_argDesc[] =
            {
                { module_argTab}, {p_tab}, {NULL}
            };

            save_argTab(p_tab, classname);
            return argScan(argc, argv, module_argDesc, classname);
        }

//
// module run function
//
    public:
        void run(void);

}; // class RackModule

/*@}*/

//
// signal handler functions
//

void signal_handler(int sig);
int init_signal_handler(RackModule *p_mod);

#endif // __RACK_MODULE_H__
