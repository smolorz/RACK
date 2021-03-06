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
 *      Oliver Wulf  <oliver.wulf@gmx.de>
 *
 */
package rack.perception;

import java.io.*;

import rack.main.RackProxy;
import rack.main.defines.ScanPoint;
import rack.main.defines.Position3d;
import rack.main.tims.*;

public class Scan2dDataMsg extends TimsMsg
{
    public int recordingTime = 0;
    public int duration = 0;
    public int maxRange = 0;
    public int sectorNum = 1;
    public int sectorIndex = 0;
    public Position3d refPos = new Position3d();  
    public int pointNum = 0;
    public ScanPoint[] point;

    public int getDataLen()
    {
        return (24 + Position3d.getDataLen() + pointNum * ScanPoint.getDataLen());
    }

    public Scan2dDataMsg(TimsRawMsg p) throws TimsException
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
        duration = dataIn.readInt();
        maxRange = dataIn.readInt();
        sectorNum = dataIn.readInt();
        sectorIndex = dataIn.readInt();

        refPos.readData(dataIn);
        
        pointNum = dataIn.readInt();
        point = new ScanPoint[pointNum];
        
        for (int i = 0; i < pointNum; i++)
        {
            point[i] = new ScanPoint(dataIn);
        }
        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(recordingTime);
        dataOut.writeInt(duration);
        dataOut.writeInt(maxRange);
        dataOut.writeInt(sectorNum);
        dataOut.writeInt(sectorIndex);
        
        refPos.writeData(dataOut);

        dataOut.writeInt(pointNum);
        for (int i = 0; i < pointNum; i++)
        {
            point[i].writeDataOut(dataOut);
        }
    }
}
