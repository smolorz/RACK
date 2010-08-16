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

public class PlannerDataMsg extends TimsMsg
{
    public int recordingTime = 0;
    public int state = 0;
    public int messageNum = 0;
    public PlannerString[] message;

    public String messageString = "";

    public int getDataLen()
    {
    	return (12 + messageNum * PlannerString.getDataLen());
    }

    public PlannerDataMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public PlannerDataMsg(TimsRawMsg p) throws TimsException
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

        recordingTime = dataIn.readInt();
        state = dataIn.readInt();
        messageNum = dataIn.readInt();

        message = new PlannerString[messageNum];

        for (int i = 0; i < messageNum; i++)
        {
            message[i] = new PlannerString(dataIn);
        }
 //       messageString = new String(message);
 //       messageString = messageString.trim();
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(recordingTime);
        dataOut.writeInt(state);
        dataOut.writeInt(messageNum);
        for (int i = 0; i < messageNum; i++)
        {
            message[i].writeData(dataOut);

        }
    }
}