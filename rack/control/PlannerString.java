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
 *      Daniel Lecking <lecking@rts.uni-hannover.de>
 *
 */
package rack.control;

import java.io.*;
import rack.main.tims.*;

public class PlannerString
{
    public static final int MAX_STRING_LEN = 80;

    public int stringLen = 0; // 4 Byte
    public String string = "";

    static public int getDataLen()
    {
        return(4 + MAX_STRING_LEN);
    }

    public PlannerString()
    {
    }

    public PlannerString(EndianDataInputStream dataIn) throws IOException
    {
        byte[] fullString = new byte[MAX_STRING_LEN];

        stringLen  = dataIn.readInt();

        dataIn.readFully(fullString);
        string = new String(fullString);

        if(stringLen == 0)
        {
            string = "";
        }
        else if(stringLen < MAX_STRING_LEN)
        {
            string = string.substring(0, stringLen).trim();
        }
    }

    public void writeData(DataOutputStream dataOut) throws IOException
    {
        dataOut.writeInt(stringLen);
        dataOut.writeBytes(string);

        for (int i = 0; i < (MAX_STRING_LEN - string.length()); i++)
        {
            dataOut.writeByte(0);
        }
    }
}
