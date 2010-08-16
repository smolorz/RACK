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

import java.io.DataOutputStream;
import java.io.IOException;

import rack.main.tims.EndianDataInputStream;

public class MclDataPoint
{
    public static int SAMPLE = 1;
    public static int MEASUREMENT = 2;
	public static int REFLECTOR = 3;
    public static int LINE_FEATURE = 10;


    public int x = 0; // 4 Byte
    public int y = 0; // 4 Byte
    public int z = 0; // 4 Byte
    public int type = 0; // 4 Byte
    public int layer = 0; // 4 Byte

    public MclDataPoint(int x, int y, int z, int type, int layer)
    {
        this.x = x;
        this.y = y;
        this.z = z;
        this.type = type;
        this.layer = layer;
    }

    public MclDataPoint()
    {
        this.x = 0;
        this.y = 0;
        this.z = 0;
        this.type = 0;
        this.layer = 0;
    }

    static public int getDataLen()
    {
        return (16);
    }

    public void readData(EndianDataInputStream dataIn) throws IOException
    {
        x = dataIn.readInt();
        y = dataIn.readInt();
        z = dataIn.readInt();
        type = dataIn.readInt();
        layer = dataIn.readInt();
    }

    public void writeData(DataOutputStream dataOut) throws IOException
    {
        dataOut.writeInt(x);
        dataOut.writeInt(y);
        dataOut.writeInt(z);
        dataOut.writeInt(type);
        dataOut.writeInt(layer);
    }
}
