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

public class MclProxy extends RackDataProxy
{
    public static final byte MSG_MCL_LOAD_MAP =
        RackProxy.MSG_POS_OFFSET + 1;


    public MclProxy(int system, int instance, TimsMbx replyMbx)
    {
      super(RackName.create(system, RackName.MCL, instance, 0), replyMbx, 2500);
    }

    public synchronized MclDataMsg getData(int recordingTime)
    {
      try
      {
          TimsRawMsg raw = getRawData(recordingTime);

          if (raw != null)
          {
              MclDataMsg data = new MclDataMsg(raw);
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

    public synchronized MclDataMsg getData()
    {
        return(getData(0));
    }

    public synchronized void loadMap(MclFilenameMsg filename)
    {
        currentSequenceNo++;

        try
        {
            replyMbx.send(MSG_MCL_LOAD_MAP, commandMbx,
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
                               RackName.nameString(commandMbx) + ".loadMap " + e);
        }
    }
}
