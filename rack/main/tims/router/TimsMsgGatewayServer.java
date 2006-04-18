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

import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * In dieser Klasse lauscht das TimsMsgGateway
 * auf eingehende Verbindungen.
 */
public class TimsMsgGatewayServer extends Thread {

  protected ServerSocket serverSocket;
//  protected TimsMsgGateway gateway;

  public TimsMsgGatewayServer(InetAddress ip, int port) throws IOException {
//    this.gateway = gateway;
    serverSocket = new ServerSocket(port, 10, ip);

        System.out.println("TimsMsgGateway init ip " +
                           serverSocket.getInetAddress().getHostAddress() +
                           " port " + serverSocket.getLocalPort());
    }

    public void run()
    {
        Socket                   newSocket;

        try
        {
            while(true)
            {
                newSocket = serverSocket.accept();
                newSocket.setTcpNoDelay(true);
        TimsMsgGateway.insertSocket(newSocket);
            }
        }
        catch(IOException e)
        {
            System.out.println("TimsMsgGateway error. " + e.toString());
        }
    }


}
