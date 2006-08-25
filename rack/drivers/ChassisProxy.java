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

import rack.main.*;
import rack.main.tims.*;


public class ChassisProxy extends RackDataProxy
{
    public static final byte MSG_CHASSIS_MOVE =
        RackProxy.MSG_POS_OFFSET + 1;

    public static final byte MSG_CHASSIS_GET_PARAMETER =
        RackProxy.MSG_POS_OFFSET + 2;

    public static final byte MSG_CHASSIS_SET_ACTIVE_PILOT =
        RackProxy.MSG_POS_OFFSET + 3;

    public static final byte MSG_CHASSIS_PARAMETER =
        RackProxy.MSG_NEG_OFFSET - 1;

    public static final byte INVAL_PILOT = -1;

    public ChassisProxy(int id, TimsMbx replyMbx)
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
        catch (TimsException e)
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
            replyMbx.send0(MSG_CHASSIS_GET_PARAMETER, commandMbx,
                    (byte) 0, currentSequenceNo);

            TimsDataMsg reply;
            do
            {
                reply = replyMbx.receive(1000);
            }
            while ((reply.seqNr != currentSequenceNo)
                    & (reply.type == MSG_CHASSIS_PARAMETER));
            
            ChassisParamMsg data = new ChassisParamMsg(reply);
            return (data);
        }
        catch (TimsException e)
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

            replyMbx.send(MSG_CHASSIS_SET_ACTIVE_PILOT, commandMbx,
                          (byte) 0, currentSequenceNo, cmdMsg);

            TimsDataMsg reply;
            do
            {
                reply = replyMbx.receive(onTimeout);
            }
            while (reply.seqNr != currentSequenceNo);

            if (reply.type == RackProxy.MSG_OK)
            {
                System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                        + RackName.nameString(commandMbx) + ".setActivePilot");
            }
            else
            {
                System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                        + RackName.nameString(commandMbx)
                        + ".setActivePilot replied error");
            }
        }
        catch (TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx) + ".on " + e);
        }
    }

    public int getCommandMbx()
    {
        return (RackName.create(RackName.CHASSIS, id));
    }
}
