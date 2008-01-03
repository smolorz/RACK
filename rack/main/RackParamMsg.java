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
 */
package rack.main;

import java.io.*;

import rack.main.tims.*;

public class RackParamMsg extends TimsMsg
{
    public int parameterNum = 0;
    public RackParam[] parameter = new RackParam[0];

    public RackParamMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public RackParamMsg(RackParam p)
    {
        parameterNum = 1;
        parameter = new RackParam[1];
        parameter[0] = p;
    }

    public boolean checkTimsMsgHead()
    {
        if (type == RackProxy.MSG_PARAM)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    public int getDataLen()
    {
        return (4 + parameterNum * RackParam.getDataLen());
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

        parameterNum = dataIn.readInt();
        parameter = new RackParam[parameterNum];
        
        for (int i = 0; i < parameterNum; i++)
        {
            parameter[i] = new RackParam(dataIn);
        }
        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(parameterNum);
        for (int i = 0; i < parameterNum; i++)
        {
            parameter[i].writeData(dataOut);
        }
    }
    
    public String toString()
    {
        return "parameterNum " + parameterNum;
    }
}
