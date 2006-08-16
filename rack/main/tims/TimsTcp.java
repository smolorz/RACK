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

import java.net.*;
import java.io.*;
import java.util.*;

import rack.main.naming.*;
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.router.TimsRouter;
import rack.main.tims.router.TimsRouterMbxMsg;
import rack.main.tims.exceptions.*;

public class TimsTcp extends Tims
{
    protected Socket                socket = null;
    protected InputStream           tcpIn = null;
    protected BufferedOutputStream  tcpOut = null;
    protected Object                dataCountSync = new Object();
    protected Vector                mbxList = new Vector();
    protected InetAddress           addr;
    protected int                   port;
    protected int                   dataCount;
    protected long                  dataCountTime;


    public TimsTcp(InetAddress addr, int port) throws MsgException
    {
      this.addr = addr;
      this.port = port;

      try {
        socket = new Socket(addr, port);  // open socket connection to TimsMsgGateway
        socket.setSoTimeout(10000);
        socket.setTcpNoDelay(true);
        tcpOut = new BufferedOutputStream(socket.getOutputStream());
        tcpIn  = socket.getInputStream();

        // init MBX 0 for communication between Tims and TimsRouter
        TimsMbx mbx = new TimsMbx();
        mbx.name = 0;
        mbxList.addElement(mbx);

        thisRouter = this;
        start();

      } catch(IOException e) {

        thisRouter = null;
        tcpOut = null;
        tcpIn  = null;

        if (socket != null) {
          try {
            socket.close();
          } catch(IOException ee) {}

          socket = null;
        }
        throw(new MsgIOException("Can't connect to TimsRouter. " + e.toString()));
      }
    }

    public synchronized void snd(TimsMsg p) throws MsgException
    {
//    System.out.println(/*"TimsRouter " + */p);

      BufferedOutputStream out = tcpOut;

      if (out == null) {
        throw(new MsgIOException("No connection to TimsRouter"));
      }

      try {
        p.writeTimsMsg(out);
      } catch(IOException e) {
        throw(new MsgIOException("Can't send message to TimsRouter. " + e.toString()));
      }
    }

    public TimsDataMsg rcv(int mbxName, int timeout) throws MsgException
    {
      TimsDataMsg p;
      TimsMbx mbx = getMbx(mbxName);

      if (mbx != null) {

        synchronized(mbx) {

          if (mbx.isEmpty()) {
            try {
              mbx.wait(timeout);
            } catch(InterruptedException e) {}
          }

          if (mbx.isEmpty()) {
            throw(new MsgTimeoutException("Receive timeout"));
          } else {
            p = (TimsDataMsg)mbx.remove(0);
            return p;
          }

        }

      } else {
        throw(new MsgMbxException("Unknown mbx " + RackName.string(mbxName)));
      }

    }

    public void run()
    {
      TimsDataMsg p;
      TimsMbx     mbx;
      InputStream in   = tcpIn;
      Socket      sock = socket;

      while (terminate == false) {

        // normal operation
        try {

          while(in != null) {
            p = new TimsDataMsg(in);

            if ((p.dest == 0) && (p.src == 0) &&
                (p.type == TimsRouter.GET_STATUS)) {

              // reply to lifesign
              try {
                sendReply0(RackMsgType.MSG_OK, p);
              } catch (MsgException e1) {}

            } else {
              synchronized(dataCountSync) {
                dataCount += p.getDataLen();
              }

              mbx = getMbx(p.dest);
              if (mbx != null) {
                synchronized(mbx) {
                  mbx.addElement(p);
                  mbx.notifyAll();
                }
              } else {
                System.out.println("Tims received message for unknown mbx " + p.dest);
              }
            }
            in   = tcpIn;
            sock = socket;
          }
        }  catch(IOException e) {
          System.out.println("Tims " + e.getMessage());

          try {
            if (sock != null)
              sock.close();
            socket = null;
            tcpIn  = null;
            tcpOut = null;
          } catch (IOException e1) {}

        } catch (Throwable t) {
          System.out.println("Tims " + t);
          t.printStackTrace();
        }

        // try to reconnect to TimsRouterTcp
        if (tcpIn == null) {
          synchronized(mbxList) {
            try {
              socket = new Socket(addr, port);  // open socket connection to TimsMsgGateway
              socket.setSoTimeout(10000);
              socket.setTcpNoDelay(true);
              tcpOut = new BufferedOutputStream(socket.getOutputStream());
              tcpIn  = socket.getInputStream();

              TimsRouterMbxMsg initMbxM = new TimsRouterMbxMsg();

              for(int i = 0; i < mbxList.size(); i++) {
                initMbxM.mbx = ((TimsMbx)mbxList.elementAt(i)).name;

                send(TimsRouter.MBX_INIT, 0, 0, (byte)0, (byte)0, initMbxM);
              }

              System.out.println("Tims reconnected to " + addr.getHostAddress() + ":" + port);

            } catch(Exception e) {

              System.out.println("Tims " + e.getMessage());

              tcpOut = null;
              tcpIn  = null;

              if (socket != null) {
                try {
                  socket.close();
                } catch(IOException ee) {}

                socket = null;
              }
            }

            in   = tcpIn;
            sock = socket;

            try {
              sleep(1000);
            } catch (InterruptedException e1) {}
          }
        }
      }
      System.out.println("TimsTcp terminated");
    }

