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

import rack.main.RackProxy;
import rack.main.tims.*;

public class PositionWgs84DataMsg extends TimsMsg
{
    public double   latitude;       // rad
    public double   longitude;      // rad
    public int      altitude;       // mm over mean sea level
    public float    heading;        // rad

    public int getDataLen()
    {
        return (24);
    }

    public PositionWgs84DataMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public PositionWgs84DataMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if (type == RackProxy.MSG_DATA &&
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

        latitude    = dataIn.readDouble();
        longitude   = dataIn.readDouble();
        altitude    = dataIn.readInt();
        heading     = dataIn.readFloat();

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeDouble(latitude);
        dataOut.writeDouble(longitude);
        dataOut.writeInt(altitude);
        dataOut.writeFloat(heading);
    }
}
