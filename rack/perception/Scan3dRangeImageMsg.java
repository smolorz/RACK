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
 *      Oliver Wulf  <oliver.wulf@gmx.de>
 *
 */
package rack.perception;

import java.io.*;

import rack.main.defines.Position3d;
import rack.main.defines.Scan3dRangeImagePoint;
import rack.main.tims.*;

public class Scan3dRangeImageMsg extends TimsMsg
{
    public int recordingTime;
    public int scanMode;
    public int maxRange;
    public short scanNum;
    public short scanPointNum;
    public Scan3dRangeImagePoint[][] point;

    public int getDataLen()
    {
        return (16 + Position3d.getDataLen() + scanNum * scanPointNum * 4);
    }

    public Scan3dRangeImageMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public Scan3dRangeImageMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public int getScanMode()
    {
        return (scanMode & 0xff);
    }

    public boolean isPositiveTurn()
    {
        if ((scanMode & 0x100) == 0x100)
        {
            return (true);
        }
        else
        {
            return (false);
        }
    }

    public boolean isSquareGrid()
    {
        if ((scanMode & 0x200) == 0x200)
        {
            return (true);
        }
        else
        {
            return (false);
        }
    }

    public boolean isStart180()
    {
        if ((scanMode & 0x400) == 0x400)
        {
            return (true);
        }
        else
        {
            return (false);
        }
    }

    public boolean checkTimsMsgHead()
    {
        if (type == Scan3dProxy.MSG_SCAN3D_RANGE_IMAGE)
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
        scanMode = dataIn.readInt();
        maxRange = dataIn.readInt();
        scanNum = dataIn.readShort();
        scanPointNum = dataIn.readShort();

        point = new Scan3dRangeImagePoint[scanNum][scanPointNum];
        for (int i = 0; i < scanNum; i++)
        {
            for (int j = 0; j < scanPointNum; j++)
            {
                point[i][j] = new Scan3dRangeImagePoint(dataIn);
            }
        }

        bodyByteorder = BIG_ENDIAN;

        // System.out.println("scanType " + scanType + " maxRange " + maxRange +
        // " scanNum " + scanNum + " scanPointNum " + scanPointNum);
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);
        dataOut.writeInt(recordingTime);
        dataOut.writeInt(scanMode);
        dataOut.writeInt(maxRange);
        dataOut.writeShort(scanNum);
        dataOut.writeShort(scanPointNum);

        for (int i = 0; i < scanNum; i++)
        {
            for (int j = 0; j < scanPointNum; j++)
            {
                point[i][j].writeDataOut(dataOut);
            }
        }
    }
}
