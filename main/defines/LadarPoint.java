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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
package rack.main.defines;

import java.io.DataOutputStream;
import java.io.IOException;

import rack.main.tims.EndianDataInputStream;

public class LadarPoint {
    
    public static final int TYPE_UNKNOWN          = 0x0000;
	public static final int TYPE_TRANSPARENT      = 0x0001;
	public static final int TYPE_RAIN		      = 0x0002;
	public static final int TYPE_DIRT		      = 0x0003;
	
	public static final int TYPE_INVALID	      = 0x0010;
	public static final int TYPE_REFLECTOR	      = 0x0080;
	
    public float angle        = 0;    // 4 Bytes
    public int distance       = 0;    // 4 Bytes
    public int type           = 0;    // 4 Bytes --> 12 Bytes completely
    
    static public int getDataLen()
    {
        return 20;
    }

    /**
     * @param dataIn
     */
    public LadarPoint(EndianDataInputStream dataIn) throws IOException
    {
        angle          = dataIn.readFloat();
        distance       = dataIn.readInt();
        type           = dataIn.readInt();
    }
    
    /**
     * @param dataOut
     */
    public void writeDataOut(DataOutputStream dataOut) throws IOException
    {
        dataOut.writeFloat(angle);
        dataOut.writeInt(distance);
        dataOut.writeInt(type);
    }
    
    public String toString()
    {
        return angle + " " + distance + " " + type;
    }
}
