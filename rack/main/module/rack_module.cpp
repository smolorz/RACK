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
#include <sys/mman.h>
#include <typeinfo>
#include <string>

#include <main/rack_module.h>
#include <main/rack_datamodule.h>
#include <main/rack_proxy.h>
#include <main/defines/list_head.h>

//
// Mailbox stuff
//
#define MODULE_MAX_MBX                ( 1 << LOCAL_ID_RANGE )

// Mailbox flags
#define RACKMBX_CREATED               0x0001

// initStatus bits
#define INIT_BIT_CMDMBX_CREATED         0
#define INIT_BIT_GDOSMBX_CREATED        1
#define INIT_BIT_CMDTSK_CREATED         2
#define INIT_BIT_DATATSK_CREATED        3
#define INIT_BIT_CMDTSK_STARTED         4
#define INIT_BIT_DATATSK_STARTED        5

class MbxListHead : public ListHead {

  public:

    RackMailbox  *p_mbx;
    void         *buffer;
    unsigned int flags;


    MbxListHead(RackMailbox *p_mbx, void *buffer) {
      this->p_mbx  = p_mbx;
      this->buffer = buffer;
      flags        = 0;
    }

};

ListHead mbxList;

//
// arguments
//

argTable_t module_argTab[] = {

  {ARGOPT_REQ, "instance" , ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "The global instance number of this driver", { -1 } },

  {ARGOPT_REQ, "cmdTaskPrio", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "priority of the command Task (1-32)", { -1 } },

  {ARGOPT_REQ, "dataTaskPrio", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "priority of the data Task (1-32)", { -1 } },

  {ARGOPT_OPT, "gdosLevel", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "GDOS level (0:print, 1:error, 2:warning, 3:info, 4:detail) [2]", { 2 } },

  {0, "", 0, 0, "", { 0 } }
};

//
// arguments
//

static argTable_t* arg_table;
static char classname[50];

//
// command task
//

// realtime context
void cmd_task_proc(void *arg)
{
    Module*      p_mod     = (Module *)arg;
    GdosMailbox* gdos      = p_mod->gdos;
    MessageInfo  msgInfo;
    char recv_data[p_mod->cmdMbxMsgDataSize];
    int ret;

    if (timer_is_running() != 1)
    {
        GDOS_ERROR("Timer isn't running\n");
        return;
    }

    RackTask::enableRealtimeMode();

    p_mod->cmdTaskRunning = 1;

    GDOS_DBG_INFO("cmdTask: run\n");
    while (p_mod->terminate == 0)
    {
        ret = p_mod->cmdMbx.recvDataMsg(recv_data, p_mod->cmdMbxMsgDataSize, &msgInfo);
        if (ret)
        {
            if (p_mod->terminate == 0)
            {
                GDOS_ERROR("cmdTask: can't receive message on cmd mbx\n");
                RackTask::sleep(p_mod->cmdTaskErrorTime_ns);
            }
            else
            {
                goto cmd_task_exit;
            }
        }
        else
        {
            ret = p_mod->moduleCommand(&msgInfo);
            if (ret && msgInfo.type > 0)
            {
                p_mod->cmdMbx.sendMsgReply(MSG_ERROR, &msgInfo);
            }
        }
    } // while()

cmd_task_exit:
    GDOS_DBG_INFO("cmdTask: terminate\n");
    p_mod->cmdTaskRunning = 0;
}

//
// data task and help functions
//

// realtime context
inline void notify(int8_t type, Module *p_mod)
{
  if (p_mod->replyMsgInfo.dest > 0) {
    p_mod->cmdMbx.sendMsgReply(type, &p_mod->replyMsgInfo);
    clearMsgInfo(&p_mod->replyMsgInfo);
  }
}

