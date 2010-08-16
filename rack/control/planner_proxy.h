/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Daniel Lecking <lecking@rts.uni-hannover.de>
 *
 */

#ifndef __PLANNER_PROXY_H__
#define __PLANNER_PROXY_H__

#include <main/rack_proxy.h>

#define MAX_STRING_LEN      80
#define MAX_MESSAGE_NUM     50
#define MAX_COMMAND_NUM     50

//######################################################################
//# Planner Message Types
//######################################################################

#define MSG_PLANNER_COMMAND             (RACK_PROXY_MSG_POS_OFFSET + 1)
#define MSG_PLANNER_GET_COMMAND         (RACK_PROXY_MSG_POS_OFFSET + 2)

//######################################################################
//# PlannerData
//######################################################################

typedef struct{
    int             stringLen;
    char            string[MAX_STRING_LEN];
} __attribute__((packed)) planner_string;

typedef struct{
    rack_time_t     recordingTime; // has to be first element
    int             state;
    int             messageNum;
    planner_string  message[MAX_MESSAGE_NUM];
} __attribute__((packed)) planner_data;

typedef struct{
    int             commandNum;
    planner_string  command[MAX_COMMAND_NUM];
} __attribute__((packed)) planner_command;

class PlannerData
{
    public:
        static void le_to_cpu(planner_data *data)
        {
            data->recordingTime     = __le32_to_cpu(data->recordingTime);
            data->state             = __le32_to_cpu(data->state);
            data->messageNum        = __le32_to_cpu(data->messageNum);
        }

        static void be_to_cpu(planner_data *data)
        {
            data->recordingTime     = __be32_to_cpu(data->recordingTime);
            data->state             = __be32_to_cpu(data->state);
            data->messageNum        = __be32_to_cpu(data->messageNum);
        }

        static planner_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            planner_data *p_data = (planner_data *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }
};

//######################################################################
//# Planner Command
//######################################################################

class PlannerCommand
{
        public:
        static void le_to_cpu(planner_command *data)
        {
            data->commandNum     = __le32_to_cpu(data->commandNum);
        }

        static void be_to_cpu(planner_command *data)
        {
            data->commandNum     = __be32_to_cpu(data->commandNum);
        }

        static planner_command* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            planner_command *p_data = (planner_command *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }
};

//######################################################################
//# Planner Proxy
//######################################################################

class PlannerProxy : public RackDataProxy
{

      public:

        PlannerProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
                : RackDataProxy(workMbx, sys_id, PLANNER, instance)
        {
        };

        ~PlannerProxy()
        {
        };


//
// planner get data
//

        int getData(planner_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp)
        {
              return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
        }

        int getData(planner_data *recv_data, ssize_t recv_datalen,
                    rack_time_t timeStamp, uint64_t reply_timeout_ns);

//
// planner getCommandList
//

        int getCommandList(planner_command *recv_data, ssize_t recv_datalen)
        {
            return getCommandList(recv_data, recv_datalen, dataTimeout);
        }

        int getCommandList(planner_command *recv_data, ssize_t recv_datalen,
                 uint64_t reply_timeout_ns);


        int sendCommand(planner_command *recv_data, ssize_t recv_datalen)
        {
            return sendCommand(recv_data, recv_datalen, dataTimeout);
        }

        int sendCommand(planner_command *recv_data, ssize_t recv_datalen,
                 uint64_t reply_timeout_ns);

};

#endif // __PLANNER_PROXY_H__
