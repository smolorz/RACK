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
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;
import rack.main.tims.router.*;

public abstract class RackProxy
{

    protected int commandMbx = 0;
    protected int replyMbx = 0;

    /** zum Empfang von kontinuierlichen Daten */
    protected int dataMbx = 0;
    protected int id = 0;

    protected int onTimeout = 0;
    protected int offTimeout = 0;
    protected int dataTimeout = 0;

    protected byte currentSequenceNo = 0;

    public RackProxy(int commandMbx, int replyMbx, int onTimeout,
            int offTimeout, int dataTimeout)
    {
        this.commandMbx = commandMbx;
        this.replyMbx = replyMbx;
        this.onTimeout = onTimeout;
        this.offTimeout = offTimeout;
        this.dataTimeout = dataTimeout;
    }

    public RackProxy(int commandMbx, int replyMbx, int dataMbx, int onTimeout,
            int offTimeout, int dataTimeout)
    {
        this.commandMbx = commandMbx;
        this.replyMbx = replyMbx;
        this.dataMbx = dataMbx;
        this.onTimeout = onTimeout;
        this.offTimeout = offTimeout;
        this.dataTimeout = dataTimeout;
    }

    public synchronized void on()
    {
        currentSequenceNo++;
        try
        {
            TimsMsgRouter.send0(RackMsgType.MSG_ON, commandMbx, replyMbx,
                    (byte) 0, currentSequenceNo);
            TimsMsg reply;

            do
            {
                reply = TimsMsgRouter.receive(replyMbx, onTimeout);
            }
            while (reply.seq_nr != currentSequenceNo);

            // System.out.println(RackName.nameString(replyMbx) + ": " +
            // RackName.nameString(commandMbx) + ".on");
        }
        catch (MsgException e)
        {
            System.out.println(RackName.nameString(replyMbx) + ": "
                    + RackName.nameString(commandMbx) + ".on " + e);
        }
    }

    public synchronized void off()
    {
        currentSequenceNo++;
        try
        {
            TimsMsgRouter.send0(RackMsgType.MSG_OFF, commandMbx, replyMbx,
                    (byte) 0, currentSequenceNo);
            TimsMsg reply;

            do
            {
                reply = TimsMsgRouter.receive(replyMbx, offTimeout);
            }
            while (reply.seq_nr != currentSequenceNo);

            // System.out.println(RackName.nameString(replyMbx) + ": " +
            // RackName.nameString(commandMbx) + ".off");
        }
        catch (MsgException e)
        {
            System.out.println(RackName.nameString(replyMbx) + ": "
                    + RackName.nameString(commandMbx) + ".off " + e);
        }
    }

    public synchronized int getStatus()
    {
        currentSequenceNo++;
        try
        {
            TimsMsgRouter.send0(RackMsgType.MSG_GET_STATUS, commandMbx,
                    replyMbx, (byte) 0, currentSequenceNo);
            TimsMsg reply;

            do
            {
                reply = TimsMsgRouter.receive(replyMbx, dataTimeout);
            }
            while (reply.seq_nr != currentSequenceNo);

            /*
             * switch(reply.type) { case RackMsgType.MSG_ENABLED:
             * System.out.println(RackName.nameString(replyMbx) + ": " +
             * RackName.nameString(commandMbx) + ".getStatus ENABLED"); break;
             * case RackMsgType.MSG_DISABLED:
             * System.out.println(RackName.nameString(replyMbx) + ": " +
             * RackName.nameString(commandMbx) + ".getStatus DISABLED"); break;
             * case RackMsgType.MSG_ERROR:
             * System.out.println(RackName.nameString(replyMbx) + ": " +
             * RackName.nameString(commandMbx) + ".getStatus ERROR"); break;
             * case RackMsgType.MSG_NOT_AVAILABLE:
             * System.out.println(RackName.nameString(replyMbx) + ": " +
             * RackName.nameString(commandMbx) + ".getStatus NOT_AVAILABLE");
             * break; default: System.out.println(RackName.nameString(replyMbx) + ": " +
             * RackName.nameString(commandMbx) + ".getStatus ERROR unknown type " +
             * reply.type); break; }
             */

            return (reply.type);
        }
        catch (MsgTimeoutException e)
        {
            // System.out.println(RackName.nameString(replyMbx) + ": " +
            // RackName.nameString(commandMbx) +
            // ".getStatus TIMEOUT");

            return (RackMsgType.MSG_TIMEOUT);

        }
        catch (MsgException e)
        {
            System.out
                    .println(RackName.nameString(replyMbx) + ": "
                            + RackName.nameString(commandMbx)
                            + ".getStatus ERROR " + e);

            return (RackMsgType.MSG_ERROR);
        }
    }

    /**
     * @return instanceId
     */
    public int getInstanceId()
    {
        return id;
    }

    public abstract int getCommandMbx();

}
