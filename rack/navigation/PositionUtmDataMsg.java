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

/**
 * UTM lateral band A...Z without I and O. Band >= N is northern hemisphere
 */
public class PositionUtmDataMsg extends TimsMsg
{
	public enum PositionUtmBand
	{
	    A, B, C, D, E, F, G, H, J, K, L, M, N, P, Q, R, S, T, U, V, W, X, Y, Z
	}

    public long             northing;       // mm
    public long             easting;        // mm
    public int              zone;           // utm zone
    public PositionUtmBand  band;           // see PositionUtmBand
    public int              altitude;       // mm over mean sea level
    public float            heading;        // rad
    
    public int getDataLen()
    {
        return 32;
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

        northing    = dataIn.readLong();
        easting     = dataIn.readLong();
        zone        = dataIn.readInt();
        int b       = dataIn.readInt();
        altitude    = dataIn.readInt();
        heading     = dataIn.readFloat();

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeDouble(northing);
        dataOut.writeDouble(easting);
        dataOut.writeInt(zone);
        dataOut.writeInt(altitude);
        dataOut.writeInt(0);
        dataOut.writeFloat(heading);
    }

    public boolean isString(String utmString)
    {
        return utmString.startsWith("32U");
    }

    public void fromString(String utmString) throws NumberFormatException
    {
        String token[] = utmString.split(" ");

        if (token.length == 3)
        {
            String e = token[1].trim();
            e = e.substring(0, e.length() - 1);
            easting = (long)(Double.parseDouble(e) * 1000.0);

            String n = token[2].trim();
            n = n.substring(0, n.length() - 1);
            northing = (long)(Double.parseDouble(n) * 1000.0);
        }
        else
        {
            throw new NumberFormatException("String is not in UTM format");
        }
    }

    public String toString()
    {
        double e = (double)easting / 1000.0;
        double n = (double)northing / 1000.0;
        return zone+"U " + e + "m E " + n + "m N ";
    }
}
