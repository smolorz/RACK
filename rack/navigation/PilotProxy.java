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
import rack.main.defines.Position3D;
import rack.main.tims.*;
import rack.navigation.PilotDestMsg;

public class PilotProxy extends RackDataProxy
{
  public static final byte MSG_PILOT_SET_DESTINATION = RackProxy.MSG_POS_OFFSET + 1;
	  
  public PilotProxy(int id, TimsMbx replyMbx)
  {
    super(RackName.create(RackName.PILOT, id), replyMbx, 5000, 1000, 1000);
    this.id = id;
  }

  public int getCommandMbx()
  {
    return(RackName.create(RackName.PILOT, id));
  }

  public synchronized PilotDataMsg getData(int recordingTime)
  {
    try {
      TimsDataMsg raw = getRawData(recordingTime);

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
  
  public synchronized void setDestination(Position3D pos)
  {
      try {
    	  PilotDestMsg destinationMsg = new PilotDestMsg();
    	  destinationMsg.pos = pos;
    	  destinationMsg.moveDir = 0.0f;
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

          TimsDataMsg reply;
          do
          {
            reply = replyMbx.receive(1000);
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
}
