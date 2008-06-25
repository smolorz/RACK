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
package rack.navigation;

import java.io.*;

import rack.main.tims.*;

public class PositionUtmDataMsg extends TimsMsg
{
    public int      zone;           // utm zone
    public double   northing;       // mm
    public double   easting;        // mm
    public int      altitude;       // mm over mean sea level
    public float    heading;        // rad
    
    public int getDataLen()
    {
        return 28;
    }

    public PositionUtmDataMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public PositionUtmDataMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if (type == PositionProxy.MSG_POSITION_UTM &&
            msglen == HEAD_LEN + getDataLen())
        {
            return true;
        }
        else
            return false;
    }

    public void readTimsMsgBody(InputStream in) throws IOException
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

        zone        = dataIn.readInt();
        northing    = dataIn.readDouble();
        easting     = dataIn.readDouble();
        altitude    = dataIn.readInt();
        heading     = dataIn.readFloat();

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(zone);
        dataOut.writeDouble(northing);
        dataOut.writeDouble(easting);
        dataOut.writeInt(altitude);
        dataOut.writeFloat(heading);
    }
    
    public static boolean isString(String utmString)
    {
        return utmString.startsWith("32U");
    }
    
    public void fromString(String utmString) throws NumberFormatException
    {
        String token[] = utmString.split(" ");
        
        if (token.length == 5)
        {
            String e = token[1].trim();
            if(e.endsWith("m"))
            {
                e = e.substring(0, e.length() - 1);
            }
            easting = Double.parseDouble(e) * 1000.0;

            String n = token[3].trim();
            if(n.endsWith("m"))
            {
                n = n.substring(0, n.length() - 1);
            }
            northing = Double.parseDouble(n) * 1000.0;
        }
        else
        {
            throw new NumberFormatException("String is not in UTM format");
        }
    }
    
    public String toString()
    {
        double e = (double)((int)(easting / 10.0)) / 100.0;
        double n = (double)((int)(northing / 10.0)) / 100.0;
        return "32U " + e + "m E" + n + "m N ";
    }
}