// realtime context
void data_task_proc(void *arg)
{
    int ret;
    DataModule*   p_mod = (DataModule *)arg;
    GdosMailbox*  gdos  = p_mod->gdos;

    if (timer_is_running() != 1)
    {
        GDOS_ERROR("Timer isn't running\n");
        return;
    }

    RackTask::enableRealtimeMode();

    p_mod->dataTaskRunning = 1;

    GDOS_DBG_INFO("dataTask: run\n");

    while (p_mod->terminate == 0)
    {
        switch (p_mod->status)
        {
            case MODULE_STATE_ENABLED:

                if (p_mod->targetStatus == MODULE_TSTATE_ON)
                {
                    ret = p_mod->moduleLoop();
                    if (ret)
                    {
                        GDOS_ERROR("Error in moduleLoop()\n");
                        p_mod->status = MODULE_STATE_ERROR;
                        GDOS_PRINT("Turning off ... \n");
                        p_mod->moduleOff();
                        GDOS_PRINT("Module off \n");
                        notify(MSG_ERROR, p_mod);
                    }
                    else
                    {
                        notify(MSG_OK, p_mod);
                    }
                }
                else
                {
                    p_mod->status = MODULE_STATE_DISABLED;
                    GDOS_PRINT("Turning off ... \n");
                    p_mod->moduleOff();
                    GDOS_PRINT("Module off \n");
                }
                break;

            case MODULE_STATE_DISABLED:

                if (p_mod->targetStatus == MODULE_TSTATE_ON)
                {
                    GDOS_PRINT("Turning on ... \n");
                    ret = p_mod->moduleOn();
                    if (ret)
                    {
                        GDOS_ERROR("Can't turn on module\n");
                        p_mod->targetStatus = MODULE_TSTATE_OFF;
                        notify(MSG_ERROR, p_mod);
                    }
                    else    // module is on now
                    {
                        GDOS_PRINT("Module on\n");
                        p_mod->status = MODULE_STATE_ENABLED;

                        // calling loop first time

                        ret = p_mod->moduleLoop();
                        if (ret)
                        {
                            GDOS_ERROR("Error in moduleLoop()\n");
                            p_mod->status = MODULE_STATE_ERROR;
                            GDOS_PRINT("Turning off ... \n");
                            p_mod->moduleOff();
                            GDOS_PRINT("Module off \n");
                            notify(MSG_ERROR, p_mod);
                        }
                        else
                        {
                            notify(MSG_OK, p_mod);
                        }
                    }
                }
                else
                {
                    RackTask::sleep(p_mod->dataTaskDisableTime_ns);
                }
                break;

            case MODULE_STATE_ERROR:

                RackTask::sleep(p_mod->dataTaskErrorTime_ns);

                if (p_mod->targetStatus == MODULE_TSTATE_ON)
                {
                    GDOS_PRINT("Turning on ... \n");
                    ret = p_mod->moduleOn();
                    if (ret)
                    {
                        GDOS_ERROR("Can't turn on module\n");
                        GDOS_PRINT("Turning off ... \n");
                        p_mod->moduleOff();
                        GDOS_PRINT("Module off \n");
                        notify(MSG_ERROR,p_mod);
                    }
                    else
                    {
                        GDOS_PRINT("Module on\n");
                        p_mod->status = MODULE_STATE_ENABLED;
                        notify(MSG_OK,p_mod);
                    }
                }
                else
                {
                    GDOS_PRINT("Module off\n");
                    p_mod->status = MODULE_STATE_DISABLED;
                }
                break;

            default:
                GDOS_ERROR("Unknown module status %d\n", p_mod->status);
                p_mod->status = MODULE_STATE_ERROR;
                GDOS_PRINT("Turning off ... \n");
                p_mod->moduleOff();
                GDOS_PRINT("Module off\n");
        }
    }

    GDOS_DBG_INFO("dataTask: terminate\n");
    p_mod->dataTaskRunning = 0;
}

//######################################################################
//# class Module
//######################################################################

//
// Constructor & Destuctor
//

