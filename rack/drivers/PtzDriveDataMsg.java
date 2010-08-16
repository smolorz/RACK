/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Matthias Hentschel  <hentschel@rts.uni-hannover.de>
 *
 */
package rack.drivers;

import java.io.*;

import rack.main.RackProxy;
import rack.main.tims.*;

public class PtzDriveDataMsg extends TimsMsg
{
    public int       recordingTime = 0;
    public float     posPan        = 0;
    public float     posTilt       = 0;
    public float     posZoom       = 0;


    public int getDataLen()
    {
      return 16;
    }

    public PtzDriveDataMsg()
    {
      msglen = HEAD_LEN + getDataLen();
    }

    public PtzDriveDataMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
    	if ((type == RackProxy.MSG_DATA) &&
            (msglen == HEAD_LEN + getDataLen()))
    	{
    		return(true);
    	}
    	else
    	{
    		return(false);
    	}
    }

    public void readTimsMsgBody(InputStream in) throws IOException
    {
      EndianDataInputStream dataIn;
      if (bodyByteorder == BIG_ENDIAN)
      {
    	  dataIn = new BigEndianDataInputStream(in);
      }
      else
      {
    	  dataIn = new LittleEndianDataInputStream(in);
      }

      recordingTime = dataIn.readInt();
      posPan        = dataIn.readFloat();
      posTilt       = dataIn.readFloat();
      posZoom       = dataIn.readFloat();
      bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
      DataOutputStream dataOut = new DataOutputStream(out);

      dataOut.writeInt(recordingTime);
      dataOut.writeFloat(posPan);
      dataOut.writeFloat(posTilt);
      dataOut.writeFloat(posZoom);
    }
}
