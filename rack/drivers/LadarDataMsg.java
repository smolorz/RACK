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
package rack.drivers;

import java.io.*;

import rack.main.RackProxy;
import rack.main.tims.*;

import rack.main.defines.LadarPoint;

public class LadarDataMsg extends TimsMsg
{
    public int recordingTime = 0;
    public int duration = 0;
    public int maxRange = 0;
    public float startAngle = 0.0f;
    public float endAngle = 0.0f;
    public int pointNum = 0;
    public LadarPoint[] point = new LadarPoint[0];

    public int getDataLen()
    {
        return (24 + pointNum * LadarPoint.getDataLen());
    }

    public LadarDataMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public LadarDataMsg(TimsRawMsg p) throws TimsException
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
        duration      = dataIn.readInt();
        maxRange      = dataIn.readInt();
        startAngle    = dataIn.readFloat();
        endAngle      = dataIn.readFloat();
        pointNum      = dataIn.readInt();
        point 		  = new LadarPoint[pointNum];
        for (int i = 0; i < pointNum; i++)
        {
            point[i] = new LadarPoint(dataIn);
        }

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(recordingTime);
        dataOut.writeInt(duration);
        dataOut.writeInt(maxRange);
        dataOut.writeFloat(startAngle);
        dataOut.writeFloat(endAngle);
        dataOut.writeInt(pointNum);

        for (int i = 0; i < pointNum; i++)
        {
            point[i].writeDataOut(dataOut);
        }
    }
}
