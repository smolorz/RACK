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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
package rack.navigation;

import rack.main.*;
import rack.main.defines.Position3d;
import rack.main.tims.*;
import rack.navigation.PilotDestMsg;
import rack.navigation.PilotMultiDestMsg;
import rack.navigation.PilotHoldMsg;

public class PilotProxy extends RackDataProxy
{
  public static final byte MSG_PILOT_SET_DESTINATION = RackProxy.MSG_POS_OFFSET + 1;
  public static final byte MSG_PILOT_HOLD_COMMAND = RackProxy.MSG_POS_OFFSET + 2;
  public static final byte MSG_PILOT_SET_MULTI_DESTINATION = RackProxy.MSG_POS_OFFSET + 6;

  public static final byte PILOT_HOLD_ENABLED = 1;
  public static final byte PILOT_HOLD_DISABLED = 0;

  public static final byte PILOT_STATE_IDLE = 0;
  public static final byte PILOT_STATE_PLANNING = 1;
  public static final byte PILOT_STATE_RUNNING = 2;
  public static final byte PILOT_STATE_PATH_ERROR = 3;

  public PilotProxy(int system, int instance, TimsMbx replyMbx)
  {
    super(RackName.create(system, RackName.PILOT, instance, 0), replyMbx, 500);
  }

  public synchronized PilotDataMsg getData(int recordingTime)
  {
    try {
      TimsRawMsg raw = getRawData(recordingTime);

      if (raw != null) {
        PilotDataMsg data = new PilotDataMsg(raw);
        return(data);
      } else {
        return(null);
      }
    } catch(TimsException e) {
      System.out.println(e.toString());
      return(null);
    }
  }

  public synchronized PilotDataMsg getData()
  {
    return(getData(0));
  }
  
  public synchronized void setDestination(Position3d pos)
  {
      try {
    	  PilotDestMsg destinationMsg = new PilotDestMsg();
    	  destinationMsg.pos   = pos;
    	  destinationMsg.speed = 1;
          replyMbx.send(MSG_PILOT_SET_DESTINATION, commandMbx,(byte)0,(byte)0, destinationMsg);
          replyMbx.receive(0);
 
          System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                  + RackName.nameString(commandMbx) + ".setDestination position "
                  + pos);
      }
      catch(TimsException e)
      {
          System.out.println(RackName.nameString(replyMbx.getName()) + ": " + RackName.nameString(commandMbx) + ".setEstimate " + e);
      }
  }
  
  public synchronized void setDestination(PilotDestMsg dest)
  {
      currentSequenceNo++;

      try
      {
          replyMbx.send(MSG_PILOT_SET_DESTINATION, commandMbx,
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
  
  public synchronized void setMultiDestination(PilotMultiDestMsg dest)
  {
      currentSequenceNo++;
      try
      {
          replyMbx.send(MSG_PILOT_SET_MULTI_DESTINATION, commandMbx,
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
                             ".setMultiDestination " + e);
      }
  }
  
  public synchronized void holdCommand(PilotHoldMsg hold)
  {
      currentSequenceNo++;

      try
      {
          replyMbx.send(MSG_PILOT_HOLD_COMMAND, commandMbx,
                             (byte)0, currentSequenceNo, hold);

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
                             ".holdCommand " + e);
      }
  }  
  
  
  
}
