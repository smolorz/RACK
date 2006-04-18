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
#ifndef __JOYSTICK_PROXY_H__
#define __JOYSTICK_PROXY_H__

#include <main/rack_proxy.h>
#include <main/defines/position3d.h>

#define JOYSTICK_MAX_LED    8

//######################################################################
//# Joystick Message Types
//######################################################################

#define MSG_SET_POSITION            (RACK_PROXY_MSG_POS_OFFSET + 1)

//######################################################################
//# Joystick Data (static size - MESSAGE)
//######################################################################

typedef struct {
    RACK_TIME	recordingTime;  // have to be first element
    position_3d position;       // joystick-position in percent -100<x<100 ...
    int32_t     buttons;        // binary button information bit0->button0, ...
} __attribute__((packed)) joystick_data;

class JoystickData
{
    public:
        static void le_to_cpu(joystick_data *data)
        {
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            Position3D::le_to_cpu(&data->position);
            data->buttons       = __le32_to_cpu(data->buttons);
        }

        static void be_to_cpu(joystick_data *data)
        {
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            Position3D::be_to_cpu(&data->position);
            data->buttons       = __be32_to_cpu(data->buttons);
        }

        static joystick_data* parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            joystick_data *p_data = (joystick_data *)msgInfo->p_data;

            if (msgInfo->flags & MSGINFO_DATA_LE) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->usedMbx->setDataByteorder(msgInfo);
            return p_data;
        }
};

//######################################################################
//# Chassis Proxy Functions
//######################################################################

class JoystickProxy : public RackDataProxy
{

  	public:

	    JoystickProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
	            : RackDataProxy(workMbx, sys_id, JOYSTICK, instance)
	    {
	    };

	    ~JoystickProxy()
	    {
	    };

//
// joystick data
//

	    int getData(joystick_data *recv_data, ssize_t recv_datalen,
	                RACK_TIME timeStamp, MessageInfo *msgInfo)
	    {
	      	return getData(recv_data, recv_datalen, timeStamp,
	      	               dataTimeout, msgInfo);
	    }

	    int getData(joystick_data *recv_data, ssize_t recv_datalen,
	                RACK_TIME timeStamp, uint64_t reply_timeout_ns,
	                MessageInfo *msgInfo);

//
// set data
//

	    int setData(joystick_data *p_data)
	    {
	      return setData(p_data, dataTimeout);
	    }

	    int setData(joystick_data *p_data, uint64_t reply_timeout_ns)
	    {
	        return proxySendDataCmd(MSG_SET_POSITION, &p_data,
	                                sizeof(joystick_data), reply_timeout_ns);
	    }

};

#endif //_JOYSTICK_PROXY_H_
