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
package rack.main.proxy;

import java.io.*;
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;
import rack.main.tims.streams.*;

/** Paket zum Abmelden von einem kontinuierlichen Datenstream.
 * Die Mailbox, die abgemeldet weden soll, muss mit uebergeben werden */

public class StopContDataMsg extends TimsMsg
{
    /** Mailbox die abgemeldet werden soll */
    public int dataMbx; // 4 Bytes

    public int getDataLen()
    {
      return 4;
    }

    public StopContDataMsg()
    {
      msglen = headLen + getDataLen();
    }

    public StopContDataMsg(TimsDataMsg p) throws MsgException
    {
        readTimsDataMsg(p);
    }

    protected boolean checkTimsMsgHead()
    {
      if ((type == RackMsgType.MSG_STOP_CONT_DATA) &&
          (msglen == headLen + getDataLen())) {
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

      dataMbx = dataIn.readInt();
      bodyByteorder = BIG_ENDIAN;
    }


    protected void writeTimsMsgBody(OutputStream out) throws IOException
    {
      DataOutputStream dataOut = new DataOutputStream(out);
      dataOut.writeInt(dataMbx);
    }
}
