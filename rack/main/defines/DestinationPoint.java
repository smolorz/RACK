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
package rack.main.defines;

import java.io.DataOutputStream;
import java.io.IOException;

import rack.main.tims.EndianDataInputStream;

public class DestinationPoint
{
	public Position3d pos	= new Position3d();
    public int  speed       = 0;

    static public int getDataLen()
    {
        return (4+Position3d.getDataLen());
    }

    public DestinationPoint()
    {
    }

    public DestinationPoint(Position3d pos, int speed)
    {
        this.pos         = pos;
        this.speed       = speed;       
    }

    public void readData(EndianDataInputStream dataIn) throws IOException
    {
        pos.readData(dataIn);
        speed           = dataIn.readInt();
    };

    public void writeData(DataOutputStream dataOut) throws IOException
    {
    	pos.writeData(dataOut);
        dataOut.writeInt(speed);
    }
}
