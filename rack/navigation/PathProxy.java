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
package rack.navigation;

import rack.main.*;
import rack.main.tims.*;
import rack.main.RackName;

public class PathProxy extends RackDataProxy
{
    public static final byte MSG_PATH_MAKE =
        RackProxy.MSG_POS_OFFSET + 1;

    public static final byte MSG_PATH_REPLAN =
        RackProxy.MSG_POS_OFFSET + 2;

    public static final byte MSG_PATH_SET_DESTINATION =
        RackProxy.MSG_POS_OFFSET + 3;

    public static final byte MSG_PATH_SET_RDDF =
        RackProxy.MSG_POS_OFFSET + 4;

    public static final byte MSG_PATH_GET_DBG_GRIDMAP =
        RackProxy.MSG_POS_OFFSET + 5;

    public static final byte MSG_PATH_GET_LAYER =
        RackProxy.MSG_POS_OFFSET + 6;

    public static final byte MSG_PATH_SET_LAYER =
        RackProxy.MSG_POS_OFFSET + 7;

    public static final byte MSG_PATH_DBG_GRIDMAP =
        RackProxy.MSG_NEG_OFFSET - 1;

    public static final byte MSG_PATH_LAYER =
        RackProxy.MSG_NEG_OFFSET - 2;


    public PathProxy(int system, int instance, TimsMbx replyMbx)
    {
      super(RackName.create(system, RackName.PATH, instance, 0), replyMbx, 5000);
    }

    public synchronized PathDataMsg getData(int recordingTime)
    {
      try
      {
          TimsRawMsg raw = getRawData(recordingTime);

          if (raw != null)
          {
              PathDataMsg data = new PathDataMsg(raw);
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

    public synchronized PathDataMsg getData()
    {
      return(getData(0));
    }

    public synchronized void setDestination(PathDestMsg dest)
    {
        currentSequenceNo++;

        try
        {
            replyMbx.send(MSG_PATH_SET_DESTINATION, commandMbx,
                               (byte)0, currentSequenceNo, dest);

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
                               ".setDestination " + e);
        }
    }

    public synchronized void setRddf(PathRddfMsg rddf)
    {
        currentSequenceNo++;
        try
        {
            replyMbx.send(MSG_PATH_SET_RDDF, commandMbx,
                               (byte)0, currentSequenceNo, rddf);

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
                               ".setRddf " + e);
        }
    }

    public synchronized void make()
    {
    	PathMakeMsg makeMsg = new PathMakeMsg();

    	// use default values
    	makeMsg.vMax = -1;
    	makeMsg.accMax = -1;
        makeMsg.decMax = -1;
    	makeMsg.omegaMax = -1.0f;

    	make(makeMsg);
    }

    public synchronized void make(PathMakeMsg make)
    {
        currentSequenceNo++;

        try
        {
            replyMbx.send(MSG_PATH_MAKE, commandMbx,
                               (byte)0, currentSequenceNo, make);

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
                               ".make " + e);
        }
    }

    public synchronized void setLayer(PathLayerMsg layer)
    {
        currentSequenceNo++;

        try
        {
            replyMbx.send(MSG_PATH_SET_LAYER, commandMbx,
                          (byte)0, currentSequenceNo, layer);

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
                               ".layer " + e);
        }
    }

    public synchronized PathLayerMsg getLayer()
    {
        currentSequenceNo++;
        try
        {
            replyMbx.send0(MSG_PATH_GET_LAYER, commandMbx,
                    (byte) 0, currentSequenceNo);

            TimsRawMsg reply;
            do
            {
                reply = replyMbx.receive(replyTimeout);
            }
            while ((reply.seqNr != currentSequenceNo)
                    & (reply.type == MSG_PATH_LAYER));

            PathLayerMsg data = new PathLayerMsg(reply);
            return (data);
        }
        catch (TimsException e)
        {
        	System.out.println(e);
            return (null);
        }
    }

    public void createPath(PathDestMsg dest)
    {
        setDestination(dest);
        make();
    }

    public void createPath()
    {
        make();
    }
}