    public void terminate()
    {
        super.terminate();
        
        // close connection to router to terminate

        tcpIn  = null;
        tcpOut = null;

        if (socket != null)
        {
            try
            {
                socket.close();
            }
            catch (IOException e) {}
        }
        
        socket = null;
        
        try
        {
            this.interrupt();
            this.join(1000);
        }
        catch (Exception e) {}
    }
    
    public synchronized void init(int mbxName) throws MsgException
    {
      TimsMbx mbx;
      TimsRouterMbxMsg p = new TimsRouterMbxMsg();
      TimsDataMsg reply;

      mbx = getMbx(mbxName);

      if (mbx == null) {
        p.mbx = mbxName;
        mbxClean(0);

        send(TimsRouter.MBX_INIT_WITH_REPLY, 0, 0, (byte)0, (byte)0, p);

        reply = receive(0, 1000);

        if ((reply != null) &&
            (reply.type == RackMsgType.MSG_OK)) {

          synchronized(mbxList) {
            mbx = new TimsMbx();
            mbx.name = mbxName;
            mbxList.addElement(mbx);
          }

        } else {

          throw(new MsgMbxException("Can't init mbx " +
                                        RackName.string(mbxName) +
                                        ". Allready initialised"));
        }
      } else {

        throw(new MsgMbxException("Can't init mbx " +
                                      RackName.string(mbxName) +
                                      ". Allready initialised"));
      }
    }

    public synchronized void delete(int mbxName) throws MsgException
    {
      TimsMbx mbx;
      TimsRouterMbxMsg p = new TimsRouterMbxMsg();
      TimsDataMsg reply;

      mbx = getMbx(mbxName);
      if (mbx != null) {
        p.mbx = mbxName;
        mbxClean(0);

        send(TimsRouter.MBX_DELETE_WITH_REPLY, 0, 0, (byte)0, (byte)0, p);
        reply = receive(0, 1000);

        if ((reply != null) && (reply.type == RackMsgType.MSG_OK)) {
          synchronized (mbxList) {
            mbxList.removeElement(mbx);
//          System.out.println("remove from mbxList: mbx "+ mbxName);
          }
        } else {
          throw ( new MsgMbxException("Can't delete mbx " +
                                          RackName.string(mbxName) +
                                          ". not initialised"));
        }
      } else {
        throw(new MsgMbxException("Can't delete mbx " +
                                      RackName.string(mbxName) +
                                      ". Not initialised"));
      }
    }

    public synchronized void clean(int mbxName) throws MsgException
    {
      TimsMbx mbx;
      mbx = getMbx(mbxName);
      if (mbx != null) {
        synchronized(mbx) {
          mbx.clear();
          mbx.notifyAll();
        }
      } else {
        throw(new MsgMbxException("Can't clear mbx " +
                                      RackName.string(mbxName) +
                                      ". Not initialised"));
      }
    }

    public int dataRate()
    {
      int dataRate, deltaT;
      long newTime;

      synchronized(dataCountSync) {
        newTime = System.currentTimeMillis();
        deltaT = (int)(newTime - dataCountTime);
        dataRate = 1000 * dataCount / deltaT / 1024;
        dataCount = 0;
        dataCountTime = newTime;
      }
      return dataRate;
    }

    protected TimsMbx getMbx(int mbxName)
    {
      TimsMbx mbx;

      synchronized(mbxList) {
        for (int i = 0; i < mbxList.size(); i++) {
          mbx = (TimsMbx)(mbxList.elementAt(i));
          if (mbx.name == mbxName) {
            return mbx;
          }
        }
      }
      return null;
    }

}
