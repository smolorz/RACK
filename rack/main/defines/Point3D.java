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
import rack.main.tims.streams.EndianDataInputStream;

public class Point3D
{
    public int x     = 0;
    public int y     = 0;
    public int z     = 0;
        
    static public int getDataLen()
    {
        return (12);
    }

    public void readData(EndianDataInputStream dataIn) throws IOException 
    {
        x = dataIn.readInt();
        y = dataIn.readInt();
        z = dataIn.readInt();
    }

    public void writeData(DataOutputStream dataOut) throws IOException 
    {
        dataOut.writeInt(x);
        dataOut.writeInt(y);
        dataOut.writeInt(z);
    }
}