// non realtime context
Module::Module( uint32_t class_id,
                uint64_t cmdTaskErrorTime_ns,
                uint64_t dataTaskErrorTime_ns,
                uint64_t dataTaskDisableTime_ns,
                int32_t  cmdMbxMsgSlots,
                uint32_t cmdMbxMsgDataSize,
                uint32_t cmdMbxFlags)
{
    // get instance from module_argTab
    inst                      = getIntArg("instance", module_argTab);
    gdosLevel                 = GDOS_MSG_DEBUG_BEGIN -
                                getIntArg("gdosLevel", module_argTab);
    cmdTaskPrio               = getIntArg("cmdTaskPrio", module_argTab);
    dataTaskPrio              = getIntArg("dataTaskPrio", module_argTab);

    classID                   = class_id;
    name                      = RackName::create(classID, inst);

    mailboxBaseAdr            = name;
    mailboxFreeAdr            = name + 1;

    // command mailbox defaults
    this->cmdMbxMsgSlots      = cmdMbxMsgSlots;
    this->cmdMbxMsgDataSize   = cmdMbxMsgDataSize;
    this->cmdMbxFlags         = cmdMbxFlags;

    // init Tasks
    this->cmdTaskErrorTime_ns     = cmdTaskErrorTime_ns;
    this->dataTaskErrorTime_ns    = dataTaskErrorTime_ns;
    this->dataTaskDisableTime_ns  = dataTaskDisableTime_ns;

    modBits.clearAllBits();
    initBits.clearAllBits();

    terminate                 = 0;
    status                    = MODULE_STATE_DISABLED;
    targetStatus              = MODULE_TSTATE_OFF;
    initializing              = 1;

    clearMsgInfo(&replyMsgInfo);

    cmdTaskRunning  = 0;
    dataTaskRunning = 0;
}

Module::~Module()
{
}

//
// Mailbox creation
//

// non realtime context
void      Module::mailboxList(void)
{
  MbxListHead *p_list = (MbxListHead *)mbxList.next;
  printf("**** RACK MAILBOX-LIST @ 0x%p *** \n", &mbxList );
  printf("  Address |   prev     |    this    |    next    \n");
  printf("--------------------------------------------------\n");
  while(p_list != &mbxList) {
    printf(" %8x | 0x%08x | 0x%08x | 0x%08x |\n", p_list->p_mbx->getAdr(),
           (unsigned int)p_list->prev, (unsigned int)p_list,
           (unsigned int)p_list->next );
    p_list = (MbxListHead *)p_list->next;
  }
  printf("--------------------------------------------------\n");
}

uint32_t  Module::mailbox_getNextFreeAdr(void)
{
  return mailboxFreeAdr++;
}

int       Module::mailbox_putLastAdr(void)
{
  return mailboxFreeAdr--;
}

