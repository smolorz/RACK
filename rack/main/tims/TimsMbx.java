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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
package rack.main.tims;

import java.util.*;

public class TimsMbx extends Vector
{
  private static final long serialVersionUID = 1L;

  protected int     name = 0;
  protected Tims    tims;

  /**
   * Internal constructor.
   * 
   * Use Tims.mbxInit to create an instance
   * 
   * @see Tims#mbxInit(int)
   */
  TimsMbx(int mbxName, Tims tims)
  {
      this.name = mbxName;
      this.tims = tims;
  }

  public int getName()
  {
      return name;
  }

  public void send(TimsMsg m) throws TimsException
  {
      m.src      = name;

      tims.send(m);
  }

  public void send(byte type, int dest, byte priority,
                          byte seqNr, TimsMsg m)
                          throws TimsException
  {
    m.type     = type;
    m.dest     = dest;
    m.src      = name;
    m.priority = priority;
    m.seqNr    = seqNr;

    tims.send(m);
  }

  public void send0(byte type, int dest, byte priority, byte seqNr)
                     throws TimsException
  {
    TimsDataMsg m = new TimsDataMsg();

    m.type     = type;
    m.dest     = dest;
    m.src      = name;
    m.priority = priority;
    m.seqNr    = seqNr;

    tims.send(m);
  }

  public void sendReply(byte type, TimsMsg replyOn, TimsMsg m)
                     throws TimsException
  {
    m.type     = type;
    m.dest     = replyOn.src;
    m.src      = name;
    m.priority = replyOn.priority;
    m.seqNr    = replyOn.seqNr;

    tims.send(m);
  }

  public void sendReply0(byte type, TimsMsg replyOn)
                     throws TimsException
  {
    TimsDataMsg m = new TimsDataMsg();

    m.type     = type;
    m.dest     = replyOn.src;
    m.src      = name;
    m.priority = replyOn.priority;
    m.seqNr    = replyOn.seqNr;

    tims.send(m);
  }

  public TimsDataMsg receive(int timeout)
                            throws TimsException
  {
    return tims.receive(this, timeout);
  }

  /**
   * TimsMbxs are equal if the mbxNames are equal.
   *
   * @param o TimsMbx to compare to.
   * @return true if the specified Object is a TimsMbx with the same name
   */
  public synchronized boolean equals(Object o)
  {
    try
    {
        return name == ((TimsMbx) o).name;
    }
    catch (java.lang.ClassCastException e)
    {
        return false;
    }
  }
}
