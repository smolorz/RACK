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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
package rack.main.tims;

import java.io.*;

public class TimsRawMsg extends TimsMsg
{
    public byte[] body = new byte[0];

    public TimsRawMsg()
    {
        msglen  = HEAD_LEN;
    }

    public TimsRawMsg(InputStream in) throws IOException
    {
        readTimsMsg(in);
    }

    public TimsRawMsg(TimsMsg m, InputStream in) throws IOException
    {
        // copy message head
        headByteorder   = m.headByteorder;
        bodyByteorder   = m.bodyByteorder;
        priority        = m.priority;
        seqNr           = m.seqNr;
        type            = m.type;
        dest            = m.dest;
        src             = m.src;
        msglen          = m.msglen;
        
        readTimsMsgBody(in);
    }

    public int getDataLen()
    {
        return body.length;
    }

    public boolean checkTimsMsgHead()
    {
        return true;
    }

    public void readTimsMsgBody(InputStream in) throws IOException
    {
        if ((msglen - HEAD_LEN) > 0)
        {
            DataInputStream dataIn = new DataInputStream(in);
            body = new byte[msglen - HEAD_LEN];
            dataIn.readFully(body);
        }
        else
        {
            body = new byte[0];
        }
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        out.write(body);
    }
}