int       Module::mailboxCreate(RackMailbox *p_mbx, uint32_t adr, int slots,
                                size_t data_size, uint32_t flags, int8_t prio)
{
    int ret;
    void         *buffer     = NULL;
    size_t       buffersize  = (RackMailbox::getMsgOverhead() + data_size) * slots;
    MbxListHead  *p_new      = NULL;

    if (!p_mbx)
    {
        GDOS_ERROR("mailboxCreate: Invalid mailbox pointer\n");
        return -EINVAL;
    }

    // allocate buffer
    if (flags & MBX_IN_USERSPACE)
    {
        buffer = malloc(buffersize);
        if (!buffer)
        {
            GDOS_ERROR("mailboxCreate: Can't allocate userspace mailbox buffer\n");
            ret = -ENOMEM;
            goto create_error;
        }
    }

    // add it to list
    p_new = new MbxListHead(p_mbx, buffer);
    if (!p_new)
    {
        GDOS_ERROR("mailboxCreate: Can't allocate mailbox list entry\n");
        ret = -ENOMEM;
        goto create_error;
    }
    p_new->add_tail(&mbxList);

    // create
    ret = p_mbx->create(adr, slots, data_size, buffer, buffersize, prio );
    if (ret)
    {
        GDOS_ERROR("mailboxCreate: Can't create mailbox %x, slots: %d, datasize: %d,"
                    " buffer @ 0x%p, prio: %d, code = %d\n", adr, slots, data_size,
                    buffer, prio, ret);
        goto create_error;
    }
    p_new->flags |= RACKMBX_CREATED;
    if (buffer)
    {
        GDOS_DBG_INFO("MAILBOX: adr: %x, slots: %d, data/msg: %d, size %d, prio: %d, USER\n",
                    adr, slots, data_size, buffersize, prio);
    }
    else
    {
        GDOS_DBG_INFO("MAILBOX: adr: %x, slots: %d, data/msg: %d, size %d, prio: %d, KERNEL\n",
                    adr, slots, data_size, buffersize, prio);
    }
    return 0;

create_error:

    if (p_new && (p_new->flags & RACKMBX_CREATED) )
    {
        GDOS_DBG_INFO("mailboxCreate: mailbox %x -> destroy mailbox \n", adr);
        p_mbx->remove();
    }

    if (p_new)
    {
        // remove entry from list
        GDOS_DBG_INFO("mailboxCreate: mailbox %x -> free mailbox list entry @ 0x%p\n", adr, p_new);
        p_new->del();
        free(p_new);
    }

    if (buffer)
    {
        GDOS_DBG_INFO("mailboxCreate: mailbox %x -> free buffer @ 0x%p\n", adr, buffer);
        free(buffer);
    }

    return ret;
}

int       Module::createMbx(RackMailbox *p_mbx, int slots, size_t data_size,
                            uint32_t flags)
{
    return mailboxCreate(p_mbx, mailbox_getNextFreeAdr(), slots, data_size, flags, getDataTaskPrio() );
}

void      Module::destroyMbx(RackMailbox *p_mbx)
{
    MbxListHead *p_list  = NULL;
    MbxListHead *p_entry = NULL;
    unsigned int address;

    if (!p_mbx)
    {
        GDOS_ERROR("mbxDestroy: Invalid mailbox pointer\n");
        return;
    }

    address = p_mbx->getAdr();

    // get mailbox list entry
    p_list = (MbxListHead *)mbxList.next;
    while (p_list != &mbxList)
    {
        if (p_list->p_mbx == p_mbx)
        {
            p_entry = p_list;
            break;
        }
        p_list = (MbxListHead *)p_list->next;
    }

    if (!p_entry)
        return;

    if (p_entry->flags & RACKMBX_CREATED)
    {
        GDOS_DBG_INFO("mbxDestroy: mailbox %x -> destroy mailbox \n", address );
        p_entry->p_mbx->remove();
    }

    if (p_entry->buffer)
    {
        GDOS_DBG_INFO("mbxDestroy: mailbox %x -> free buffer @ 0x%p\n",
                      address, p_entry->buffer);
        free(p_entry->buffer);
    }

    // remove entry from list
    p_entry->del();

    GDOS_DBG_INFO("mbxDestroy: mailbox %x -> free mailbox list entry @ 0x%p\n",
                  address, p_entry);
    free(p_entry);

    p_mbx = NULL;

    // mailboxList();

}

int       Module::createCmdMbx(void)
{
    return mailboxCreate(&cmdMbx, name, cmdMbxMsgSlots, cmdMbxMsgDataSize,
                         cmdMbxFlags, getDataTaskPrio() );
}

//
// Module init and cleanup
//

// non realtime context (linux)
void      Module::deleteGdosMbx()
{
    // delete gdos mailbox -> messages now on local console
    if (modBits.testAndClearBit(INIT_BIT_GDOSMBX_CREATED))
    {
        GDOS_DBG_DETAIL("Deleting Gdos Mailbox -> next messages on local console\n");
        delete gdos;
        gdos = NULL;
    }
}

