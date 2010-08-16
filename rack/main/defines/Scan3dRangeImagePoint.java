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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.main.defines;

import java.io.*;

import rack.main.tims.*;

public class Scan3dRangeImagePoint
{

    public short range = 0; // 2 Bytes
    public short type = 0; // 2 Bytes --> 4 Bytes completely

    /**
     * @param dataIn
     */
    public Scan3dRangeImagePoint(EndianDataInputStream dataIn)
            throws IOException
    {
        range = dataIn.readShort();
        type = dataIn.readShort();
    }

    /**
     * @param dataOut
     */
    public void writeDataOut(DataOutputStream dataOut) throws IOException
    {
        dataOut.writeShort(range);
        dataOut.writeShort(type);
    }
}
