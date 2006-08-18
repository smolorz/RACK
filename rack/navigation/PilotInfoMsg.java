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
package rack.navigation;

import java.io.*;

import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;
import rack.main.tims.streams.*;

import rack.main.defines.Position3D;
import rack.main.defines.PolarSpline;

/** Paket zur Uebertragung von Splinedaten.
 *
 * Datenreihenfolge: recordingTime, splineNum, PolarSpline
 */

public class PilotInfoMsg extends TimsMsg
{
    public int           recordingtime = 0;
    public Position3D    pos = new Position3D(0,0,0,0.0f,0.0f,0.0f);
    public int           speed = 0;
    public float         curve = 0;
    public int           splineNum = 0;
    public PolarSpline[] spline = new PolarSpline[0];

    public int getDataLen()
    {
        return (16 + Position3D.getDataLen() +
                splineNum * PolarSpline.getDataLen());
    }

    public PilotInfoMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public PilotInfoMsg(TimsDataMsg p) throws MsgException
    {
        readTimsDataMsg(p);
    }

    protected boolean checkTimsMsgHead()
    {
        if (type == RackMsgType.MSG_DATA)
        {
            return(true);
        }
        else
        {
            return(false);
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

        recordingtime = dataIn.readInt();

        pos.readData(dataIn);
        speed                     = dataIn.readInt();
        curve                     = dataIn.readFloat();
        splineNum                 = dataIn.readInt();
        spline                    = new PolarSpline[splineNum];

        for (int i = 0; i < splineNum; i++)
        {
            spline[i] = new PolarSpline(dataIn);
        }
        bodyByteorder = BIG_ENDIAN;
    }


    protected void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);
        dataOut.writeInt(recordingtime);

        pos.writeData(dataOut);
        dataOut.writeInt(speed);
        dataOut.writeFloat(curve);
        dataOut.writeInt(splineNum);

        for (int i = 0; i < splineNum; i++)
        {
            spline[i].writeData(dataOut);
        }
    }
}
