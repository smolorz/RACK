/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2007 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Marko Reimer  <reimer@rts.uni-hannover.de>
 *
 */
package rack.perception;

import rack.main.*;
import rack.main.tims.*;
import rack.navigation.PositionDataMsg;
import rack.main.RackName;

public class ObjRecogProxy extends RackDataProxy
{
  public static final byte MSG_SET_ESTIMATE =
        RackProxy.MSG_POS_OFFSET + 1;
    
  public ObjRecogProxy(int id, TimsMbx replyMbx)
  {
    super(RackName.create(RackName.OBJ_RECOG, id), replyMbx, 2500);
  }

  public synchronized ObjRecogDataMsg getData(int recordingTime)
  {
    try {
      TimsRawMsg raw = getRawData(recordingTime);
      if (raw != null) {
          ObjRecogDataMsg data = new ObjRecogDataMsg(raw);
        return data;
      } else {
        return null;
      }
    } catch(TimsException e) {
      System.out.println(e.toString());
      return(null);
    }
  }

  public synchronized ObjRecogDataMsg getData()
  {
    try {
      TimsRawMsg raw = getNextData();
      if (raw != null) {
          ObjRecogDataMsg data = new ObjRecogDataMsg(raw);
        return data;
      } else {
        return null;
      }
    } catch(TimsException e) {
      System.out.println(e.toString());
      return(null);
    }
  } 
  
  public synchronized void setEstimate(ObjRecogDataMsg objEstimateData, int recordingTime)
  {   
      currentSequenceNo++;
      try
      {
          PositionDataMsg updateMsg = new PositionDataMsg();
          updateMsg.recordingTime = recordingTime;        
          objEstimateData.recordingTime = recordingTime;

          replyMbx.send(MSG_SET_ESTIMATE,
                  commandMbx,
                  (byte)0,
                  currentSequenceNo,
                  objEstimateData);
      
          TimsRawMsg reply;
          do
          {
              reply = replyMbx.receive(replyTimeout);
          }
          while(reply.seqNr != currentSequenceNo);

          if (reply.type == RackProxy.MSG_OK)
          {
              System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                      + RackName.nameString(commandMbx) + ".setEstimate");
          }
          else
          {
              System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                      + RackName.nameString(commandMbx)
                      + ".setEstimate replied error");
          }            
      }
      catch (TimsException e)
      {
          System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                  + RackName.nameString(commandMbx) + ".setEstimate " + e);
      } 
  }  
}
