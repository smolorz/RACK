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
 *      Oliver Wulf        <oliver.wulf@gmx.de>
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
package rack.tools;

import rack.main.*;
import rack.main.tims.*;

public class DatalogProxy extends RackDataProxy
{
    public static final byte MSG_DATALOG_SET_LOG =
        RackProxy.MSG_POS_OFFSET + 1;

    public static final byte MSG_DATALOG_GET_LOG_STATUS =
        RackProxy.MSG_POS_OFFSET + 2;

    public static final byte MSG_DATALOG_RESET =
        RackProxy.MSG_POS_OFFSET + 3;

    public static final byte MSG_DATALOG_LOG_STATUS =
        RackProxy.MSG_NEG_OFFSET - 1;

    public DatalogProxy(int id, TimsMbx replyMbx)
    {
        super(RackName.create(RackName.DATALOG, id), replyMbx, 10000, 5000, 5000);
        this.id = id;
    }
        
    public synchronized void reset()
    {
        currentSequenceNo++;
        try
        {
            replyMbx.send0(MSG_DATALOG_RESET, commandMbx,
                           (byte) 0, currentSequenceNo);

            TimsDataMsg reply;
            do
            {
                reply = replyMbx.receive(1000);
            }
            while (reply.seqNr != currentSequenceNo);
                    
            if (reply.type == RackProxy.MSG_OK)
            {
                System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                        + RackName.nameString(commandMbx) + ".reset");
            }
            else
            {
                System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                        + RackName.nameString(commandMbx)
                        + ".reset replied error");
            }            
        }
        catch (TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx) + ".reset " + e);
        }
    }

    public synchronized DatalogLogInfoMsg getLogStatus()
    {
        currentSequenceNo++;
        try
        {
            replyMbx.send0(MSG_DATALOG_GET_LOG_STATUS, commandMbx,
                           (byte) 0, currentSequenceNo);

            TimsDataMsg reply;
            do
            {
                reply = replyMbx.receive(1000);
            }
            while ((reply.seqNr != currentSequenceNo) &
                   (reply.type == MSG_DATALOG_LOG_STATUS));

            DatalogLogInfoMsg data = new DatalogLogInfoMsg(reply);
            return(data);
        }
        catch (TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx) + ".getLogStatus " + e);
            return(null);
        }
    }
    
    public synchronized void setLog(DatalogLogInfoMsg data)
    {
        currentSequenceNo++;

        try
        {
            replyMbx.send(MSG_DATALOG_SET_LOG, commandMbx,
                               (byte)0, currentSequenceNo, data);
        
            TimsDataMsg reply;
            do
            {
                reply = replyMbx.receive(1000);
            }
            while(reply.seqNr != currentSequenceNo);

            if (reply.type == RackProxy.MSG_OK)
            {
                System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                        + RackName.nameString(commandMbx) + ".setLog");
            }
            else
            {
                System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                        + RackName.nameString(commandMbx)
                        + ".setLog replied error");
            }            
        }
        catch (TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx) + ".setLog " + e);
        }
    }

    public int getCommandMbx()
    {
        return(RackName.create(RackName.DATALOG, id));
    }
}
