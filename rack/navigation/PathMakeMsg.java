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
package rack.navigation;

import java.io.*;

import rack.main.RackProxy;
import rack.main.tims.*;

public class PathMakeMsg extends TimsMsg
{
    public int   vMax = 0;
    public int   accMax = 0;
    public int   decMax = 0;
    public float omegaMax = 0.0f;

    public int getDataLen()
    {
       return 16;
    }

    public PathMakeMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public PathMakeMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if ((type == RackProxy.MSG_DATA)
                && (msglen == HEAD_LEN + getDataLen()))
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

        vMax	 = dataIn.readInt();
        accMax   = dataIn.readInt();
        decMax   = dataIn.readInt();
        omegaMax = dataIn.readFloat();
        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(vMax);
        dataOut.writeInt(accMax);
        dataOut.writeInt(decMax);
        dataOut.writeFloat(omegaMax);
    }
}
