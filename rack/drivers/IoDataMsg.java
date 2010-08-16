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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
package rack.drivers;

import java.io.*;

import rack.main.RackProxy;
import rack.main.tims.*;

public class IoDataMsg extends TimsMsg
{
    public int       recordingTime = 0;
    public int		 valueNum 	   = 0;
	public byte[] 	 value    	   = new byte[0];

    public int getDataLen()
    {
        return 8 + valueNum;

    }

    public IoDataMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public IoDataMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if (type == RackProxy.MSG_DATA)
        {
            return (true);
        }
        else
        {
            return (false);
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
        valueNum	  = dataIn.readInt();

    	value	      = new byte[valueNum];
    	dataIn.readFully(value);

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(recordingTime);
        dataOut.writeInt(valueNum);

        for (int i = 0; i < valueNum; i++)
        {
        	dataOut.writeByte(value[i]);
        }
    }
}
