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
package rack.main;

import rack.main.tims.*;

public abstract class RackDataProxy extends RackProxy
{
    public RackDataProxy(int commandMbx, TimsMbx replyMbx, int onTimeout,
            int offTimeout, int dataTimeout)
    {
        super(commandMbx, replyMbx, onTimeout, offTimeout, dataTimeout);
    }

    public RackDataProxy(int commandMbx, TimsMbx replyMbx, TimsMbx dataMbx,
            int onTimeout, int offTimeout, int dataTimeout)
    {
        super(commandMbx, replyMbx, dataMbx, onTimeout, offTimeout, dataTimeout);
    }

    protected synchronized TimsDataMsg getRawData(int recordingTime)
    {
        currentSequenceNo++;

        try
        {
            GetDataMsg p = new GetDataMsg();
            p.recordingTime = recordingTime;

            replyMbx.send(RackProxy.MSG_GET_DATA, commandMbx,
                          (byte) 0, currentSequenceNo, p);

            TimsDataMsg reply;

            do
            {

                reply = replyMbx.receive(dataTimeout);

            }
            while (reply.seqNr != currentSequenceNo);

            if (reply.type == RackProxy.MSG_DATA)
            {

                // System.out.println(RackName.nameString(replyMbx) + ": " +
                // RackName.nameString(commandMbx) + ".getData");
                return (reply);
            }
            else
            {
                // System.out.println(RackName.nameString(replyMbx) + ": " +
                // RackName.nameString(commandMbx) + ".getData replied error");
                return (null);
            }
        }
        catch (TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx) + ".getData " + e);
            return (null);
        }
    }

    public synchronized void getContData(int periodTime)
    {
        currentSequenceNo++;

        if (dataMbx == null)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx)
                    + ".getContData: Keine Datenmailbox eingerichtet.");
            return;
        }
        try
        {
            GetContDataMsg p = new GetContDataMsg();
            p.periodTime = periodTime;
            p.dataMbx = dataMbx.getName();

            replyMbx.send(RackProxy.MSG_GET_CONT_DATA, commandMbx,
                          (byte) 0, currentSequenceNo, p);

            TimsDataMsg reply;
            do
            {
                reply = replyMbx.receive(dataTimeout);
            }
            while (reply.seqNr != currentSequenceNo);

            // System.out.println(RackName.nameString(replyMbx) + ": " +
            // RackName.nameString(commandMbx) +
            // ".getContinuousData frequency " + frequency);
        }
        catch (TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx) + ".getContData " + e);
        }
    }

    public synchronized void stopContData()
    {
        currentSequenceNo++;

        if (dataMbx == null)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx)
                    + ".stopContData: Keine Datenmailbox eingerichtet.");
            return;
        }

        try
        {
            StopContDataMsg p = new StopContDataMsg();
            p.dataMbx = dataMbx.getName();

            replyMbx.send(RackProxy.MSG_STOP_CONT_DATA, commandMbx,
                          (byte) 0, currentSequenceNo, p);

            TimsDataMsg reply;
            do
            {
                reply = replyMbx.receive(dataTimeout);
            }
            while (reply.seqNr != currentSequenceNo);

            // System.out.println(RackName.nameString(replyMbx) + ": " +
            // RackName.nameString(commandMbx) +
            // ".stopContData");
        }
        catch (TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx) + ".stopContData " + e);
        }
    }
}
