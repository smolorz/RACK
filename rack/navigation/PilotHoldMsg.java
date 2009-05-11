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
 *      Daniel Lecking <lecking@rts.uni-hannover.de>
 *
 */
package rack.navigation;

import java.io.*;

import rack.main.*;
import rack.main.tims.*;
import rack.main.defines.*;


public class PilotHoldMsg extends TimsMsg
{
    public int holdState    = 0;
    public int holdTime 	= 0;
    public Position3d pos = new Position3d();

    public int getDataLen()
    {
        return (8 + Position3d.getDataLen());
    }

    public PilotHoldMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public PilotHoldMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if (type == RackProxy.MSG_DATA &&
            msglen == HEAD_LEN + getDataLen())
        {
            return true;
        }
        else
            return false;
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

        holdState = dataIn.readInt();
        holdTime = dataIn.readInt();
        pos.readData(dataIn);

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(holdState);
        dataOut.writeInt(holdTime);
        pos.writeData(dataOut);
    }
}