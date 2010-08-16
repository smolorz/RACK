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
import rack.main.defines.*;
import rack.main.tims.*;

public class PathDestMsg extends TimsMsg
{
    public Position2d pos = new Position2d();
    public float 	  moveDir;
    public int   	  layer;

    public int getDataLen()
    {
       return (8 + Position2d.getDataLen());
    }

    public PathDestMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public PathDestMsg(TimsRawMsg p) throws TimsException
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

        pos.readData(dataIn);
        moveDir = dataIn.readFloat();
        layer   = dataIn.readInt();

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        pos.writeData(dataOut);
        dataOut.writeFloat(moveDir);
        dataOut.writeInt(layer);
    }
}
