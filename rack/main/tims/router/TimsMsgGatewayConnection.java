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
package rack.main.tims.router;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.util.Enumeration;
import java.util.Hashtable;

import rack.main.naming.*;
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;

/**
 * TimsMsgGatewayConnection: jede verbindung zum Gateway bekommt
 * eine eigen Connection
 * @version $Id: TimsMsgGatewayConnection.java,v 1.12 2003/11/27 08:10:02 amichael Exp $
 */

class TimsMsgGatewayConnection extends Thread {

  protected InetAddress           ip;
  protected int                   port;
  protected InputStream           tcpIn;
  protected BufferedOutputStream  tcpOut;
  protected Hashtable             mbxList;
  private   int                   connectionCount;
  public    String                connectionListRepresentation;
  private   String                poutLeader = "TimsMsgGateway ";


  protected TimsMsgGatewayConnection(Socket socket, Hashtable mbxList)
    throws IOException {
    TimsMsgGateway.connectionCount++;
    this.connectionCount = TimsMsgGateway.connectionCount;
    this.mbxList = mbxList;

    tcpIn  = socket.getInputStream();
    tcpOut = new BufferedOutputStream(socket.getOutputStream());
    ip     = socket.getInetAddress();
    port   = socket.getPort();

    TimsMsgGateway.connectionVec.addElement(this);
    if (TimsMsgGateway.guiMode) {
      poutLeader = "";
      connectionListRepresentation = connectionCount + " " + ip.toString();
      TimsMsgGateway.connectionListModel.addElement(connectionListRepresentation);
    }
  }

  public synchronized void send(TimsMsg p) {
    try {
      p.writeTimsMsg(tcpOut);
    } catch (IOException e) {
      System.out.println(e.toString());
    }
  }

  public synchronized boolean isConnectionAlive() {
    return (true);
  }