// non realtime context
int       Module::moduleInit(void)
{
    int ret;

    argDescriptor_t module_argDesc[] = {
          {module_argTab},
          {arg_table},
          {NULL}
    };

    GDOS_DBG_DETAIL("Module::moduleInit ... \n");

    // disable memory swapping for this program
    mlockall(MCL_CURRENT | MCL_FUTURE);

    // init signal handler
    ret = init_signal_handler(this);
    if (ret)
    {
        goto exit_error;
    }

    // create command mailbox
    ret = createCmdMbx();
    if (ret)
    {
        GDOS_ERROR("Can't create command mailbox\n");
        goto exit_error;
    }
    modBits.setBit(INIT_BIT_CMDMBX_CREATED);
    GDOS_DBG_INFO("Command mailbox created\n");

    // create gdos mailbox --> Messages were transmitted to GUI now
    gdos = new GdosMailbox(&cmdMbx, gdosLevel);
    if (!gdos)
    {
        ret = -ENOMEM;
        GDOS_ERROR("Can't create gdos mailbox, code = %d\n", ret);
        goto exit_error;
    }
    modBits.setBit(INIT_BIT_GDOSMBX_CREATED);
    GDOS_DBG_INFO("Gdos mailbox created\n");

    // init rack time
    ret = rackTime.init();
    if (ret)
    {
        GDOS_ERROR("Can't init rack time, code = %d\n", ret);
        goto exit_error;
    }

    if (rackTime.global)
    {
        GDOS_PRINT("Using global time, offset: %d ms\n",
                   rackTime.offset / 1000000);
    }
    else
    {
        GDOS_PRINT("Using local time\n");
    }


    // print all start arguments
    gdosAllArgs(module_argDesc, gdos);

    // create command task
    ret = cmdTask.create(0, cmdTaskPrio, T_FPU | T_JOINABLE);
    if (ret)
    {
        GDOS_ERROR("Can't init command task, code = %d\n", ret);
        goto exit_error;
    }
    modBits.setBit(INIT_BIT_CMDTSK_CREATED);
    GDOS_DBG_INFO("Command task created \n");

    // create data task
    ret = dataTask.create(0, dataTaskPrio, T_FPU | T_JOINABLE);
    if (ret)
    {
        GDOS_ERROR("Can't init data task, code = %d\n", ret);
        goto exit_error;
    }
    modBits.setBit(INIT_BIT_DATATSK_CREATED);
    GDOS_DBG_INFO("Data task created \n");

    return 0;

exit_error:
    GDOS_ERROR("Error while initializing module. Internal initBits = 0x%x\n",
               modBits.getBits());

    // !!! call local cleanup function !!!
    Module::moduleCleanup();

    return ret;
}

// non realtime context
void      Module::moduleCleanup(void)
{
    GDOS_DBG_DETAIL("Module::moduleCleanup ... \n");

    deleteGdosMbx();

    // delete command mailbox
    if (modBits.testAndClearBit(INIT_BIT_CMDMBX_CREATED))
    {
        destroyMbx(&cmdMbx);
        GDOS_DBG_DETAIL("Command mailbox deleted\n");
    }

    if (modBits.testAndClearBit(INIT_BIT_CMDTSK_STARTED))
    {
        GDOS_DBG_DETAIL("Join - waiting for command task \n");
        cmdTask.join();
    }
    GDOS_DBG_DETAIL("Command task joined\n");

    if (modBits.testAndClearBit(INIT_BIT_DATATSK_STARTED))
    {
        GDOS_DBG_DETAIL("Join data task - waiting\n");
        dataTask.join();
    }
    GDOS_DBG_DETAIL("Data task joined\n");

    // delete data task
    if (modBits.testAndClearBit(INIT_BIT_DATATSK_CREATED))
    {
        dataTask.destroy();
        GDOS_DBG_DETAIL("Data task deleted\n");
    }

    // delete command task
    if (modBits.testAndClearBit(INIT_BIT_CMDTSK_CREATED))
    {
        cmdTask.destroy();
        GDOS_DBG_DETAIL("Command task deleted\n");
    }

    //TODO
    // free all created mutexes

    //TODO
    // free all created tasks

    //TODO
    // free all opened devices

}

