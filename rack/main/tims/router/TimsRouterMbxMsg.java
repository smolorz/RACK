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

import rack.main.tims.msg.*;
import rack.main.tims.exceptions.*;
import rack.main.tims.streams.*;

import java.io.*;

public class TimsRouterMbxMsg extends TimsMsg
{
    public int mbx = 0;   // 4 Bytes

    public int getDataLen()
    {
      return 4;
    }

    public TimsRouterMbxMsg()
    {
      msglen = HEAD_LEN + getDataLen();
    }

    public TimsRouterMbxMsg(TimsDataMsg p) throws MsgException
    {
      readTimsDataMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
      if ((msglen == (HEAD_LEN + getDataLen() )) &&
         ((type == TimsRouter.MBX_INIT) |
          (type == TimsRouter.MBX_DELETE) |
          (type == TimsRouter.MBX_INIT_WITH_REPLY)|
          (type == TimsRouter.MBX_DELETE_WITH_REPLY))) {
          return(true);
      }
      else
          return(false);
    }

    public void readTimsMsgBody(InputStream in) throws IOException
    {
      if (bodyByteorder == BIG_ENDIAN) {

        DataInputStream dataIn = new DataInputStream(in);
        mbx = dataIn.readInt();

      } else {

        LittleEndianDataInputStream dataIn = new LittleEndianDataInputStream(in);
        mbx = dataIn.readInt();
      }
      bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
      DataOutputStream dataOut = new DataOutputStream(out);
      dataOut.writeInt(mbx);
    }
}
