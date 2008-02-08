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
package rack.main.defines;

import java.io.DataOutputStream;
import java.io.IOException;

import rack.main.tims.EndianDataInputStream;

public class PolarSpline
{
    public Point2d		basepoint = new Point2d();
	public Position2d   startPos  = new Position2d();
    public Position2d   endPos    = new Position2d();
    public Position2d   centerPos = new Position2d();
    public int          length    = 0;
    public int          radius    = 0;
    public int          vMax      = 0;
    public int          vStart    = 0;
    public int          vEnd      = 0;
    public int          aMax      = 0;
    public int			type	  = 0;
    public int			request	  = 0;
    public int          lbo       = 0;

    static public int getDataLen()
    {
        return (36 + Point2d.getDataLen() + 3 * Position2d.getDataLen());
    }

    public PolarSpline()
    {
    }

    public PolarSpline(EndianDataInputStream dataIn) throws IOException
    {
        basepoint.readData(dataIn);
    	startPos.readData(dataIn);
        endPos.readData(dataIn);
        centerPos.readData(dataIn);
        length    = dataIn.readInt();
        radius    = dataIn.readInt();
        vMax      = dataIn.readInt();
        vStart    = dataIn.readInt();
        vEnd      = dataIn.readInt();
        aMax      = dataIn.readInt();
        type	  = dataIn.readInt();
        request   = dataIn.readInt();
        lbo       = dataIn.readInt();
    }

    public void writeData(DataOutputStream dataOut) throws IOException
    {
        basepoint.writeData(dataOut);
    	startPos.writeData(dataOut);
        endPos.writeData(dataOut);
        centerPos.writeData(dataOut);
        dataOut.writeInt(length);
        dataOut.writeInt(radius);
        dataOut.writeInt(vMax);
        dataOut.writeInt(vStart);
        dataOut.writeInt(vEnd);
        dataOut.writeInt(aMax);
        dataOut.writeInt(type);
        dataOut.writeInt(request);
        dataOut.writeInt(lbo);
    }
}