  public void run() {
    TimsDataMsg p;
    TimsMsgGatewayConnection toConnection;

    System.out.println(buildMsg("login ip " + ip.getHostAddress() + " port " + port));
    try {
      while (true) {
        p = new TimsDataMsg(tcpIn);

        if ((p.dest == 0) & (p.src == 0)) {
          switch (p.type) {

            case RackMsgType.MSG_GET_STATUS :
              //System.out.println(
              //  "TimsMsgGateway Con. "
              //    + connectionCount
              //    + " send GW-status ENABLED to "
              //    + RackName.string(p.from));
              TimsMsg0 reply2  = new TimsMsg0();
              reply2.type      = RackMsgType.MSG_ENABLED;
              reply2.dest      = p.src;
              reply2.src       = p.dest;
              reply2.priority  = p.priority;
              reply2.seq_nr    = p.seq_nr;
              send(reply2);
              break;

              //                    case TimsMsgGateway.LOGIN:
              //                        break;

            case TimsMsgGateway.MBX_INIT :
              try {
                synchronized (mbxList) {
                  TimsRouterMbxMsg initM = new TimsRouterMbxMsg(p);

                  if (mbxList.get(new Integer(initM.mbx)) == null) {
                    mbxList.put( new Integer(initM.mbx), this);

                    System.out.println(buildMsg("mbx init " +
                                       RackName.string(initM.mbx)));
                  } else {
                    System.out.println(buildMsg("can't init mbx " +
                                       RackName.string(initM.mbx)));
                  }
                }
              } catch (MsgException e) {
                System.out.println(e.toString());
              }
              break;

            case TimsMsgGateway.MBX_INIT_WITH_REPLY :
              try {
                synchronized (mbxList) {
                  TimsRouterMbxMsg initM = new TimsRouterMbxMsg(p);

                  TimsMsg0 reply  = new TimsMsg0();
                  reply.dest      = p.src;
                  reply.src       = p.dest;
                  reply.priority  = p.priority;
                  reply.seq_nr    = p.seq_nr;

                  if (mbxList.get(new Integer(initM.mbx)) == null) {
                    mbxList.put(new Integer(initM.mbx), this);

                    reply.type = RackMsgType.MSG_OK;
                    send(reply);

                    System.out.println(buildMsg("mbx init " +
                                       RackName.string(initM.mbx)));
                  } else {
                    reply.type = RackMsgType.MSG_ERROR;
                    send(reply);

                    System.out.println(buildMsg("can't init mbx " +
                                       RackName.string(initM.mbx)));
                  }
                }
              } catch (MsgException e) {
                System.out.println(e.toString());
              }
              break;

            case TimsMsgGateway.MBX_DELETE_WITH_REPLY :
              try {
                synchronized (mbxList) {
                  TimsRouterMbxMsg delM = new TimsRouterMbxMsg(p);
                  Integer key = new Integer(delM.mbx);

                  TimsMsg0 reply = new TimsMsg0();
                  reply.dest     = p.src;
                  reply.src      = p.dest;
                  reply.priority = p.priority;
                  reply.seq_nr   = p.seq_nr;

                  if (mbxList.get(key) == this) {
                    mbxList.remove(key);
                    reply.type = RackMsgType.MSG_OK;
                    send(reply);

                    System.out.println(buildMsg("mbx delete " +
                                       RackName.string(key.intValue())));
                  } else {
                    reply.type = RackMsgType.MSG_ERROR;
                    send(reply);

                    System.out.println(buildMsg("mbx delete Error: " +
                                       RackName.string(key.intValue()) +
                                       " not in list!"));
                  }
                }
              } catch (MsgException e) {
                System.out.println(e.toString());
              }
              break;

            case TimsMsgGateway.MBX_DELETE :
              try {
                synchronized (mbxList) {
                  TimsRouterMbxMsg delM = new TimsRouterMbxMsg(p);
                  Integer key = new Integer(delM.mbx);

                  if (mbxList.get(key) == this) {
                    mbxList.remove(key);

                    System.out.println(buildMsg("mbx delete " +
                                       RackName.string(key.intValue())));
                  } else {

                    System.out.println(buildMsg("mbx delete Error: " +
                                       RackName.string(key.intValue()) +
                                       " not in list!"));
                  }
                }
              } catch (MsgException e) {
                System.out.println(e.toString());
              }
              break;

            case TimsMsgGateway.SET_TIME :
              try {
                // das ist nicht so ganz sauber, da
                // ich aber kein neues time-packet erstellen
                // will, lese ich die eine int ueber das mbx-packet.
                p.type = TimsMsgGateway.MBX_INIT;
                TimsRouterMbxMsg initM = new TimsRouterMbxMsg(p);
                long time = initM.mbx;

                TimsMsgGateway.setTimeMillis(time);
                TimsMsg0 reply = new TimsMsg0();
                reply.dest     = p.src;
                reply.src      = p.dest;
                reply.priority = p.priority;
                reply.seq_nr   = p.seq_nr;
                reply.type     = RackMsgType.MSG_OK;
                send(reply);

                System.out.println(buildMsg("Set Time: "+time));

              } catch (MsgException e) {
                System.out.println(e.toString());
              }
            break;

            case TimsMsgGateway.GET_TIME :
              TimsRouterMbxMsg reply = new TimsRouterMbxMsg();
              reply.dest     = p.src;
              reply.src      = p.dest;
              reply.priority = p.priority;
              reply.seq_nr   = p.seq_nr;
              reply.type     = TimsMsgGateway.TIME_DATA;
              // das ist nicht so ganz sauber, da
              // ich aber kein neues time-packet erstellen
              // will, lese ich die eine int ueber das mbx-packet.
              int time = (int) TimsMsgGateway.getTimeMillis();
              reply.mbx = time;
              send(reply);
              System.out.println(buildMsg("Get Time, sent: "+time));
              break;

            case TimsMsgGateway.ENABLE_DEBUG_OUTPUT :
              TimsMsgGateway.setDebugOutputOn(true);
              System.out.println(buildMsg("debug output on"));
              break;

            case TimsMsgGateway.DISABLE_DEBUG_OUTPUT :
              TimsMsgGateway.setDebugOutputOn(false);
              System.out.println(buildMsg("debug output off"));
              break;

            default :
              System.out.println(
                buildMsg("unknown command " + p));
          }
        } else {
          synchronized (mbxList) {
            toConnection = (TimsMsgGatewayConnection)
                              mbxList.get(new Integer(p.dest));

            if (toConnection != null) {
              toConnection.send(p);
              if (TimsMsgGateway.isDebugOutputOn()) {
                // debug printout, every transmitted message
                System.out.println(buildMsg(p.toString()));
              }
            } else {
              TimsMsg0 reply = new TimsMsg0();
              reply.type     = RackMsgType.MSG_NOT_AVAILABLE;
              reply.dest     = p.src;
              reply.src      = p.dest;
              reply.priority = p.priority;
              reply.seq_nr   = p.seq_nr;
              send(reply);

              if (TimsMsgGateway.isDebugOutputOn()) {
                // debug printout, every transmitted message
                System.out.println(
                  buildMsg(p + " not avaiable"));
              }
            }
          }
        }
      }
    } catch (IOException e) {
    }

    synchronized (mbxList) {
      Integer key;

      for (Enumeration e = mbxList.keys(); e.hasMoreElements();) {
        key = (Integer) e.nextElement();

        if (mbxList.get(key) == this) {
          mbxList.remove(key);
          System.out.println(
            buildMsg(
              "mbx delete " + RackName.string(key.intValue())));
        }
      }
    }

    TimsMsgGateway.connectionVec.removeElement(this);
    if (TimsMsgGateway.guiMode) {
      TimsMsgGateway.connectionListModel.removeElement(connectionListRepresentation);
    }

    System.out.println(buildMsg("logout ip " + ip.getHostAddress() + " port " + port));
  }

  public void killConnection() {
    try {
      tcpIn.close();
    } catch (IOException e) {}
  }

  private String buildMsg(String msg) {
    return poutLeader
      + "Con. "
      + connectionCount
      + " Time: "
      + TimsMsgGateway.getTimeMillis()
      + " "
      + msg;
  }
}
