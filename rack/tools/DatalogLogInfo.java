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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 */
package rack.tools;

import java.io.*;

import rack.main.tims.*;


public class DatalogLogInfo
{
    public int     logEnable = 0;
    public int     moduleMbx = 0;
    public int     periodTime = 0;
    public String  filename = "";    

    static public int getDataLen()
    {
        return(12 + 40);
    }

    public DatalogLogInfo()
    {
    }

    public DatalogLogInfo(EndianDataInputStream dataIn) throws IOException
    {
        byte[] name = new byte[40];

        logEnable  = dataIn.readInt();
        moduleMbx  = dataIn.readInt();
        periodTime = dataIn.readInt();
        
        dataIn.readFully(name);
        filename   = new String(name);
    }
    
    public void writeData(DataOutputStream dataOut) throws IOException
    {
        dataOut.writeInt(logEnable);
        dataOut.writeInt(moduleMbx);
        dataOut.writeInt(periodTime);
        dataOut.writeBytes(filename);
        
        for (int i = 0; i < (40 - filename.length()); i++)
        {
            dataOut.writeByte(0);
        }
    }       
}


