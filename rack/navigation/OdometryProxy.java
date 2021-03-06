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
 *      Oliver Wulf  <oliver.wulf@gmx.de>
 *
 */
package rack.navigation;

import rack.main.*;
import rack.main.tims.*;

public class OdometryProxy extends RackDataProxy
{
    public static final byte MSG_ODOMETRY_RESET =
        RackProxy.MSG_POS_OFFSET + 1;

  public OdometryProxy(int system, int instance, TimsMbx replyMbx)
  {
    super(RackName.create(system, RackName.ODOMETRY, instance, 0), replyMbx, 500);
  }

  public synchronized OdometryDataMsg getData(int recordingTime)
  {
    try {
      TimsRawMsg raw = getRawData(recordingTime);
      if (raw != null) {
        OdometryDataMsg data = new OdometryDataMsg(raw);
        return(data);
      } else {
        return(null);
      }
    } catch(TimsException e) {
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
          replyMbx.send0(MSG_ODOMETRY_RESET, commandMbx,
                            (byte)0, currentSequenceNo);
        TimsRawMsg reply;
        do {
          reply = replyMbx.receive(replyTimeout);
        } while (reply.seqNr != currentSequenceNo);

        if (reply.type == RackProxy.MSG_OK) {
          System.out.println(RackName.nameString(replyMbx.getName()) + ": " +
                             RackName.nameString(commandMbx) + ".reset");
        } else {
          System.out.println(RackName.nameString(replyMbx.getName()) + ": " +
                             RackName.nameString(commandMbx) + ".reset replied error");
        }
      } catch(TimsException e) {
        System.out.println(RackName.nameString(replyMbx.getName()) + ": " +
                           RackName.nameString(commandMbx) + ".on " + e);
      }
    }
}
