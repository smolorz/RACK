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
 *      Oliver Wulf <oliver.wulf@gmx.de>
 *
 */
package rack.navigation;

import rack.main.*;
import rack.main.tims.*;
import rack.main.RackName;

public class FeatureMapProxy extends RackDataProxy
{
    public static final byte MSG_FEATURE_MAP_LOAD_MAP      = RackProxy.MSG_POS_OFFSET + 1;
    public static final byte MSG_FEATURE_MAP_ADD_LINE      = RackProxy.MSG_POS_OFFSET + 2;
    public static final byte MSG_FEATURE_MAP_SAVE_MAP      = RackProxy.MSG_POS_OFFSET + 3;
    public static final byte MSG_FEATURE_MAP_GIVE_MAP      = RackProxy.MSG_POS_OFFSET + 4;
    public static final byte MSG_FEATURE_MAP_DELETE_LINE   = RackProxy.MSG_POS_OFFSET + 5;
    public static final byte MSG_FEATURE_MAP_DISPLACE_LINE = RackProxy.MSG_POS_OFFSET + 6;

    public FeatureMapProxy(int system, int instance, TimsMbx replyMbx)
    {
      super(RackName.create(system, RackName.FEATURE_MAP, instance, 0), replyMbx, 1000);
    }

    public synchronized FeatureMapDataMsg getData(int recordingTime)
    {
      try
      {

          TimsRawMsg raw = getRawData(recordingTime);

          if (raw != null)
          {
              FeatureMapDataMsg data = new FeatureMapDataMsg(raw);
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

    public synchronized FeatureMapDataMsg getData()
    {
    	return(getData(0));
    }

    public synchronized void loadMap(FeatureMapFilenameMsg filename)
    {
        currentSequenceNo++;

        try
        {
            replyMbx.send(MSG_FEATURE_MAP_LOAD_MAP, commandMbx,
                               (byte)0, currentSequenceNo, filename);

            TimsRawMsg reply;
            do
            {
              reply = replyMbx.receive(replyTimeout);
            }
            while(reply.seqNr != currentSequenceNo);
            System.out.println("Map loaded!");
        }
        catch(TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": " +
                               RackName.nameString(commandMbx) + ".loadMap " + e);
        }
    }

    public synchronized void addLine(FeatureMapFeatureMsg feature)
    {
        currentSequenceNo++;
        try
        {
    		replyMbx.send(MSG_FEATURE_MAP_ADD_LINE, commandMbx,
                    (byte)0, currentSequenceNo, feature);

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
                                   ".addLine " + e);
        }
    }
    public synchronized void saveMap(FeatureMapFilenameMsg filename)
    {
        currentSequenceNo++;

        try
        {
            replyMbx.send(MSG_FEATURE_MAP_SAVE_MAP, commandMbx,
                               (byte)0, currentSequenceNo, filename);

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
                               RackName.nameString(commandMbx) + ".saveMap " + e);
        }
    }
    public synchronized void deleteLine(FeatureMapFeatureMsg feature)
    {
        currentSequenceNo++;
        try
        {
    		replyMbx.send(MSG_FEATURE_MAP_DELETE_LINE, commandMbx,
                    (byte)0, currentSequenceNo, feature);

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
                                   ".deleteLine " + e);
        }
    }
    public synchronized void displaceLine(FeatureMapFeatureMsg feature)
    {
        currentSequenceNo++;
        try
        {
    		replyMbx.send(MSG_FEATURE_MAP_DISPLACE_LINE, commandMbx,
                    (byte)0, currentSequenceNo, feature);

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
                                   ".displaceLine " + e);
        }
    }
}
