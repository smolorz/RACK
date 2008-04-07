/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2007 University of Hannover
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

#include <sys/mman.h>
#include <typeinfo>
#include <string>
#include <signal.h>
#ifdef __XENO__
#include <execinfo.h>
#endif

#include <main/rack_module.h>
#include <main/rack_proxy.h>
#include <main/defines/list_head.h>

#define CMD_TASK_TIMEOUT 500000000llu

//
// Mailbox stuff
//
#define MODULE_MAX_MBX                  ( 1 << LOCAL_ID_RANGE )

// Mailbox flags
#define RACKMBX_CREATED                 0x0001

// initStatus bits
#define INIT_BIT_CMDMBX_CREATED         0
#define INIT_BIT_GDOS_CREATED           1
#define INIT_BIT_CMDTSK_CREATED         2
#define INIT_BIT_DATATSK_CREATED        3
#define INIT_BIT_CMDTSK_STARTED         4
#define INIT_BIT_DATATSK_STARTED        5
#define INIT_BIT_MALLOC_PARAM_MSG       6

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

  {ARGOPT_OPT, "instance" , ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "The global instance number of this driver", { 0 } },

  {ARGOPT_OPT, "gdosLevel", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "GDOS level (0:print, 1:error, 2:warning, 3:info, 4:detail) [2]", { 2 } },

  {ARGOPT_OPT, "targetStatus", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "target status (0:off, 1:on) [0])", { 0 } },

  {ARGOPT_OPT, "cmdTaskPrio", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "priority of the command Task (1-32)", { 1 } },

  {ARGOPT_OPT, "dataTaskPrio", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "priority of the data Task (1-32)", { 1 } },

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
    RackModule*  p_mod     = (RackModule *)arg;
    GdosMailbox* gdos      = p_mod->gdos;
    message_info  msgInfo;
    char recv_data[p_mod->cmdMbxMsgDataSize];
    int ret;

    RackTask::enableRealtimeMode();

    GDOS_DBG_INFO("CmdTask: Started\n");

    while (p_mod->terminate == 0)
    {
        ret = p_mod->cmdMbx.recvDataMsgTimed(CMD_TASK_TIMEOUT, recv_data, p_mod->cmdMbxMsgDataSize, &msgInfo);
        if (ret)
        {
            if((ret != -EWOULDBLOCK) && (ret != -ETIMEDOUT))
            {
                if(p_mod->terminate == 0)
                {
                    GDOS_ERROR("CmdTask: Can't receive message on cmdMbx (code %i)\n", ret);
                    p_mod->terminate = 1;
                }
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

    GDOS_DBG_INFO("CmdTask: Terminated\n");
}

//
// data task and help functions
//

// realtime context
inline void notify(int8_t type, RackModule *p_mod)
{
    if (p_mod->replyMsgInfo.dest > 0)
    {
        p_mod->cmdMbx.sendMsgReply(type, &p_mod->replyMsgInfo);
        clearMsgInfo(&p_mod->replyMsgInfo);
    }
}

// realtime context
void data_task_proc(void *arg)
{
    int ret;
    RackModule*     p_mod = (RackModule*)arg;
    GdosMailbox*    gdos  = p_mod->gdos;

    RackTask::enableRealtimeMode();

    GDOS_DBG_INFO("DataTask: Started\n");

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
                        if(p_mod->terminate)
                        {
                            break;
                        }

                        if(p_mod->targetStatus == MODULE_TSTATE_ON)
                        {
                            GDOS_PRINT("Error\n");
                            p_mod->moduleOff();
                            p_mod->status = MODULE_STATE_ERROR;
                        }
                        else
                        {
                            GDOS_DBG_INFO("Turning off module ...\n");
                            p_mod->moduleOff();
                            GDOS_PRINT("Module off\n");
                            p_mod->status = MODULE_STATE_DISABLED;
                        }
                        notify(MSG_ERROR, p_mod);
                    }
                    else
                    {
                        notify(MSG_OK, p_mod);
                    }
                }
                else
                {
                    GDOS_DBG_INFO("Turning off module ...\n");
                    p_mod->moduleOff();
                    GDOS_PRINT("Module off\n");
                    p_mod->status = MODULE_STATE_DISABLED;
                }
                break;

            case MODULE_STATE_DISABLED:

                if (p_mod->targetStatus == MODULE_TSTATE_ON)
                {
                    GDOS_DBG_INFO("Turning on module ...\n");
                    ret = p_mod->moduleOn();
                    if (ret)
                    {
                        if(!p_mod->terminate)
                        {
                            GDOS_PRINT("Error, can't turn on module\n");
                        }
                        p_mod->moduleOff();
                        p_mod->status = MODULE_STATE_ERROR;
                        notify(MSG_ERROR, p_mod);
                    }
                    else    // module is on now
                    {
                        GDOS_PRINT("Module on\n");
                        p_mod->status = MODULE_STATE_ENABLED;
                        notify(MSG_OK, p_mod);
                    }
                }
                else
                {
                    RackTask::sleep(100000000llu);  // 100ms
                }
                break;

            case MODULE_STATE_ERROR:

                // wait for a random time between 2s and 4s
                p_mod->dataTaskErrorTimer = (int)(20 * (1.0 + (double)rand() / (double)RAND_MAX));

                p_mod->status = MODULE_STATE_ERROR_WAIT;

                break;

            case MODULE_STATE_ERROR_WAIT:

                if (p_mod->targetStatus == MODULE_TSTATE_ON)
                {
                    RackTask::sleep(100000000llu);  // 100ms

                    p_mod->dataTaskErrorTimer--;

                    if(p_mod->dataTaskErrorTimer <= 0)
                    {
                        p_mod->status = MODULE_STATE_ERROR_RETRY;
                    }
                }
                else
                {
                    GDOS_DBG_INFO("Turning off module ...\n");
                    p_mod->moduleOff();
                    GDOS_PRINT("Module off\n");
                    p_mod->status = MODULE_STATE_DISABLED;
                    notify(MSG_OK, p_mod);
                }
                break;

            case MODULE_STATE_ERROR_RETRY:

                GDOS_DBG_INFO("Turning on module ...\n");
                ret = p_mod->moduleOn();
                if (ret)
                {
                    if(!p_mod->terminate)
                    {
                        GDOS_PRINT("Error, can't turn on module\n");
                    }
                    p_mod->moduleOff();
                    p_mod->status = MODULE_STATE_ERROR;
                }
                else
                {
                    GDOS_PRINT("Module on\n");
                    p_mod->status = MODULE_STATE_ENABLED;
                }
                break;

            default:
                GDOS_ERROR("Unknown module status %d\n", p_mod->status);
                p_mod->moduleOff();
                p_mod->targetStatus = MODULE_STATE_DISABLED;
                p_mod->status = MODULE_STATE_DISABLED;
        }
    } // while()

    if (p_mod->status == MODULE_STATE_ENABLED)
    {
        GDOS_DBG_INFO("Turning off module ...\n");
        p_mod->moduleOff();
        GDOS_PRINT("Module off\n");
        p_mod->status = MODULE_STATE_DISABLED;
    }

    GDOS_DBG_INFO("DataTask: Terminated\n");
}

 /*!
 * @ingroup module
 *
 *@{*/

