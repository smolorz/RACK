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

public class FeatureMapDataPoint
{

    public static int LINE_FEATURE = 10;

    public double x 	= 0;
    public double y 	= 0;
    public double x2 	= 0;
    public double y2 	= 0;
    public double l  	= 0;
    public double rho   = 0;
    public double sin   = 0;
    public double cos   = 0;
    public int layer    = 0;
    public int type     = 0;

    static public int getDataLen()
    {
        return (56);
    }

    public FeatureMapDataPoint(int x, int y, int x2, int y2, int l, int rho, int sin, int cos, int layer, int type)
    {
        this.x 		= x;
        this.y 		= y;
        this.x2 	= x2;
        this.y2		= y2;
        this.l		= l;
        this.rho	= rho;
        this.sin	= sin;
        this.cos	= cos;
        this.layer  = layer;
        this.type = type;
    }

    public FeatureMapDataPoint()
    {
        this.x 		= 0;
        this.y 		= 0;
        this.x2 	= 0;
        this.y2		= 0;
        this.l		= 0;
        this.rho	= 0;
        this.sin	= 0;
        this.cos	= 0;
        this.layer  = 0;
        this.type = 0;
    }

    public void readData(EndianDataInputStream dataIn) throws IOException
    {
        x 	 = dataIn.readDouble();
        y 	 = dataIn.readDouble();
        x2 	 = dataIn.readDouble();
        y2 	 = dataIn.readDouble();
        l  	 = dataIn.readDouble();
        rho  = dataIn.readDouble();
        sin  = dataIn.readDouble();
        cos  = dataIn.readDouble();
        layer = dataIn.readInt();
        type = dataIn.readInt();
    }

    public void writeData(DataOutputStream dataOut) throws IOException
    {
        dataOut.writeDouble(x);
        dataOut.writeDouble(y);
        dataOut.writeDouble(x2);
        dataOut.writeDouble(y2);
        dataOut.writeDouble(l);
        dataOut.writeDouble(rho);
        dataOut.writeDouble(sin);
        dataOut.writeDouble(cos);
        dataOut.writeInt(layer);
        dataOut.writeInt(type);
    }
}
