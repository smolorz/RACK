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
package rack.main.tims.msg;

import java.io.*;

public class TimsDataMsg extends TimsMsg
{
  public byte[] body = null;

  public int getDataLen()
  {
    return 0;
  }

  public TimsDataMsg()
  {
    msglen = headLen;
  }

  public TimsDataMsg(InputStream in) throws IOException
  {
    readTimsMsg(in);
  }

  protected TimsDataMsg(TimsMsg msg, InputStream in) throws IOException
  {
    System.out.println("not jet implemented");
  }

  protected boolean checkTimsMsgHead()
  {
    return(true);
  }

  protected void readTimsMsgBody(InputStream in) throws IOException
  {
    DataInputStream dataIn = new DataInputStream(in);
    body = new byte[msglen - headLen];
    dataIn.readFully(body);
  }

  protected void writeTimsMsgBody(OutputStream out) throws IOException
  {
    if (body != null) {
      out.write(body);
    }
  }
}
