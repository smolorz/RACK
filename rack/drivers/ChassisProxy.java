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
package rack.drivers;

import rack.main.naming.*;

import rack.main.proxy.*;
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;
import rack.main.tims.router.*;


public class ChassisProxy extends RackDataProxy
{
    public static final byte MSG_CHASSIS_MOVE =
        RackMsgType.RACK_PROXY_MSG_POS_OFFSET + 1;

    public static final byte MSG_CHASSIS_GET_PARAMETER =
        RackMsgType.RACK_PROXY_MSG_POS_OFFSET + 2;

    public static final byte MSG_CHASSIS_SET_ACTIVE_PILOT =
        RackMsgType.RACK_PROXY_MSG_POS_OFFSET + 3;

    public static final byte MSG_CHASSIS_PARAMETER =
        RackMsgType.RACK_PROXY_MSG_NEG_OFFSET - 1;

    public static final byte INVAL_PILOT = -1;

    public ChassisProxy(int id, int replyMbx)
    {
        super(RackName.create(RackName.CHASSIS, id), replyMbx, 5000, 1000, 1000);
        this.id = id;
    }

    public synchronized ChassisDataMsg getData(int recordingtime)
    {
        try
        {
            TimsDataMsg raw = getRawData(recordingtime);
            if (raw != null)
            {
                ChassisDataMsg data = new ChassisDataMsg(raw);
                return data;
            }
            else
            {
                return null;
            }
        }
        catch (MsgException e)
        {
            System.out.println(e.toString());
            return null;
        }
    }

    public synchronized ChassisDataMsg getData()
    {
        return (getData(0));
    }

    public synchronized ChassisParamMsg getParam()
    {
        currentSequenceNo++;
        try
        {
            TimsMsgRouter.send0(MSG_CHASSIS_GET_PARAMETER, commandMbx, replyMbx,
                    (byte) 0, currentSequenceNo);

            TimsDataMsg reply;
            do
            {
                reply = TimsMsgRouter.receive(replyMbx, 1000);
            }
            while ((reply.seq_nr != currentSequenceNo)
                    & (reply.type == MSG_CHASSIS_PARAMETER));
            
            ChassisParamMsg data = new ChassisParamMsg(reply);
            return (data);
        }
        catch (MsgException e)
        {
        	System.out.println(e);
            return (null);
        }
    }

    public synchronized void setActivePilot(int pilotMbx)
    {
        currentSequenceNo++;
        try
        {
            ChassisSetActivePilotMsg cmdMsg = new ChassisSetActivePilotMsg(
                    pilotMbx);

            TimsMsgRouter.send(MSG_CHASSIS_SET_ACTIVE_PILOT, commandMbx,
                    replyMbx, (byte) 0, currentSequenceNo, cmdMsg);

            TimsDataMsg reply;
            do
            {
                reply = TimsMsgRouter.receive(replyMbx, onTimeout);
            }
            while (reply.seq_nr != currentSequenceNo);

            if (reply.type == RackMsgType.MSG_OK)
            {
                System.out.println(RackName.nameString(replyMbx) + ": "
                        + RackName.nameString(commandMbx) + ".setActivePilot");
            }
            else
            {
                System.out.println(RackName.nameString(replyMbx) + ": "
                        + RackName.nameString(commandMbx)
                        + ".setActivePilot replied error");
            }
        }
        catch (MsgException e)
        {
            System.out.println(RackName.nameString(replyMbx) + ": "
                    + RackName.nameString(commandMbx) + ".on " + e);
        }
    }

    public int getCommandMbx()
    {
        return (RackName.create(RackName.CHASSIS, id));
    }
}
