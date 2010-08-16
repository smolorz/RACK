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
 *      Oliver Wulf <oliver.wulf@gmx.de>
 *
 */
package rack.navigation;

import java.io.*;

import rack.main.*;
import rack.main.tims.*;


public class FeatureMapFeatureMsg extends TimsMsg
{
    public int recordingTime = 0;
    public int featureNumTemp = 0;
    public FeatureMapDataPoint dataPoint = new FeatureMapDataPoint();

    public int getDataLen()
    {
        //return (4 + FeatureMapDataPoint.getDataLen());
        return (8 + FeatureMapDataPoint.getDataLen());
    }

    public FeatureMapFeatureMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public FeatureMapFeatureMsg(TimsRawMsg p) throws TimsException
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

        recordingTime = dataIn.readInt();
        dataPoint.readData(dataIn);
        featureNumTemp = dataIn.readInt();

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(recordingTime);
        dataPoint.writeData(dataOut);
        dataOut.writeInt(featureNumTemp);
    }
}