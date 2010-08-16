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


import rack.main.RackProxy;
import rack.main.defines.*;
import rack.main.tims.*;

public class FeatureMapDataMsg extends TimsMsg
{
    public int recordingTime 	= 0;
    public Position3d pos = new Position3d();
    public int featureNum = 0;
    public FeatureMapDataPoint[] feature;

    public int getDataLen()
    {
         return (8 + Position3d.getDataLen() + featureNum * FeatureMapDataPoint.getDataLen());
    }

    public FeatureMapDataMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public FeatureMapDataMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if (type == RackProxy.MSG_DATA)
        {
            return true;
        }
        else
        {
            return false;
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
        pos.readData(dataIn);
        featureNum = dataIn.readInt();

        feature = new FeatureMapDataPoint[featureNum];

        for (int i = 0; i < featureNum; i++)
        {
            feature[i] = new FeatureMapDataPoint();
            feature[i].readData(dataIn);
        }
        bodyByteorder = BIG_ENDIAN;
     }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(recordingTime);
        pos.writeData(dataOut);
        dataOut.writeInt(featureNum);

        for (int i = 0; i < featureNum; i++)
        {
        	feature[i].writeData(dataOut);
        }
    }
}
