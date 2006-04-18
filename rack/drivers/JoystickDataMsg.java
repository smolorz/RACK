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
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;
import rack.main.tims.streams.*;
import rack.main.defines.Position3D;

public class JoystickDataMsg extends TimsMsg
{
    public int recordingtime = 0;
    public Position3D position = new Position3D(); // 24 bytes
    public int buttons = 0;

    public int getDataLen()
    {
        return ( 4 + Position3D.getDataLen() + 4);
    }

    public JoystickDataMsg()
    {
        type = RackMsgType.MSG_DATA;
        msglen = headLen + getDataLen();
    }

    public JoystickDataMsg(TimsDataMsg p) throws MsgException
    {
        readTimsDataMsg(p);
    }

    protected boolean checkTimsMsgHead()
    {
        if ((type == RackMsgType.MSG_DATA)
                && (msglen == headLen + getDataLen()))
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
        recordingtime = dataIn.readInt();
        position.readData(dataIn);
        buttons = dataIn.readInt();

        bodyByteorder = BIG_ENDIAN;
    }

    protected void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(recordingtime);
        position.writeData(dataOut);
        dataOut.writeInt(buttons);
    }

}
