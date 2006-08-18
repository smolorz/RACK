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

import rack.main.naming.*;
import rack.main.proxy.*;
import rack.main.tims.Tims;
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.RackMsgType;
import rack.main.tims.exceptions.*;
import rack.main.defines.Position3D;;

public class PositionProxy extends RackDataProxy
{
    public static final byte MSG_POSITION_UPDATE =
        RackMsgType.RACK_PROXY_MSG_POS_OFFSET + 1;

    public PositionProxy(int id , int replyMbx)
    {
        super(RackName.create(RackName.POSITION, id), replyMbx, 5000, 1000, 1000);
        this.id = id;
    }

    public PositionProxy(int id , int replyMbx, int dataMbx)
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
        catch(MsgException e)
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
            Tims.send(MSG_POSITION_UPDATE,
                               commandMbx,
                               replyMbx,
                               (byte)0,
                               (byte)currentSequenceNo,
                               updateMsg);

            TimsDataMsg reply;

            do
            {
                reply = Tims.receive(replyMbx, 1000);
                System.out.println(RackName.nameString(replyMbx) + ": " + reply.type + " " + reply.seqNr);
            }
            while(reply.seqNr != currentSequenceNo);
        }
        catch(MsgException e)
        {
            System.out.println(RackName.nameString(replyMbx) + ": " + RackName.nameString(commandMbx) + ".update " + e);
        }
    }

    public PositionDataMsg readContinuousData(int timeOut) {
        if (dataMbx == 0) {
            System.out.println(RackName.nameString(replyMbx) + ": " + RackName.nameString(commandMbx) + ".readContinuousData: Keine Datenmailbox eingerichtet."  );
            return null;
        }
        try
        {
            TimsDataMsg data = Tims.receive(dataMbx, timeOut);
            PositionDataMsg locData = new PositionDataMsg(data);
            System.out.println(RackName.nameString(replyMbx) + ": " + RackName.nameString(commandMbx) + ".readContinuousData");
            return(locData);
        }
        catch(MsgException e)
        {
//            System.out.println(RackName.nameString(replyMbx) + ": " + RackName.nameString(commandMbx) + ".readContinuousData "  + e);
            return(null);
        }

    }

    public int getCommandMbx()
    {
        return(RackName.create(RackName.POSITION, id));
    }
}
