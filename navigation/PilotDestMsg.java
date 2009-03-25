/*
 * RACK-RTS - Robotics Application Construction Kit (RTS internal)
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * All rights reserved.
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


public class PilotDestMsg extends TimsMsg
{
    public Position3d pos = new Position3d();
    public int speed      = 0;

    public int getDataLen()
    {
        return (4 + Position3d.getDataLen());
    }

    public PilotDestMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public PilotDestMsg(TimsRawMsg p) throws TimsException
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

        pos.readData(dataIn);
        speed = dataIn.readInt();

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        pos.writeData(dataOut);
        dataOut.writeInt(speed);
    }
}