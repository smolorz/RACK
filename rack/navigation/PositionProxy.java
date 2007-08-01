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
    public static final byte MSG_POSITION_WGS84_TO_POS =
        RackProxy.MSG_POS_OFFSET + 2;
    public static final byte MSG_POSITION_POS_TO_WGS84 =
        RackProxy.MSG_POS_OFFSET + 3;
    
    public static final byte MSG_POSITION_POS =
        RackProxy.MSG_NEG_OFFSET - 2;
    public static final byte MSG_POSITION_WGS84 =
        RackProxy.MSG_NEG_OFFSET - 3;

    public PositionProxy(int id , TimsMbx replyMbx)
    {
        super(RackName.create(RackName.POSITION, id), replyMbx, 500);
    }

    public PositionProxy(int id , TimsMbx replyMbx, TimsMbx dataMbx)
    {
        super(RackName.create(RackName.POSITION, id), replyMbx, dataMbx, 500);
    }

    public synchronized PositionDataMsg getData(int recordingTime)
    {
        try
        {
            TimsRawMsg raw = getRawData(recordingTime);
            
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

    public synchronized void update(Position3d pos, int recordingTime)
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

            TimsRawMsg reply;

            do
            {
                reply = replyMbx.receive(replyTimeout);
                System.out.println(RackName.nameString(replyMbx.getName()) + ": " + reply.type + " " + reply.seqNr);
            }
            while(reply.seqNr != currentSequenceNo);
        }
        catch(TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": " + RackName.nameString(commandMbx) + ".update " + e);
        }
    }

    public synchronized PositionDataMsg wgs84ToPos(PositionWgs84DataMsg posWgs84)
    {
        currentSequenceNo++;

        try {
            replyMbx.send(MSG_POSITION_WGS84_TO_POS,
                               commandMbx,
                               (byte)0,
                               (byte)currentSequenceNo,
                               posWgs84);

            TimsRawMsg reply;

            do
            {
                reply = replyMbx.receive(replyTimeout);
            }
            while((reply.seqNr != currentSequenceNo)
                   & (reply.type == MSG_POSITION_POS));
            
            PositionDataMsg data = new PositionDataMsg(reply);
            return data;
        }
        catch(TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": " + RackName.nameString(commandMbx) + ".wgs84ToPos " + e);
            return null;
        }
    }
    
    public synchronized PositionWgs84DataMsg posToWgs84(PositionDataMsg posData)
    {
        currentSequenceNo++;

        try {
            replyMbx.send(MSG_POSITION_POS_TO_WGS84,
                               commandMbx,
                               (byte)0,
                               (byte)currentSequenceNo,
                               posData);

            TimsRawMsg reply;

            do
            {
                reply = replyMbx.receive(replyTimeout);
            }
            while ((reply.seqNr != currentSequenceNo)
                    & (reply.type == MSG_POSITION_WGS84));
            
            PositionWgs84DataMsg data = new PositionWgs84DataMsg(reply);
            return data;
        }
        catch(TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": " + RackName.nameString(commandMbx) + ".posToWgs84 " + e);
            return null;
        }
    }

    
    public PositionDataMsg readContinuousData(int timeOut) {
        if (dataMbx == null) {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": " + RackName.nameString(commandMbx) + ".readContinuousData: Keine Datenmailbox eingerichtet."  );
            return null;
        }
        try
        {
            TimsRawMsg data = dataMbx.receive(timeOut);
            PositionDataMsg locData = new PositionDataMsg(data);
            System.out.println(RackName.nameString(replyMbx.getName()) + ": " + RackName.nameString(commandMbx) + ".readContinuousData");
            return(locData);
        }
        catch(TimsException e)
        {
//            System.out.println(replyMbx.nameString() + ": " + RackName.nameString(commandMbx) + ".readContinuousData "  + e);
            return(null);
        }

    }
}