//######################################################################
//# class RackModule
//######################################################################

/*
 * @brief RackModule constructor
 *
 * This function creates a RackModule with all needed mailboxes and tasks.
 *
 * @param class_id
 * @param cmdTaskErrorTime_ns
 * @param cmdMbxMsgSlots
 * @param cmdMbxMsgDataSize
 * @param cmdMbxFlags
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT)
 *
 * Rescheduling: never.
 */
RackModule::RackModule( uint32_t classId,
                uint64_t dataTaskErrorTime_ns,
                int32_t  cmdMbxMsgSlots,
                uint32_t cmdMbxMsgDataSize,
                uint32_t cmdMbxFlags)
{
    // get instance from module_argTab
    inst                      = getIntArg("instance", module_argTab);
    gdosLevel                 = GDOS_MSG_DEBUG_BEGIN - getIntArg("gdosLevel", module_argTab);
    cmdTaskPrio               = getIntArg("cmdTaskPrio", module_argTab);
    dataTaskPrio              = getIntArg("dataTaskPrio", module_argTab);

    name                      = RackName::create(classId, inst);

    mailboxBaseAdr            = name;
    mailboxFreeAdr            = name + 1;

    this->cmdMbxMsgSlots      = cmdMbxMsgSlots;
    this->cmdMbxFlags         = cmdMbxFlags;
    if(cmdMbxMsgDataSize < 240)
    {
        // minimum size for rack_param_msg
        this->cmdMbxMsgDataSize   = 240;
    }
    else
    {
        this->cmdMbxMsgDataSize   = cmdMbxMsgDataSize;
    }

    moduleInitBits.clearAllBits();
    initBits.clearAllBits();

    terminate                 = 0;
    status                    = MODULE_STATE_DISABLED;
    initializing              = 1;

    if(getIntArg("targetStatus", module_argTab) > 0)
    {
        targetStatus = MODULE_TSTATE_ON;
    }
    else
    {
        targetStatus = MODULE_TSTATE_OFF;
    }

    clearMsgInfo(&replyMsgInfo);
}

