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
package rack.drivers;

import java.io.*;

import rack.main.RackProxy;
import rack.main.tims.*;
import rack.main.defines.Position3d;

public class JoystickDataMsg extends TimsMsg
{
    public int recordingTime = 0;
    public Position3d position = new Position3d(); // 24 bytes
    public int buttons = 0;

    public int getDataLen()
    {
        return ( 4 + Position3d.getDataLen() + 4);
    }

    public JoystickDataMsg()
    {
        type = RackProxy.MSG_DATA;
        msglen = HEAD_LEN + getDataLen();
    }

    public JoystickDataMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    protected boolean checkTimsMsgHead()
    {
        if ((type == RackProxy.MSG_DATA)
                && (msglen == HEAD_LEN + getDataLen()))
        {
            return (true);
        }
        else
        {
            return (false);
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
        recordingTime = dataIn.readInt();
        position.readData(dataIn);
        buttons = dataIn.readInt();

        bodyByteorder = BIG_ENDIAN;
    }

    protected void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(recordingTime);
        position.writeData(dataOut);
        dataOut.writeInt(buttons);
    }

}
