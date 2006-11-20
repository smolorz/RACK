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


public class DatalogLogInfoMsg extends TimsMsg
{
    public int              logNum = 0;
    public DatalogLogInfo[] logInfo = new DatalogLogInfo[0];

    public int getDataLen()
    {
        return (4 + logNum * DatalogLogInfo.getDataLen());
    }

    public DatalogLogInfoMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public DatalogLogInfoMsg(TimsDataMsg p) throws TimsException
    {
        readTimsDataMsg(p);
    }

    protected boolean checkTimsMsgHead()
    {
  //      System.out.println("msglen="+msglen+", HEAD_LEN + getDataLen="+HEAD_LEN + getDataLen());
        if ((type == DatalogProxy.MSG_DATALOG_LOG_STATUS) |
           (type == RackProxy.MSG_DATA))
//        if (msglen == HEAD_LEN + getDataLen())        
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

        logNum = dataIn.readInt();
        logInfo = new DatalogLogInfo[logNum];
        
        for (int i = 0; i < logNum; i++)
        {
            logInfo[i] = new DatalogLogInfo(dataIn);
        }
        bodyByteorder = BIG_ENDIAN;
    }
    
    protected void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);
        dataOut.writeInt(logNum);

        for (int i = 0; i < logNum; i++)
        {
            logInfo[i].writeData(dataOut);
        }
    }       
}


