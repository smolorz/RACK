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

import rack.main.tims.*;

public class ServoDriveMoveVelMsg extends TimsMsg
{
    public float vel = 0;

    public int getDataLen()
    {
        return 4;
    }

    public ServoDriveMoveVelMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public ServoDriveMoveVelMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if ((type == ServoDriveProxy.MSG_SERVO_DRIVE_MOVE_VEL)
                && (msglen == HEAD_LEN + getDataLen()))
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

        vel = dataIn.readFloat();

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeFloat(vel);
    }
}
