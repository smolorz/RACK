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
 *      Oliver Wulf        <oliver.wulf@gmx.de>
 *
 */
package rack.main;

import java.io.DataOutputStream;
import java.io.IOException;

import rack.main.tims.EndianDataInputStream;

public class RackParam
{
    public static final int   INT32 = 0;
    public static final int   STRING = 1;
    public static final int   FLOAT = 2;
    
    public static final int   MAX_STRING_LEN = 80;
    
    public String       name = "";
    public int          type = 0;
    public int          valueInt32 = 0;
    public float        valueFloat = 0.0f;
    public String       valueString = "";
    
    public RackParam()
    {
    }

    public RackParam(EndianDataInputStream in) throws IOException
    {
        readData(in);
    }

    static public int getDataLen()
    {
        return (12 + 2 * MAX_STRING_LEN);
    }

    public void readData(EndianDataInputStream dataIn) throws IOException 
    {
        byte[] stringBuffer = new byte[MAX_STRING_LEN];

        dataIn.readFully(stringBuffer);
        name        = new String(stringBuffer);
        name        = name.trim();
        type        = dataIn.readInt();
        valueInt32  = dataIn.readInt();
        valueFloat  = dataIn.readFloat();
        dataIn.readFully(stringBuffer);
        valueString = new String(stringBuffer);
        valueString = valueString.trim();
    }

    public void writeData(DataOutputStream dataOut) throws IOException 
    {
        dataOut.writeBytes(name);
        for (int i = 0; i < (MAX_STRING_LEN - name.length()); i++)
        {
            dataOut.writeByte(0);
        }
        dataOut.writeInt(type);
        dataOut.writeInt(valueInt32);
        dataOut.writeFloat(valueFloat);
        dataOut.writeBytes(valueString);
        for (int i = 0; i < (MAX_STRING_LEN - valueString.length()); i++)
        {
            dataOut.writeByte(0);
        }
    }    
    
    public String toString()
    {
        switch(type)
        {
        case STRING:
            return name + " -> \"" + valueString + "\"";
        case FLOAT:
            return name + " -> " + valueFloat;
        default: // INT32
            return name + " -> " + valueInt32;
        }
    }
}
