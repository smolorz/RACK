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
package rack.main.tims.msg;

import rack.main.tims.exceptions.*;

import java.io.*;

public class TimsMsg0 extends TimsMsg
{

    public int getDataLen()
    {
        return 0;
    }

    public TimsMsg0()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public TimsMsg0(TimsDataMsg p) throws MsgException
    {
        readTimsDataMsg(p);
    }

    protected boolean checkTimsMsgHead()
    {
        if (msglen == HEAD_LEN + getDataLen() )
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
        bodyByteorder = BIG_ENDIAN;
    }

    protected void writeTimsMsgBody(OutputStream out) throws IOException
    {
    }
}
