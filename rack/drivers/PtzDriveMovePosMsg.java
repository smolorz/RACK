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

public class PtzDriveMovePosMsg extends TimsMsg
{
    public float posPan  = 0;
    public float posTilt = 0;
    public float posZoom = 0;
    public float velPan	 = 0;
    public float velTilt = 0;
    public float velZoom = 0;
    public float accPan  = 0;
    public float accTilt = 0;
    public float accZoom = 0;
    public int replyPosReached = 0;

    public int getDataLen()
    {
        return (40);
    }

    public PtzDriveMovePosMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public PtzDriveMovePosMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if ((type == PtzDriveProxy.MSG_PTZ_DRIVE_MOVE_POS)
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

        posPan	 = dataIn.readFloat();
        posTilt  = dataIn.readFloat();
        posZoom	 = dataIn.readFloat();
        velPan   = dataIn.readFloat();
        velTilt  = dataIn.readFloat();
        velZoom  = dataIn.readFloat();
        accPan   = dataIn.readFloat();
        accTilt  = dataIn.readFloat();
        accZoom  = dataIn.readFloat();
        replyPosReached = dataIn.readInt();
        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeFloat(posPan);
        dataOut.writeFloat(posTilt);
        dataOut.writeFloat(posZoom);
        dataOut.writeFloat(velPan);
        dataOut.writeFloat(velTilt);
        dataOut.writeFloat(velZoom);
        dataOut.writeFloat(accPan);
        dataOut.writeFloat(accTilt);
        dataOut.writeFloat(accZoom);
        dataOut.writeInt(replyPosReached);
    }
}
