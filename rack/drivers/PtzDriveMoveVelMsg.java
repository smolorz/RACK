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
 *      Matthias Hentschel  <hentschel@rts.uni-hannover.de>
 *
 */
package rack.drivers;

import java.io.*;

import rack.main.tims.*;

public class PtzDriveMoveVelMsg extends TimsMsg
{
    public float velPan  = 0;
    public float velTilt = 0;
    public float velZoom = 0;

    public int getDataLen()
    {
        return 12;
    }

    public PtzDriveMoveVelMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public PtzDriveMoveVelMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if ((type == PtzDriveProxy.MSG_PTZ_DRIVE_MOVE_VEL)
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

        velPan  = dataIn.readFloat();
        velTilt = dataIn.readFloat();
        velZoom = dataIn.readFloat();
        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeFloat(velPan);
        dataOut.writeFloat(velTilt);
        dataOut.writeFloat(velZoom);
    }
}
