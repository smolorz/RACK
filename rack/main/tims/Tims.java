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
package rack.main.tims;

public abstract class Tims extends Thread
{
    // Tims return types / reply message types
    public static final byte MSG_OK = 0;
    public static final byte MSG_ERROR = -1;
    public static final byte MSG_TIMEOUT = -2;
    public static final byte MSG_NOT_AVAILABLE = -3;

    protected static Tims thisRouter = null;

    protected boolean terminate = false;

    public Tims()
    {
        thisRouter = this;
    }

    public void terminate()
    {
        terminate = true;
        
        if (thisRouter != null)
        {
            thisRouter = null;
        }
    }

    // public message interface

    public static void send(TimsMsg p) throws TimsException
    {
        if (thisRouter == null) {
          throw(new TimsException("No TimsMsgRouter available"));
        }
        thisRouter.snd(p);
    }

    public static void send(byte type, int dest, int src, byte priority,
                            byte seq_nr, TimsMsg p)
                            throws TimsException
    {
      if (thisRouter == null) {
        throw(new TimsException("No TimsMsgRouter available"));
      }

      p.type     = type;
      p.dest     = dest;
      p.src      = src;
      p.priority = priority;
      p.seqNr   = seq_nr;

      thisRouter.snd(p);
    }

    public static void send0(byte type, int dest, int src, byte priority, byte seq_nr)
                       throws TimsException
    {
      if (thisRouter == null) {
        throw(new TimsException("No TimsMsgRouter available"));
      }

      TimsMsg0 p = new TimsMsg0();

      p.type     = type;
      p.dest     = dest;
      p.src      = src;
      p.priority = priority;
      p.seqNr   = seq_nr;

      thisRouter.snd(p);
    }

    public static void sendReply(byte type, TimsMsg replyOn, TimsMsg p)
                       throws TimsException
    {
        if (thisRouter == null) {
            throw(new TimsException("No TimsMsgRouter available"));
        }

      p.type     = type;
      p.dest     = replyOn.src;
      p.src      = replyOn.dest;
      p.priority = replyOn.priority;
      p.seqNr   = replyOn.seqNr;

        thisRouter.snd(p);
    }

    public static void sendReply0(byte type, TimsMsg replyOn)
                       throws TimsException
    {
      if (thisRouter == null) {
        throw(new TimsException("No TimsMsgRouter available"));
      }

      TimsMsg0 p = new TimsMsg0();

      p.type     = type;
      p.dest     = replyOn.src;
      p.src      = replyOn.dest;
      p.priority = replyOn.priority;
      p.seqNr   = replyOn.seqNr;

      thisRouter.snd(p);
    }

    public static TimsDataMsg receive(int mbxName, int timeout)
                              throws TimsException
    {
      if (thisRouter == null) {
        throw(new TimsException("No TimsMsgRouter available"));
      }
      return(thisRouter.rcv(mbxName, timeout));
    }

    public static void mbxInit(int mbxName) throws TimsException
    {
      if (thisRouter == null) {
        throw(new TimsException("No TimsMsgRouter available"));
      }
      thisRouter.init(mbxName);
    }

    public static void mbxDelete(int mbxName) throws TimsException
    {
      if (thisRouter == null) {
        throw(new TimsException("No TimsMsgRouter available"));
      }
      thisRouter.delete(mbxName);
    }

    public static void mbxClean(int mbxName) throws TimsException
    {
      if (thisRouter == null) {
        throw(new TimsException("No TimsMsgRouter available"));
      }
      thisRouter.clean(mbxName);
    }

    public static int getDataRate()
    {
      if (thisRouter == null) {
        return 0;
      }
      return thisRouter.dataRate();
    }

  // end of public message interface

    protected abstract void init(int mbxName) throws TimsException;

    protected abstract void delete(int mbxName) throws TimsException;

    protected abstract void clean(int mbxName) throws TimsException;

    protected abstract void snd(TimsMsg p) throws TimsException;

    protected abstract TimsDataMsg rcv(int mbxName, int timeout) throws TimsException;

    protected abstract int dataRate();

}
