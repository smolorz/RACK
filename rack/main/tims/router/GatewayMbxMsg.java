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

class GatewayMbxMsg extends TimsMsg
{
    public int mbx = 0;   // 4 Bytes

    public int getDataLen()
    {
      return 4;
    }

    protected GatewayMbxMsg()
    {
      msglen = headLen + getDataLen();
    }

    protected GatewayMbxMsg(TimsDataMsg p) throws MsgException
    {
      readTimsDataMsg(p);
    }

    protected boolean checkTimsMsgHead()
    {
      if ((msglen == (headLen + getDataLen() )) &&
         ((type == TimsMsgGateway.MBX_INIT) |
          (type == TimsMsgGateway.MBX_DELETE) |
          (type == TimsMsgGateway.MBX_INIT_WITH_REPLY)|
          (type == TimsMsgGateway.MBX_DELETE_WITH_REPLY))) {
          return(true);
      }
      else
          return(false);
    }

    protected void readTimsMsgBody(InputStream in) throws IOException
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

    protected void writeTimsMsgBody(OutputStream out) throws IOException
    {
      DataOutputStream dataOut = new DataOutputStream(out);
      dataOut.writeInt(mbx);
    }
}
