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
 *      Daniel Lecking  <lecking@rts.uni-hannover.de>
 *
 */
package rack.control;

import rack.main.*;
import rack.main.tims.*;
import rack.main.RackName;

public class PlannerProxy extends RackDataProxy{

	public static final byte MSG_PLANNER_COMMAND     = RackProxy.MSG_POS_OFFSET + 1;
	public static final byte MSG_PLANNER_GET_COMMAND = RackProxy.MSG_POS_OFFSET + 2;

	public PlannerProxy(int system, int instance, TimsMbx replyMbx)
    {
        super(RackName.create(system, RackName.PLANNER, instance, 0), replyMbx, 5000);
    }

    public synchronized PlannerDataMsg getData(int recordingTime)
    {
        try
        {
            TimsRawMsg raw = getRawData(recordingTime);

            if (raw != null)
            {
                PlannerDataMsg data = new PlannerDataMsg(raw);
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

    public synchronized PlannerDataMsg getData()
    {
        return (getData(0));
    }

    public synchronized PlannerCommandMsg getCommandList()
    {
        currentSequenceNo++;
        try
        {
            replyMbx.send0(MSG_PLANNER_GET_COMMAND, commandMbx,
                    (byte) 0, currentSequenceNo);

            TimsRawMsg reply;
            do
            {
                reply = replyMbx.receive(replyTimeout);
            }
            while ((reply.seqNr != currentSequenceNo)
                    & (reply.type == MSG_PLANNER_COMMAND));

            PlannerCommandMsg commandList = new PlannerCommandMsg(reply);
            return (commandList);
        }
        catch (TimsException e)
        {
        	System.out.println(e);
            return (null);
        }
    }

    public synchronized void sendCommand(byte msgType,PlannerCommandMsg sendCommandList)
    {
    	currentSequenceNo++;

        try
        {
            replyMbx.send(msgType, commandMbx,
                               (byte)0, currentSequenceNo, sendCommandList);

            TimsRawMsg reply;
            do
            {
              reply = replyMbx.receive(replyTimeout);
            }
            while(reply.seqNr != currentSequenceNo);
        }
        catch(TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": " +
                               RackName.nameString(commandMbx) +
                               ".sendPlannerData " + e);
        }
    }
}
