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
      msglen = HEAD_LEN + getDataLen();
    }

    public StopContDataMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
      if ((type == RackProxy.MSG_STOP_CONT_DATA) &&
          (msglen == HEAD_LEN + getDataLen())) {
        return(true);
      } else {
        return(false);
      }
    }

    public void readTimsMsgBody(InputStream in) throws IOException
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


    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
      DataOutputStream dataOut = new DataOutputStream(out);
      dataOut.writeInt(dataMbx);
    }
}
