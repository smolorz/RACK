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
 *      Joerg Langenberg  <joerg.langenberg@gmx.net>
 *
 */
package rack.drivers;

import java.io.*;

import rack.main.RackProxy;
import rack.main.tims.*;

public class GyroDataMsg extends TimsMsg
{
    public int       recordingTime = 0;
    public float     roll          = 0;
    public float     pitch         = 0;
    public float     yaw           = 0;
    public float     aX            = 0;
    public float     aY            = 0;
    public float     aZ            = 0;
    public float     wRoll         = 0;
    public float     wPitch        = 0;
    public float     wYaw          = 0;


    public int getDataLen()
    {
        return 40;

    }

    public GyroDataMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public GyroDataMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if (type == RackProxy.MSG_DATA)
        {
            return (true);
        }
        else
        {
            return (false);
        }
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

        recordingTime = dataIn.readInt();
        roll          = dataIn.readFloat();
        pitch         = dataIn.readFloat();
        yaw           = dataIn.readFloat();
        aX            = dataIn.readFloat();
        aY            = dataIn.readFloat();
        aZ            = dataIn.readFloat();
        wRoll         = dataIn.readFloat();
        wPitch        = dataIn.readFloat();
        wYaw          = dataIn.readFloat();

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(recordingTime);
        dataOut.writeFloat(roll);
        dataOut.writeFloat(pitch);
        dataOut.writeFloat(yaw);
        dataOut.writeFloat(aX);
        dataOut.writeFloat(aY);
        dataOut.writeFloat(aZ);
        dataOut.writeFloat(wRoll);
        dataOut.writeFloat(wPitch);
        dataOut.writeFloat(wYaw);
    }
}
