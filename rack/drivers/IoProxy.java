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
 *      Matthias Hentschel  <hentschel@rts.uni-hannover.de>
 *
 */
package rack.drivers;

import rack.main.*;
import rack.main.tims.*;
import rack.main.RackName;

public class IoProxy extends RackDataProxy
{
    public static final byte MSG_IO_SET_DATA =
        RackProxy.MSG_POS_OFFSET + 1;

    public IoProxy(int system, int instance, TimsMbx replyMbx)
    {
        super(RackName.create(system, RackName.IO, instance, 0), replyMbx, 500);
    }

    public synchronized IoDataMsg getData(int recordingTime)
    {
        try
        {
            TimsRawMsg raw = getRawData(recordingTime);

            if (raw != null)
            {
                IoDataMsg data = new IoDataMsg(raw);
                return data;
            }
            else
            {
                return null;
            }
        }
        catch(TimsException e)
        {
            System.out.println(e.toString());
            return null;
        }
    }

    public synchronized IoDataMsg getData()
    {
        return(getData(0));
    }

    public synchronized void setData(IoDataMsg data)
    {
        currentSequenceNo++;
        try
        {
            replyMbx.send(MSG_IO_SET_DATA, commandMbx,
                               (byte)0, currentSequenceNo, data);

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
                               ".setData " + e);
        }
    }
}
