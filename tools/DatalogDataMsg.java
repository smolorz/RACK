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
 *      Oliver Wulf        <oliver.wulf@gmx.de>
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 */
package rack.tools;

import java.io.*;

import rack.main.RackProxy;
import rack.main.tims.*;
import rack.tools.DatalogLogInfo;


public class DatalogDataMsg extends TimsMsg
{
    public int				recordingTime = 0;
    public String           logPathName = "";    
    public int              logNum = 0;
    public DatalogLogInfo[] logInfo = new DatalogLogInfo[0];

    public int getDataLen()
    {
        return (8 + 40 + logNum * DatalogLogInfo.getDataLen());
    }

    public DatalogDataMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public DatalogDataMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if ((type == DatalogProxy.MSG_DATALOG_LOG_STATUS) |
           (type == RackProxy.MSG_DATA))
        {
            return(true);
        }
        else
        {
            return(false);
        }
    }

    public void readTimsMsgBody(InputStream in) throws IOException
    {
        byte[] name = new byte[40];
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

        dataIn.readFully(name);
        logPathName   = new String(name);
        
        logNum        = dataIn.readInt();
        logInfo       = new DatalogLogInfo[logNum];
        
        for (int i = 0; i < logNum; i++)
        {
            logInfo[i] = new DatalogLogInfo(dataIn);
        }
        bodyByteorder = BIG_ENDIAN;
    }
    
    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);
        
        dataOut.writeInt(recordingTime);
        dataOut.writeBytes(logPathName);
        
        for (int i = 0; i < (40 - logPathName.length()); i++)
        {
            dataOut.writeByte(0);
        }

        dataOut.writeInt(logNum);

        for (int i = 0; i < logNum; i++)
        {
            logInfo[i].writeData(dataOut);
        }
    }       
}


