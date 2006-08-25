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

import java.io.*;

public class TimsDataMsg extends TimsMsg
{
  public byte[] body;

  public int getDataLen()
  {
      if (body != null)
      {
          return body.length;
      }
      else
      {
          return 0;
      }
  }

  public TimsDataMsg()
  {
    msglen = HEAD_LEN;
    body = null;
  }

  public TimsDataMsg(InputStream in) throws IOException
  {
    readTimsMsg(in);
  }

  protected TimsDataMsg(TimsMsg msg, InputStream in) throws IOException
  {
      throw new IOException("Constructor not jet implemented");
  }

  protected boolean checkTimsMsgHead()
  {
    return true;
  }

  protected void readTimsMsgBody(InputStream in) throws IOException
  {
      if((msglen - HEAD_LEN) > 0)
      {
          DataInputStream dataIn = new DataInputStream(in);
          body = new byte[msglen - HEAD_LEN];
          dataIn.readFully(body);
      }
      else
      {
          body = null;
      }
  }

  protected void writeTimsMsgBody(OutputStream out) throws IOException
  {
      if (body != null)
      {
          out.write(body);
      }
  }
}
