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
 * @ingroup rack
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

/*!
 * @anchor RACK_MODULE_TSTATE   @name Rack module target state
 * Several target modes of the Rack module. The command task will set these
 * Flags.
 * @{ */

/** The command task has been received the ON-Command.
 *  Data task shall enable the Module. */
#define MODULE_TSTATE_ON                MSG_ON

/** The command task has been received the OFF-Command.
 *  Data task shall disable the Module. */
#define MODULE_TSTATE_OFF               MSG_OFF

/** @} */

/*!
 * @anchor RACK_MODULE_STATE   @name Rack module state
 * Several modes of the Rack module. The data task will set these
 * Flags after receiving the target status flags set by the command task.
 * @{ */

/** Module is enabled - Data task is working */
#define MODULE_STATE_ENABLED            MSG_ENABLED

/** Module is disabled - Command task is waiting for ON-Command */
#define MODULE_STATE_DISABLED           MSG_DISABLED

/** Module is in error state */
#define MODULE_STATE_ERROR              MSG_ERROR

/** @} */

// module mailbox create flags
#define MBX_IN_KERNELSPACE              0x0001
#define MBX_IN_USERSPACE                0x0002
#define MBX_SLOT                        0x0004
#define MBX_FIFO                        0x0008

//
// some additional functions
//

// non realtime / realtime context
static inline int timer_is_running(void)
{
  int ret;

  if (!in_rt_context()) {
    return -1;
  }
  ret = RackTask::sleep(1);
  if (ret == -EWOULDBLOCK)
    return 0;
  return 1;
}

//
// save argument table and name of module
//
void save_argTab(argTable_t* p_tab, const char* name);


extern argTable_t module_argTab[];

//######################################################################
//# class Module
//######################################################################

class Module {

//
// init bits
//

  private:
    RackBits  modBits;      // internal rack module init bits
  public:
    RackBits  initBits;     // high level driver init bits

//
// command task
//

  private:
    RackTask  cmdTask;
    int8_t    cmdTaskRunning;
    int8_t    cmdTaskPrio;
    char      cmdTaskName[50];
    uint64_t  cmdTaskErrorTime_ns;

  public:
    int8_t    getCmdTaskPrio(void) { return cmdTaskPrio; }
    int       cmdTaskJoin();

  friend void cmd_task_proc(void *arg);

//
// data task
//

  private:
    RackTask  dataTask;
    int8_t    dataTaskRunning;
    int8_t    dataTaskPrio;
    char      dataTaskName[50];
    uint64_t  dataTaskErrorTime_ns;
    uint64_t  dataTaskDisableTime_ns;

  public:
    int8_t    getDataTaskPrio(void) { return dataTaskPrio; }
    int       dataTaskJoin();

  friend void data_task_proc(void *arg);
  friend void notify(int8_t type, Module *p_mod);

//
// common task values
//
  private:
    int          terminate;      // to stop the tasks
    int          targetStatus;   // next module state -> see target status defines
    int          initializing;   // =1 if this module is still loading

  protected:
    MessageInfo  replyMsgInfo;
    int          status;      // module state -> see module status defines

    int          term(void) { return terminate; }

//
// mailboxes
//

  private:
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

  protected:
    RackMailbox   cmdMbx;

    int           createMbx(RackMailbox *p_mbx, int slots, size_t data_size,
                            uint32_t flags);
    void          destroyMbx(RackMailbox *p_mbx);
    int           initCmdMbx(int slots, size_t data_size, uint32_t flags);
    int           createCmdMbx(void);
    RackMailbox*  getCmdMbx(void) { return &cmdMbx; };

//
// Debugging
//
    GdosMailbox* gdos;
    char         gdosLevel;

    // debug mailbox functions
  public:

    GdosMailbox* getGdosMbx(void) { return gdos; }

    void setGdosLevel(int8_t newLevel)
    {
        if (gdos) // mailbox exists
        {
            gdos->setGdosLevel(newLevel);
        }
        else
        {
            gdosLevel = newLevel;
        }
    }

    void          deleteGdosMbx();

//
// Rack time
//
  public:
    RackTime rackTime;

//
// module values
//

  private:
    uint32_t    inst;      // instance number
    uint32_t    classID;   // class-id (LADAR)
    uint32_t    name;      // module name (12345678) == cmdMbxAdr

  public:
    uint32_t    getInstNo(void)     { return inst; }
    uint32_t    getClassId(void)    { return classID; }
    const char* getNameString(void) { return RackName::classString(name); }
    uint32_t    getName(void)       { return name; }

    Module(uint32_t class_id,
           uint64_t cmdTaskErrorTime_ns,
           uint64_t dataTaskErrorTime_ns,
           uint64_t dataTaskDisableTime_ns,
           int32_t  cmdMbxMsgSlots,         // command mailbox slots
           uint32_t cmdMbxMsgDataSize,      // command mailbox data size
           uint32_t cmdMbxFlags);           // command mailbox create flags


    virtual ~Module();

//
// virtual module functions
//
  public:
    virtual int   moduleInit(void);
    virtual void  moduleCleanup(void);

  protected:
    virtual int   moduleOn(void)    { return 0; };
    virtual void  moduleOff(void)   { };

    virtual int   moduleLoop(void)  { return 0; };
    virtual int   moduleCommand(MessageInfo* p_msginfo);

//
// signal handler shutdown function
//
  public:
    void             moduleShutdown()
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
                       const char *classname) {
      argDescriptor_t module_argDesc[] = {
          {module_argTab}, {p_tab}, {NULL}
      };
      save_argTab(p_tab, classname);
      return argScan(argc, argv, module_argDesc, classname);
    }

//
// module run function
//
  public:
    void run(void);

}; // class Module

//
// signal handler functions
//

void signal_handler(int sig);
int init_signal_handler(Module *p_mod);

#endif // __RACK_MODULE_H__
