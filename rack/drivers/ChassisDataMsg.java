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
 *      Marko Reimer     <reimer@l3s.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.drivers;

import java.io.*;
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;
import rack.main.tims.streams.*;

public class ChassisDataMsg extends TimsMsg
{
    public int   recordingtime = 0;
    public float deltaX        = 0;
    public float deltaY        = 0;
    public float deltaRho      = 0;
    public float vx            = 0;
    public float vy            = 0;
    public float omega         = 0;
    public float battery       = 0;
    public int   activePilot   = 0;

    public int getDataLen()
    {
        return (36);
    }

    public ChassisDataMsg()
    {
        msglen = headLen + getDataLen();
    }

    public ChassisDataMsg(TimsDataMsg p) throws MsgException
    {
        readTimsDataMsg(p);
    }

    protected boolean checkTimsMsgHead()
    {
      if ((type == RackMsgType.MSG_DATA) &&
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

      recordingtime = dataIn.readInt();
      deltaX        = dataIn.readFloat();
      deltaY        = dataIn.readFloat();
      deltaRho      = dataIn.readFloat();
      vx            = dataIn.readFloat();
      vy            = dataIn.readFloat();
      omega         = dataIn.readFloat();
      battery       = dataIn.readFloat();
      activePilot   = dataIn.readInt();
      bodyByteorder = BIG_ENDIAN;
    }

    protected void writeTimsMsgBody(OutputStream out) throws IOException
    {
      DataOutputStream dataOut = new DataOutputStream(out);
      dataOut.writeInt(recordingtime);
      dataOut.writeFloat(deltaX);
      dataOut.writeFloat(deltaY);
      dataOut.writeFloat(deltaRho);
      dataOut.writeFloat(vx);
      dataOut.writeFloat(vy);
      dataOut.writeFloat(omega);
      dataOut.writeFloat(battery);
      dataOut.writeInt(activePilot);
    }
}
