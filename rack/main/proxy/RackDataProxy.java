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
package rack.main.proxy;

import rack.main.naming.*;
import rack.main.tims.Tims;
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;

public abstract class RackDataProxy extends RackProxy
{
    public RackDataProxy(int commandMbx, int replyMbx, int onTimeout,
            int offTimeout, int dataTimeout)
    {
        super(commandMbx, replyMbx, onTimeout, offTimeout, dataTimeout);
    }

    public RackDataProxy(int commandMbx, int replyMbx, int dataMbx,
            int onTimeout, int offTimeout, int dataTimeout)
    {
        super(commandMbx, replyMbx, dataMbx, onTimeout, offTimeout, dataTimeout);
    }

    protected synchronized TimsDataMsg getRawData(int recordingtime)
    {
        currentSequenceNo++;

        try
        {
            GetDataMsg p = new GetDataMsg();
            p.recordingtime = recordingtime;

            Tims.send(RackMsgType.MSG_GET_DATA, commandMbx, replyMbx,
                    (byte) 0, currentSequenceNo, p);

            TimsDataMsg reply;

            do
            {

                reply = Tims.receive(replyMbx, dataTimeout);

            }
            while (reply.seq_nr != currentSequenceNo);

            if (reply.type == RackMsgType.MSG_DATA)
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
        catch (MsgException e)
        {
            System.out.println(RackName.nameString(replyMbx) + ": "
                    + RackName.nameString(commandMbx) + ".getData " + e);
            return (null);
        }
    }

    public synchronized void getContData(int periodTime)
    {
        currentSequenceNo++;

        if (dataMbx == 0)
        {
            System.out.println(RackName.nameString(replyMbx) + ": "
                    + RackName.nameString(commandMbx)
                    + ".getContData: Keine Datenmailbox eingerichtet.");
            return;
        }
        try
        {
            GetContDataMsg p = new GetContDataMsg();
            p.periodTime = periodTime;
            p.dataMbx = dataMbx;

            Tims.send(RackMsgType.MSG_GET_CONT_DATA, commandMbx,
                    replyMbx, (byte) 0, currentSequenceNo, p);

            TimsDataMsg reply;
            do
            {
                reply = Tims.receive(replyMbx, dataTimeout);
            }
            while (reply.seq_nr != currentSequenceNo);

            // System.out.println(RackName.nameString(replyMbx) + ": " +
            // RackName.nameString(commandMbx) +
            // ".getContinuousData frequency " + frequency);
        }
        catch (MsgException e)
        {
            System.out.println(RackName.nameString(replyMbx) + ": "
                    + RackName.nameString(commandMbx) + ".getContData " + e);
        }
    }

    public synchronized void stopContData()
    {
        currentSequenceNo++;

        if (dataMbx == 0)
        {
            System.out.println(RackName.nameString(replyMbx) + ": "
                    + RackName.nameString(commandMbx)
                    + ".stopContData: Keine Datenmailbox eingerichtet.");
            return;
        }

        try
        {
            StopContDataMsg p = new StopContDataMsg();
            p.dataMbx = dataMbx;

            Tims.send(RackMsgType.MSG_STOP_CONT_DATA, commandMbx,
                    replyMbx, (byte) 0, currentSequenceNo, p);

            TimsDataMsg reply;
            do
            {
                reply = Tims.receive(replyMbx, dataTimeout);
            }
            while (reply.seq_nr != currentSequenceNo);

            // System.out.println(RackName.nameString(replyMbx) + ": " +
            // RackName.nameString(commandMbx) +
            // ".stopContData");
        }
        catch (MsgException e)
        {
            System.out.println(RackName.nameString(replyMbx) + ": "
                    + RackName.nameString(commandMbx) + ".stopContData " + e);
        }
    }
}
