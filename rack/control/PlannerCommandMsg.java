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
 *      Daniel Lecking  <lecking@rts.uni-hannover.de>
 *
 */
package rack.control;

import java.io.*;

import rack.main.tims.*;


public class PlannerCommandMsg extends TimsMsg
{
    public int commandNum = 0;
    public PlannerString[] command;

    public int getDataLen()
    {
        return (4 + commandNum * PlannerString.getDataLen());
    }

    public PlannerCommandMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public PlannerCommandMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        return (true);
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

        bodyByteorder = BIG_ENDIAN;

        commandNum = dataIn.readInt();
        command = new PlannerString[commandNum];

        for (int i = 0; i < commandNum; i++)
        {
            command[i] = new PlannerString(dataIn);
        }
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(commandNum);
        for (int i = 0; i < commandNum; i++)
        {
            command[i].writeData(dataOut);

        }
    }

}