// realtime context
int       Module::moduleCommand(MessageInfo *msgInfo)
{
  int ret;

  switch(msgInfo->type) {

    case MSG_ON: {
        switch(targetStatus) {

          case MODULE_TSTATE_ON: {

              if (status == MODULE_STATE_ENABLED) {
                ret = cmdMbx.sendMsgReply(MSG_OK, msgInfo);
                if (ret) {
                  GDOS_ERROR("CmdTask: Can't send ok reply, code = %d\n", ret);
                  return ret;
                }
              } else {
                ret = cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
                if (ret) {
                  GDOS_ERROR("CmdTask: Can't send error reply, code = %d\n", ret);
                  return ret;
                }
              }
              return 0;
          }

          case MODULE_TSTATE_OFF: {

              targetStatus = MODULE_TSTATE_ON;
              memcpy(&replyMsgInfo, msgInfo, sizeof(MessageInfo));
              return 0;
          }

          default: {
              ret = cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
              if (ret) {
                GDOS_ERROR("CmdTask: Can't send error reply, code = %d\n", ret);
              return ret;
            }
          }
        }
        return 0;
    }

    case MSG_OFF: {
        targetStatus = MODULE_TSTATE_OFF;

        ret = cmdMbx.sendMsgReply(MSG_OK, msgInfo);
        if (ret) {
          GDOS_ERROR("CmdTask: Can't send ok reply, code = %d\n", ret);
          return ret;
        }

        if (replyMsgInfo.src > 0) {
          ret = cmdMbx.sendMsgReply(MSG_ERROR, &replyMsgInfo);
          if (ret) {
            GDOS_ERROR("CmdTask: Can't send error reply, code = %d\n", ret);
            return ret;
          }

          clearMsgInfo(&replyMsgInfo);
        }
        return 0;
    }

    case MSG_GET_STATUS: {
        ret = cmdMbx.sendMsgReply(status, msgInfo);
        if (ret) {
          GDOS_ERROR("CmdTask: Can't send status, code = %d\n", ret);
          return ret;
        }

        return 0;
    }

    case MSG_SET_LOG_LEVEL: {
//TODO
        return -0;
    }

    case MSG_GET_PERIOD_TIME: {
//TODO
        return -0;
    }


    default: {
      // nobody handles this command -> return MODULE_ERROR
      return -EINVAL;
    }
  }
}

void      Module::run(void)
{
    int ret;

    // start command task
    ret = cmdTask.start(&cmd_task_proc, this);
    if (ret)
    {
        GDOS_ERROR("Can't start command task, code = %d\n", ret);
        goto exit_error;
    }

    // check if command task is running
    if (!cmdTaskRunning)
    {
        GDOS_ERROR("Error while starting command task\n");
        ret = -EINVAL;
        goto exit_error;
    }
    modBits.setBit(INIT_BIT_CMDTSK_STARTED);
    GDOS_DBG_INFO("Command task started \n");


    // start data task
    ret = dataTask.start(&data_task_proc, this);
    if (ret)
    {
        GDOS_ERROR("Can't start data task, code = %d\n", ret);
        goto exit_error;
    }

    // check if data task is running
    if (!dataTaskRunning) {
        GDOS_ERROR("Error while starting data task\n");
        ret = -EINVAL;
        goto exit_error;
    }
    modBits.setBit(INIT_BIT_DATATSK_STARTED);
    GDOS_DBG_INFO("Data task started \n");

    pause();

exit_error:
    // call top level cleanup function
    moduleCleanup();
    return;
}

