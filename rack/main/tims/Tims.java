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

import rack.main.tims.msg.*;
import rack.main.tims.exceptions.*;

public abstract class Tims extends Thread
{

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

    public static void send(TimsMsg p) throws MsgException
    {
        if (thisRouter == null) {
          throw(new MsgIOException("No TimsMsgRouter available"));
        }
        thisRouter.snd(p);
    }

    public static void send(byte type, int dest, int src, byte priority,
                            byte seq_nr, TimsMsg p)
                            throws MsgException
    {
      if (thisRouter == null) {
        throw(new MsgIOException("No TimsMsgRouter available"));
      }

      p.type     = type;
      p.dest     = dest;
      p.src      = src;
      p.priority = priority;
      p.seq_nr   = seq_nr;

      thisRouter.snd(p);
    }

    public static void send0(byte type, int dest, int src, byte priority, byte seq_nr)
                       throws MsgException
    {
      if (thisRouter == null) {
        throw(new MsgIOException("No TimsMsgRouter available"));
      }

      TimsMsg0 p = new TimsMsg0();

      p.type     = type;
      p.dest     = dest;
      p.src      = src;
      p.priority = priority;
      p.seq_nr   = seq_nr;

      thisRouter.snd(p);
    }

    public static void sendReply(byte type, TimsMsg replyOn, TimsMsg p)
                       throws MsgException
    {
        if (thisRouter == null) {
            throw(new MsgIOException("No TimsMsgRouter available"));
        }

      p.type     = type;
      p.dest     = replyOn.src;
      p.src      = replyOn.dest;
      p.priority = replyOn.priority;
      p.seq_nr   = replyOn.seq_nr;

        thisRouter.snd(p);
    }

    public static void sendReply0(byte type, TimsMsg replyOn)
                       throws MsgException
    {
      if (thisRouter == null) {
        throw(new MsgIOException("No TimsMsgRouter available"));
      }

      TimsMsg0 p = new TimsMsg0();

      p.type     = type;
      p.dest     = replyOn.src;
      p.src      = replyOn.dest;
      p.priority = replyOn.priority;
      p.seq_nr   = replyOn.seq_nr;

      thisRouter.snd(p);
    }

    public static TimsDataMsg receive(int mbxName, int timeout)
                              throws MsgException
    {
      if (thisRouter == null) {
        throw(new MsgIOException("No TimsMsgRouter available"));
      }
      return(thisRouter.rcv(mbxName, timeout));
    }

    public static void mbxInit(int mbxName) throws MsgException
    {
      if (thisRouter == null) {
        throw(new MsgIOException("No TimsMsgRouter available"));
      }
      thisRouter.init(mbxName);
    }

    public static void mbxDelete(int mbxName) throws MsgException
    {
      if (thisRouter == null) {
        throw(new MsgIOException("No TimsMsgRouter available"));
      }
      thisRouter.delete(mbxName);
    }

    public static void mbxClean(int mbxName) throws MsgException
    {
      if (thisRouter == null) {
        throw(new MsgIOException("No TimsMsgRouter available"));
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

    protected abstract void init(int mbxName) throws MsgException;

    protected abstract void delete(int mbxName) throws MsgException;

    protected abstract void clean(int mbxName) throws MsgException;

    protected abstract void snd(TimsMsg p) throws MsgException;

    protected abstract TimsDataMsg rcv(int mbxName, int timeout) throws MsgException;

    protected abstract int dataRate();

}
