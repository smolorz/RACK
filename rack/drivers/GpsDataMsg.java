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
package rack.drivers;

import java.io.*;

import rack.main.defines.Position3D;
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;
import rack.main.tims.streams.*;

public class GpsDataMsg extends TimsMsg
{
    public int          recordingTime = 0;
    public int          mode          = 0;
    public double       latitude      = 0;  // rad
    public double       longitude     = 0;  // rad
    public int          altitude      = 0;  // mm over mean sea level
    public float        heading       = 0;  // rad
    public int          speed         = 0;  // mm/s
    public int          satelliteNum  = 0;
    public long         utcTime       = 0;  // POSIX time in sec since 1.1.1970
    public float        pdop          = 0;
    public Position3D   posGK  = new Position3D();
    
    
    public int getDataLen()
    {
        return 28 + 24 + Position3D.getDataLen();

    }

    public GpsDataMsg()
    {
        msglen = headLen + getDataLen();
    }
    
    public GpsDataMsg(TimsDataMsg p) throws MsgException
    {
        readTimsDataMsg(p);
    }

    protected boolean checkTimsMsgHead()
    {
        if (type == RackMsgType.MSG_DATA)
        {
            return (true);
        }
        else
        {
            return (false);
        }
    }

    protected void readTimsMsgBody(InputStream in) throws IOException
    {
        EndianDataInputStream dataIn;

        if (bodyByteorder == BIG_ENDIAN)
        {
            dataIn = new BigEndianDataInputStream(in);
        }
        else
        {
            dataIn = new LittleEndianDataInputStream(in);
        }

        recordingTime = dataIn.readInt();
        mode          = dataIn.readInt();
        latitude      = dataIn.readDouble();
        longitude     = dataIn.readDouble();
        altitude      = dataIn.readInt();
        heading       = dataIn.readFloat();
        speed         = dataIn.readInt();
        satelliteNum  = dataIn.readInt();
        utcTime       = dataIn.readLong();
        pdop          = dataIn.readFloat();
        posGK.readData(dataIn);
        
        bodyByteorder = BIG_ENDIAN;
    }

    protected void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);
      
        dataOut.writeInt(recordingTime);
        dataOut.writeInt(mode);
        dataOut.writeDouble(latitude);
        dataOut.writeDouble(longitude);
        dataOut.writeInt(altitude);
        dataOut.writeFloat(heading);
        dataOut.writeInt(speed);
        dataOut.writeInt(satelliteNum);
        dataOut.writeLong(utcTime);
        dataOut.writeFloat(pdop);
        posGK.writeData(dataOut);        
    }    
}