//
// save argument table and name of module
//

void save_argTab(argTable_t* p_tab, const char* name)
{
    arg_table = p_tab;
    strncpy(classname, name, 50);
}

//
// signal handler functions
//

// !!!!! WARNING !!!!!
// p_signal_module is only valid, if the derived classes have only ONE
// instance. Else the signal handler calls the wrong function.
// !!!!! WARNING !!!!!

static Module *p_signal_module = NULL;
static int signal_flags = 0;

#define DISABLE_SIGXCPU         0x0001
#define HANDLING_TERM           0x0002

void signal_handler(int sig)
{
    int nentries;
    void *array[10];
    char **strings;
    int i;

    if (!p_signal_module)
    {
        printf("SIGNAL_HANDLER: can't calling cleanup function. "
               "Pointer invalid \n");
        return;
    }

    switch(sig)
    {
#ifndef PRINTF_DEBUG
        case SIGXCPU:

            if (signal_flags & HANDLING_TERM) // ignore while cleaning up
                return;

            printf("\n");
            printf("---=== SIGNAL HANDLER of %s ===---\n", classname);
            printf(" -> SIGXCPU (%02d)\n", sig);
            printf(" !!! WARNING unexpected switch to secondary mode !!!\n");
            nentries = backtrace (array, 10);
            strings = backtrace_symbols (array, nentries);
            for (i = 0; i < nentries; i++)
            {
                printf ("  %s\n", strings[i]);
            }
            free (strings);
            printf("---=== END OF %s SIGNAL HANDLER ===---\n", classname);
            return;
#endif

        case SIGSEGV:
            if (signal_flags & HANDLING_TERM) // ignore while cleaning up
                return;

            signal_flags |= HANDLING_TERM;

            printf("---=== SIGNAL HANDLER of %s ===---\n", classname);
            printf(" -> SIGSEGV (%02d)\n", sig);
            printf(" -> Segmentation fault \n");
            nentries = backtrace (array, 10);
            strings = backtrace_symbols (array, nentries);
            for (i = 0; i < nentries; i++)
            {
                printf ("  %s\n", strings[i]);
            }
            free (strings);

            // shutdown module
            p_signal_module->moduleShutdown();

            printf("---=== END OF %s SIGNAL HANDLER ===---\n", classname);
            return;

        case SIGTERM:
        case SIGINT:
            if (signal_flags & HANDLING_TERM) // ignore while cleaning up
                return;

            signal_flags |= HANDLING_TERM;

            printf("---=== SIGNAL HANDLER of %s ===---\n", classname);
            printf(" -> SIGTERM, SIGINT (%02d)\n", sig);
            printf(" -> calling cleanup function of Module class %p\n",
                   p_signal_module);

            // shutdown module
            p_signal_module->moduleShutdown();

            printf("---=== END OF %s SIGNAL HANDLER ===---\n", classname);
            return;

        default:
            printf("------------ SIGNAL_HANDLER START -----------------\n");
            printf(" -> (%02d)\n", sig);
            printf(" -> NOT HANDLED\n");
            printf("------------ SIGNAL_HANDLER STOP ------------------\n");
            return;
    }
}

int init_signal_handler(Module *p_mod)
{
    if (!p_mod)
    {
        printf("SIGNAL_HANDLER: invalid module class pointer\n");
        return -EINVAL;
    }

    if (p_signal_module)
    {
        printf("SIGNAL_HANDLER: there is already a pointer to a class (%p)\n",
               p_signal_module);
        return -EBUSY;
    }

    p_signal_module = p_mod;

    printf("SIGNAL_HANDLER: get module class pointer %p \n", p_signal_module);

    signal(SIGTERM, signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGXCPU, signal_handler);
    signal(SIGSEGV, signal_handler);

    return 0;
}

