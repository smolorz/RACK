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
 *      Christian Wieghardt <wieghardt@rts.uni-hannover.de>
 *
 */
package rack.navigation;

import java.io.*;

import rack.main.*;
import rack.main.tims.*;
import rack.main.defines.*;


public class PilotMultiDestMsg extends TimsMsg
{
    public int pointNum = 0;
    public DestinationPoint[] point;


    public int getDataLen()
    {
        return (4 + pointNum * DestinationPoint.getDataLen());
    }

    public PilotMultiDestMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }


    public PilotMultiDestMsg(TimsRawMsg p) throws TimsException
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

        
        pointNum = dataIn.readInt();
        point    = new DestinationPoint[pointNum];

        for (int i = 0; i < pointNum; i++)
        {
            point[i].readData(dataIn);
        }

        	
        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);
        
        dataOut.writeInt(pointNum);

        for (int i = 0; i < pointNum; i++)
        {
            point[i].writeData(dataOut);
        }


    }
}