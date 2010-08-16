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

import rack.main.RackProxy;
import rack.main.defines.Position3d;
import rack.main.defines.ScanPoint;
import rack.main.Scan3dCompressTool;
import rack.main.tims.*;

public class Scan3dDataMsg extends TimsMsg
{
    // scanMode
    public static final int ROLL = 1;
    public static final int PITCH = 2;
    public static final int YAW = 3;
    public static final int TOP = 4;
    public static final int TOP_DOWN = 5;

    public int recordingTime = 0;
    public int duration = 0;
    public int maxRange = 0;
    public int scanNum = 0;
    public int scanPointNum = 0;
    public int scanMode = 0;
    public int scanHardware = 0;
    public int sectorNum = 1;
    public int sectorIndex = 0;
    public Position3d refPos = new Position3d();
    public int pointNum = 0;
    public int compressed = 0;
    public ScanPoint[] point;

    public int getDataLen()
    {
        return (44 + Position3d.getDataLen() + pointNum * ScanPoint.getDataLen());
    }

    public Scan3dDataMsg(TimsRawMsg p) throws TimsException
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
            return true;
        }
        else
        {
            return false;
        }
    }

    public boolean isSquareGrid()
    {
        if ((scanMode & 0x200) == 0x200)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    public boolean isStart180()
    {
        if ((scanMode & 0x400) == 0x400)
        {
            return true;
        }
        else
        {
            return false;
        }
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
        duration      = dataIn.readInt();
        maxRange      = dataIn.readInt();
        scanNum       = dataIn.readInt();
        scanPointNum  = dataIn.readInt();
        scanMode      = dataIn.readInt();
        scanHardware  = dataIn.readInt();
        sectorNum     = dataIn.readInt();
        sectorIndex   = dataIn.readInt();

        refPos.readData(dataIn);

        pointNum      = dataIn.readInt();
        compressed    = dataIn.readInt();

        point = new ScanPoint[pointNum];

        if (compressed > 0)
        {
            byte[] buf = new byte[pointNum * ScanPoint.getDataLen() + 42];    // buffer for compressed data

            for (int i = 0; i < compressed; i++)
            {
                buf[i] = dataIn.readByte();
            }

            // create empty points
            for (int i = 0; i < pointNum; i++)
            {
                point[i] = new ScanPoint();
            }

            // call decompression
            Scan3dCompressTool decompressor = new Scan3dCompressTool();
            decompressor.Decompress(buf, point, compressed, pointNum);
            compressed = 0;
        }
        else
        {
        	for (int i = 0; i < pointNum; i++)
        	{
        		point[i] = new ScanPoint(dataIn);
        	}
        }

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);
        dataOut.writeInt(recordingTime);
        dataOut.writeInt(duration);
        dataOut.writeInt(maxRange);
        dataOut.writeInt(scanNum);
        dataOut.writeInt(scanPointNum);
        dataOut.writeInt(scanMode);
        dataOut.writeInt(scanHardware);
        dataOut.writeInt(sectorNum);
        dataOut.writeInt(sectorIndex);

        refPos.writeData(dataOut);

        dataOut.writeInt(pointNum);
        dataOut.writeInt(compressed);

        for (int i = 0; i < pointNum; i++)
        {
            point[i].writeDataOut(dataOut);
        }
    }

}
