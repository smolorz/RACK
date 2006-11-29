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
package rack.main;

import java.io.*;

import rack.main.tims.*;

public class GetDataMsg extends TimsMsg
{
    public int recordingTime = 0; // 4 Byte

    public int getDataLen()
    {
      return 4;
    }

    public GetDataMsg()
    {
      msglen = HEAD_LEN + getDataLen();
    }

    protected GetDataMsg(TimsDataMsg p) throws TimsException
    {
        readTimsDataMsg(p);
    }

    protected boolean checkTimsMsgHead()
    {
      if ((type == RackProxy.MSG_GET_DATA) &&
          (msglen == HEAD_LEN + getDataLen() )) {
        return(true);
      } else {
        return(false);
      }
    }

    protected void readTimsMsgBody(InputStream in) throws IOException
    {
      EndianDataInputStream dataIn;

      if (bodyByteorder == BIG_ENDIAN) {
        dataIn = new BigEndianDataInputStream(in);
      } else {
        dataIn = new LittleEndianDataInputStream(in);
      }

      recordingTime = dataIn.readInt();

      bodyByteorder = BIG_ENDIAN;
    }

    protected void writeTimsMsgBody(OutputStream out) throws IOException
    {
      DataOutputStream dataOut = new DataOutputStream(out);
      dataOut.writeInt(recordingTime);
    }
}
