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

public class PilotProxy extends RackDataProxy
{
  public PilotProxy(int id, int replyMbx)
  {
    super(RackName.create(RackName.PILOT, id), replyMbx, 5000, 1000, 1000);
    this.id = id;
  }

  public int getCommandMbx()
  {
    return(RackName.create(RackName.PILOT, id));
  }

  public synchronized PilotDataMsg getData(int recordingtime)
  {
    try {
      TimsDataMsg raw = getRawData(recordingtime);

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
}