RackModule::~RackModule()
{
}

//
// Mailbox creation
//

// non realtime context
void      RackModule::mailboxList(void)
{
  MbxListHead *p_list = (MbxListHead *)mbxList.next;
  printf("**** RACK MAILBOX-LIST @ 0x%p *** \n", &mbxList );
  printf("  Address |   prev     |    this    |    next    \n");
  printf("--------------------------------------------------\n");
  while(p_list != &mbxList) {
    printf(" %8x | 0x%08x | 0x%08x | 0x%08x |\n", (unsigned int)p_list->p_mbx->getAdr(),
           (unsigned int)p_list->prev, (unsigned int)p_list,
           (unsigned int)p_list->next );
    p_list = (MbxListHead *)p_list->next;
  }
  printf("--------------------------------------------------\n");
}

uint32_t  RackModule::mailbox_getNextFreeAdr(void)
{
  return mailboxFreeAdr++;
}

int       RackModule::mailbox_putLastAdr(void)
{
  return mailboxFreeAdr--;
}

int       RackModule::mailboxCreate(RackMailbox *p_mbx, uint32_t adr, int slots,
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

int       RackModule::createMbx(RackMailbox *p_mbx, int slots, size_t data_size,
                            uint32_t flags)
{
    return mailboxCreate(p_mbx, mailbox_getNextFreeAdr(), slots, data_size, flags, dataTaskPrio);
}

void      RackModule::destroyMbx(RackMailbox *p_mbx)
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

int       RackModule::createCmdMbx(void)
{
    return mailboxCreate(&cmdMbx, name, cmdMbxMsgSlots, cmdMbxMsgDataSize,
                         cmdMbxFlags, dataTaskPrio);
}

//
// RackModule init and cleanup
//

// non realtime context (linux)
void      RackModule::deleteGdosMbx()
{
    // delete gdos mailbox -> messages now on local console
    if (moduleInitBits.testAndClearBit(INIT_BIT_GDOS_CREATED))
    {
        GDOS_DBG_INFO("Deleting GDOS -> next message on local console\n");
        delete gdos;
        gdos = NULL;
    }
}

int     RackModule::parseArgTable(argTable_t *argTable, rack_param_msg *paramMsg)
{
    int i = 0;
    while(argTable[i].name.length() != 0)
    {
        if(paramMsg != NULL)
        {
            strncpy(paramMsg->parameter[paramMsg->parameterNum].name, argTable[i].name.c_str(), RACK_PARAM_MAX_STRING_LEN);

            switch(argTable[i].val_type)
            {
            case ARGOPT_VAL_STR:
                paramMsg->parameter[paramMsg->parameterNum].type = RACK_PARAM_STRING;
                strncpy(paramMsg->parameter[paramMsg->parameterNum].valueString, argTable[i].val.s, RACK_PARAM_MAX_STRING_LEN);
                break;
            case ARGOPT_VAL_FLT:
                paramMsg->parameter[paramMsg->parameterNum].type = RACK_PARAM_FLOAT;
                paramMsg->parameter[paramMsg->parameterNum].valueFloat = argTable[i].val.f;
                break;
            default: // ARGOPT_VAL_INT:
                paramMsg->parameter[paramMsg->parameterNum].type = RACK_PARAM_INT32;
                paramMsg->parameter[paramMsg->parameterNum].valueInt32 = argTable[i].val.i;
            }
            paramMsg->parameterNum++;
        }
        i++;
    }
    return i;
}

int32_t   RackModule::getInt32Param(char* paramName)
{
    for(int i = 0; i < paramMsg->parameterNum; i++)
    {
        if(strncmp(paramMsg->parameter[i].name, paramName, RACK_PARAM_MAX_STRING_LEN) == 0)
        {
            return paramMsg->parameter[i].valueInt32;
        }
    }
    return 0;
}

char*   RackModule::getStringParam(char* paramName)
{
    for(int i = 0; i < paramMsg->parameterNum; i++)
    {
        if(strncmp(paramMsg->parameter[i].name, paramName, RACK_PARAM_MAX_STRING_LEN) == 0)
        {
            return paramMsg->parameter[i].valueString;
        }
    }
    return 0;
}

float   RackModule::getFloatParam(char* paramName)
{
    for(int i = 0; i < paramMsg->parameterNum; i++)
    {
        if(strncmp(paramMsg->parameter[i].name, paramName, RACK_PARAM_MAX_STRING_LEN) == 0)
        {
            return paramMsg->parameter[i].valueFloat;
        }
    }
    return 0;
}

void   RackModule::setInt32Param(char* paramName, int32_t value)
{
    for(int i = 0; i < paramMsg->parameterNum; i++)
    {
        if(strncmp(paramMsg->parameter[i].name, paramName, RACK_PARAM_MAX_STRING_LEN) == 0)
        {
            paramMsg->parameter[i].valueInt32 = value;
            return;
        }
    }
}

void   RackModule::setStringParam(char* paramName, char* value)
{
    for(int i = 0; i < paramMsg->parameterNum; i++)
    {
        if(strncmp(paramMsg->parameter[i].name, paramName, RACK_PARAM_MAX_STRING_LEN) == 0)
        {
            strncpy(paramMsg->parameter[i].valueString, value, RACK_PARAM_MAX_STRING_LEN);
            return;
        }
    }
}

void   RackModule::setFloatParam(char* paramName, float value)
{
    for(int i = 0; i < paramMsg->parameterNum; i++)
    {
        if(strncmp(paramMsg->parameter[i].name, paramName, RACK_PARAM_MAX_STRING_LEN) == 0)
        {
            paramMsg->parameter[i].valueFloat = value;
            return;
        }
    }
}

// non realtime context
int       RackModule::moduleInit(void)
{
    int64_t offset;
    int ret;

#ifdef __XENO__
    // disable memory swapping for this program
    mlockall(MCL_CURRENT | MCL_FUTURE);
#endif

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
    moduleInitBits.setBit(INIT_BIT_CMDMBX_CREATED);
    GDOS_DBG_INFO("Command mailbox created\n");

    // create gdos --> Messages are transmitted to GUI now
    gdos = new GdosMailbox(&cmdMbx, gdosLevel);
    if (!gdos)
    {
        ret = -ENOMEM;
        GDOS_ERROR("Can't create gdos, code = %d\n", ret);
        goto exit_error;
    }
    moduleInitBits.setBit(INIT_BIT_GDOS_CREATED);

    GDOS_PRINT("Init\n");

    // init rack time
    ret = rackTime.init(cmdMbx.getFd());
    if (ret)
    {
        GDOS_ERROR("Can't init rack time, code = %d\n", ret);
        goto exit_error;
    }

    offset = rackTime.getOffset();
    if (offset)
        GDOS_DBG_INFO("Using global time, offset: %.2f ms\n", (double)offset / 1000000);
    else
        GDOS_DBG_INFO("Using local time\n");

    // init random generation
    srand((unsigned int)rackTime.get());

    // create command task
    snprintf(cmdTaskName, sizeof(cmdTaskName), "%.28s%uC", classname, (unsigned int)inst);

    ret = cmdTask.create(cmdTaskName, 0, cmdTaskPrio, RACK_TASK_FPU | RACK_TASK_JOINABLE);
    if (ret)
    {
        GDOS_ERROR("Can't init command task, code = %d\n", ret);
        goto exit_error;
    }
    moduleInitBits.setBit(INIT_BIT_CMDTSK_CREATED);

    // create data task
    snprintf(dataTaskName, sizeof(dataTaskName), "%.28s%uD", classname, (unsigned int)inst);

    ret = dataTask.create(dataTaskName, 0, dataTaskPrio, RACK_TASK_FPU | RACK_TASK_JOINABLE);
    if (ret)
    {
        GDOS_ERROR("Can't init data task, code = %d\n", ret);
        goto exit_error;
    }
    moduleInitBits.setBit(INIT_BIT_DATATSK_CREATED);

    // fill rackParameterMsg
    ret = parseArgTable(module_argTab, NULL);
    ret += parseArgTable(arg_table, NULL);
    ret += 1;

    paramMsg = (rack_param_msg*)malloc(sizeof(rack_param_msg) + ret * sizeof(rack_param));
    if (!paramMsg)
    {
        GDOS_ERROR("Can't allocate memory for paramMsg\n");
        goto exit_error;
    }
    moduleInitBits.setBit(INIT_BIT_MALLOC_PARAM_MSG);

    strncpy(paramMsg->parameter[0].name, "name", RACK_PARAM_MAX_STRING_LEN);
    paramMsg->parameter[0].type = RACK_PARAM_STRING;
    strncpy(paramMsg->parameter[0].valueString, classname, RACK_PARAM_MAX_STRING_LEN);
    paramMsg->parameterNum = 1;

    parseArgTable(module_argTab, paramMsg);
    parseArgTable(arg_table, paramMsg);

    return 0;

exit_error:
    GDOS_ERROR("+++++++++++++++++++++++++++++++++++\n");
    GDOS_ERROR("+ Error while initializing module +\n");
    GDOS_ERROR("+++++++++++++++++++++++++++++++++++\n");

    // !!! call local cleanup function !!!
    RackModule::moduleCleanup();

    return ret;
}

// non realtime context
void      RackModule::moduleCleanup(void)
{
    if (moduleInitBits.testAndClearBit(INIT_BIT_MALLOC_PARAM_MSG))
    {
        free(paramMsg);
    }

    if (moduleInitBits.testAndClearBit(INIT_BIT_CMDTSK_STARTED))
    {
        cmdTask.join();
    }

    if (moduleInitBits.testAndClearBit(INIT_BIT_DATATSK_STARTED))
    {
        dataTask.join();
    }

    GDOS_PRINT("Terminated\n");

    deleteGdosMbx();

    // delete command mailbox
    if (moduleInitBits.testAndClearBit(INIT_BIT_CMDMBX_CREATED))
    {
        destroyMbx(&cmdMbx);
    }

    GDOS_DBG_INFO("Finished cleanup\n");
}

// realtime context
int RackModule::moduleCommand(message_info *msgInfo)
{
  rack_param_msg *newParam;
  int statusReply, ret;

  switch(msgInfo->type) {

    case MSG_ON:
        switch(targetStatus) {

          case MODULE_TSTATE_ON:

              if (status == MODULE_STATE_ENABLED) {
                ret = cmdMbx.sendMsgReply(MSG_OK, msgInfo);
                if (ret) {
                  GDOS_WARNING("CmdTask: Can't send moduleOn reply, code = %d\n", ret);
                  return ret;
                }
              } else {
                ret = cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
                if (ret) {
                  GDOS_WARNING("CmdTask: Can't send moduleOn reply, code = %d\n", ret);
                  return ret;
                }
              }
              return 0;

          case MODULE_TSTATE_OFF:

              targetStatus = MODULE_TSTATE_ON;
              memcpy(&replyMsgInfo, msgInfo, sizeof(message_info));
              return 0;

          default:
              ret = cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
              if (ret) {
                GDOS_WARNING("CmdTask: Can't send moduleOn reply, code = %d\n", ret);
              return ret;
            }
        }
        return 0;

    case MSG_OFF:
        targetStatus = MODULE_TSTATE_OFF;

        ret = cmdMbx.sendMsgReply(MSG_OK, msgInfo);
        if (ret) {
          GDOS_WARNING("CmdTask: Can't send moduleOff reply, code = %d\n", ret);
          return ret;
        }

        if (replyMsgInfo.src > 0) {
          ret = cmdMbx.sendMsgReply(MSG_ERROR, &replyMsgInfo);
          clearMsgInfo(&replyMsgInfo);
          return ret;
        }
        return 0;

    case MSG_GET_STATUS:
        if((status == MODULE_STATE_ERROR_WAIT) ||
           (status == MODULE_STATE_ERROR_RETRY))
        {
            statusReply = MODULE_STATE_ERROR;
        }
        else
        {
            statusReply = status;
        }

        ret = cmdMbx.sendMsgReply(statusReply, msgInfo);
        if (ret) {
          GDOS_WARNING("CmdTask: Can't send status, code = %d\n", ret);
          return ret;
        }

        return 0;

    case MSG_GET_PARAM:
        ret = cmdMbx.sendDataMsgReply(MSG_PARAM, msgInfo, 1, (void*)paramMsg, (uint32_t)(sizeof(rack_param_msg) + paramMsg->parameterNum * sizeof(rack_param)));
        if (ret) {
          GDOS_WARNING("CmdTask: Can't send parameter, code = %d\n", ret);
          return ret;
        }

        return 0;

    case MSG_SET_PARAM:
        newParam = RackParamMsg::parse(msgInfo);

        for(int i = 0; i < newParam->parameterNum; i++)
        {
            for(int j = 0; j < paramMsg->parameterNum; j++)
            {
                if(strncmp(newParam->parameter[i].name,
                           paramMsg->parameter[j].name,
                           RACK_PARAM_MAX_STRING_LEN) == 0)
                {
                    memcpy(&paramMsg->parameter[j], &newParam->parameter[i], sizeof(rack_param));
                }
            }

            if(strncmp(newParam->parameter[i].name, "gdosLevel", RACK_PARAM_MAX_STRING_LEN) == 0)
            {
                gdosLevel = GDOS_MSG_DEBUG_BEGIN - newParam->parameter[i].valueInt32;
                gdos->setGdosLevel(gdosLevel);
            }
        }

        ret = cmdMbx.sendMsgReply(MSG_OK, msgInfo);
        if (ret) {
          GDOS_WARNING("CmdTask: Can't send setParameter reply, code = %d\n", ret);
          return ret;
        }

        return 0;

    default:
      // nobody handles this command -> return MODULE_ERROR
      return -EINVAL;
  }
}

void  RackModule::run(void)
{
    int ret;

    // start command task
    ret = cmdTask.start(&cmd_task_proc, this);
    if (ret)
    {
        GDOS_ERROR("Can't start command task, code = %d\n", ret);
        goto exit_error;
    }
    moduleInitBits.setBit(INIT_BIT_CMDTSK_STARTED);

    // start data task
    ret = dataTask.start(&data_task_proc, this);
    if (ret)
    {
        GDOS_ERROR("Can't start data task, code = %d\n", ret);
        goto exit_error;
    }
    moduleInitBits.setBit(INIT_BIT_DATATSK_STARTED);

    pause();

exit_error:
    // call top level cleanup function
    moduleCleanup();
    return;
}

/*@}*/

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

static RackModule *p_signal_module = NULL;
static int signal_flags = 0;

#define HANDLING_TERM           0x0001
#define HANDLING_SEGV           0x0002

void signal_handler(int sig)
{
#ifdef __XENO__
    int nentries;
    void *array[10];
    char **strings;
    int i;
#endif

    printf("---=== SIGNAL HANDLER of %s ===---\n", classname);

    switch(sig)
    {
#ifdef __XENO__
        case SIGXCPU:
            printf(" -> SIGXCPU (%02d)\n", sig);
            printf(" !!! WARNING unexpected switch to secondary mode !!!\n");
            nentries = backtrace (array, 10);
            strings = backtrace_symbols (array, nentries);
            for (i = 0; i < nentries; i++)
            {
                printf ("  %s\n", strings[i]);
            }
            free (strings);
            break;

        case SIGSEGV:
            printf(" -> SIGSEGV (%02d)\n", sig);
            printf(" -> Segmentation fault \n");

            if ((signal_flags & HANDLING_SEGV) || // handle segmentation fault only once
                (signal_flags & HANDLING_TERM))   // handle segmentation fault only if module is not terminated
                break;

            signal_flags |= HANDLING_SEGV;

            nentries = backtrace (array, 10);
            strings = backtrace_symbols (array, nentries);
            for (i = 0; i < nentries; i++)
            {
                printf ("  %s\n", strings[i]);
            }
            free (strings);

            p_signal_module->moduleTerminate();
            break;
#endif
        case SIGTERM:
        case SIGINT:
            printf(" -> SIGTERM, SIGINT (%02d)\n", sig);

            if (signal_flags & HANDLING_TERM) // call moduleCleanup only once
                break;

            signal_flags |= HANDLING_TERM;

            p_signal_module->moduleTerminate();
            p_signal_module->moduleCleanup();
            break;

        default:
            printf(" -> SIGNAL (%02d)\n", sig);
            printf(" -> NOT HANDLED\n");
    }
    printf("---=== END OF %s SIGNAL HANDLER ===---\n", classname);
    return;
}

int init_signal_handler(RackModule *p_mod)
{
    if (p_signal_module)
    {
        printf("signal handler is allready defined\n");
        return -EBUSY;
    }

    p_signal_module = p_mod;

    signal(SIGTERM, signal_handler);
    signal(SIGINT,  signal_handler);
#ifdef __XENO__
    signal(SIGXCPU, signal_handler);
    signal(SIGSEGV, signal_handler);
#endif

    return 0;
}
