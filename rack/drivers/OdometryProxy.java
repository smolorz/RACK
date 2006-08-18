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
package rack.drivers;

import rack.main.naming.*;
import rack.main.proxy.*;
import rack.main.tims.Tims;
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;

public class OdometryProxy extends RackDataProxy
{
    public static final byte MSG_ODOMETRY_RESET =
        RackMsgType.RACK_PROXY_MSG_POS_OFFSET + 1;

  public OdometryProxy(int id, int replyMbx)
  {
    super(RackName.create(RackName.ODOMETRY, id), replyMbx, 5000, 1000, 1000);
    this.id = id;
  }

  public synchronized OdometryDataMsg getData(int recordingtime)
  {
    try {
      TimsDataMsg raw = getRawData(recordingtime);
      if (raw != null) {
        OdometryDataMsg data = new OdometryDataMsg(raw);
        return(data);
      } else {
        return(null);
      }
    } catch(MsgException e) {
      System.out.println(e.toString());
      return(null);
    }
  }

  public synchronized OdometryDataMsg getData()
  {
    return(getData(0));
  }

    public synchronized void reset()
    {
      currentSequenceNo++;
      try {
        Tims.send0(MSG_ODOMETRY_RESET, commandMbx, replyMbx,
                            (byte)0, currentSequenceNo);
        TimsDataMsg reply;
        do {
          reply = Tims.receive(replyMbx, onTimeout);
        } while (reply.seqNr != currentSequenceNo);

        if (reply.type == RackMsgType.MSG_OK) {
          System.out.println(RackName.nameString(replyMbx) + ": " +
                             RackName.nameString(commandMbx) + ".enableMotor");
        } else {
          System.out.println(RackName.nameString(replyMbx) + ": " +
                             RackName.nameString(commandMbx) + ".enableMotor replied error");
        }
      } catch(MsgException e) {
        System.out.println(RackName.nameString(replyMbx) + ": " +
                           RackName.nameString(commandMbx) + ".on " + e);
      }
    }

    public int getCommandMbx()
    {
      return(RackName.create(RackName.ODOMETRY, id));
    }


}
