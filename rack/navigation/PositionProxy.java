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
package rack.navigation;

import rack.main.*;
import rack.main.tims.*;
import rack.main.defines.*;

public class PositionProxy extends RackDataProxy
{
    public static final byte MSG_POSITION_UPDATE =
        RackProxy.MSG_POS_OFFSET + 1;

    public PositionProxy(int id , TimsMbx replyMbx)
    {
        super(RackName.create(RackName.POSITION, id), replyMbx, 5000, 1000, 1000);
        this.id = id;
    }

    public PositionProxy(int id , TimsMbx replyMbx, TimsMbx dataMbx)
    {
        super(RackName.create(RackName.POSITION, id), replyMbx, dataMbx, 5000, 1000, 1000);
        this.id = id;
    }

    public synchronized PositionDataMsg getData(int recordingtime)
    {
        try
        {
            TimsDataMsg raw = getRawData(recordingtime);
            if (raw!=null) {
                PositionDataMsg data = new PositionDataMsg(raw);
                return(data);
            }

            return(null);
        }
        catch(TimsException e)
        {
            System.out.println(e.toString());
            return(null);
        }
    }

    public synchronized PositionDataMsg getData()
    {
        return(getData(0));
    }

    public synchronized void update(Position3D pos, int recordingTime)
    {
        currentSequenceNo++;

        try {
            PositionDataMsg updateMsg = new PositionDataMsg();
            updateMsg.recordingTime = recordingTime;
            updateMsg.pos = pos;
            replyMbx.send(MSG_POSITION_UPDATE,
                               commandMbx,
                               (byte)0,
                               (byte)currentSequenceNo,
                               updateMsg);

            TimsDataMsg reply;

            do
            {
                reply = replyMbx.receive(1000);
                System.out.println(replyMbx.getNameString() + ": " + reply.type + " " + reply.seqNr);
            }
            while(reply.seqNr != currentSequenceNo);
        }
        catch(TimsException e)
        {
            System.out.println(replyMbx.getNameString() + ": " + RackName.nameString(commandMbx) + ".update " + e);
        }
    }

    public PositionDataMsg readContinuousData(int timeOut) {
        if (dataMbx == null) {
            System.out.println(replyMbx.getNameString() + ": " + RackName.nameString(commandMbx) + ".readContinuousData: Keine Datenmailbox eingerichtet."  );
            return null;
        }
        try
        {
            TimsDataMsg data = dataMbx.receive(timeOut);
            PositionDataMsg locData = new PositionDataMsg(data);
            System.out.println(replyMbx.getNameString() + ": " + RackName.nameString(commandMbx) + ".readContinuousData");
            return(locData);
        }
        catch(TimsException e)
        {
//            System.out.println(replyMbx.nameString() + ": " + RackName.nameString(commandMbx) + ".readContinuousData "  + e);
            return(null);
        }

    }

    public int getCommandMbx()
    {
        return(RackName.create(RackName.POSITION, id));
    }
}
