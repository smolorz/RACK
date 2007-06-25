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

/*  TiMS message head
    uint8_t       flags;     // 1 Byte: flags
    int8_t        type;      // 1 Byte: Message Type
    uint8_t       priority;  // 1 Byte: Priority
    uint8_t       seq_nr;    // 1 Byte: Sequence Number
    uint32_t      dest;      // 4 Byte: Destination ID
    uint32_t      src;       // 4 Byte: Source ID
    uint32_t      msglen;    // 4 Byte: length of complete message
    uint8_t       data[0];   // 0 Byte: following data
*/

public abstract class TimsMsg
{
    public static final int    BIG_ENDIAN    = 0;         // network byteorder
    public static final int    LITTLE_ENDIAN = 1;         // Intel byteorder

    public static final int    HEAD_LEN      = 16;        // length of message-head

    public int                 headByteorder = BIG_ENDIAN;
    public int                 bodyByteorder = BIG_ENDIAN;

    public byte                type          = 0;
    public byte                priority      = 0;         // 0 = less important message
    public byte                seqNr         = 0;
    public int                 dest          = 0;
    public int                 src           = 0;
    protected int              msglen        = 0;

    public TimsMsg()
    {
        msglen = HEAD_LEN;
    }

    public TimsMsg(TimsRawMsg m) throws TimsException
    {
        readTimsRawMsg(m);
    }

    public TimsMsg(InputStream in) throws IOException
    {
        readTimsMsg(in);
    }

    public void readTimsMsg(InputStream in) throws IOException
    {
        readTimsMsgHead(in);

        if (checkTimsMsgHead() == false)
        {
            // head doesn't fit to TimsMsg class, save message as TimsRawMsg
            new TimsRawMsg(this, in);
            throw new IOException("Message head doesn't fit (" + toString() + ")");
        }
        readTimsMsgBody(in);
    }

    public void readTimsRawMsg(TimsRawMsg m) throws TimsException
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

        if (checkTimsMsgHead() == false)
        {
            throw (new TimsException("Message head doesn't fit (" + toString() + ")"));
        }

        try
        {
            readTimsMsgBody(new ByteArrayInputStream(m.body));
        }
        catch (IOException e)
        {
            throw (new TimsException("Message body doesn't fit (" + toString() + ")"));
        }
    }

    public void writeTimsMsg(OutputStream out) throws IOException
    {
        msglen = HEAD_LEN + getDataLen();

        writeTimsMsgHead(out);
        writeTimsMsgBody(out);
        out.flush();
    }

    public void readTimsMsgHead(InputStream in) throws IOException
    {
        int flags = in.read();

        headByteorder = (flags) & 0x01;
        bodyByteorder = (flags >> 1) & 0x01;

        EndianDataInputStream dataIn;

        if (headByteorder == BIG_ENDIAN)
        {
            dataIn = new BigEndianDataInputStream(in);
        }
        else
        {
            dataIn = new LittleEndianDataInputStream(in);
        }

        type        = dataIn.readByte();
        priority    = dataIn.readByte();
        seqNr       = dataIn.readByte();
        dest        = dataIn.readInt();
        src         = dataIn.readInt();
        msglen      = dataIn.readInt();

        headByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgHead(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        int flags = headByteorder + bodyByteorder * 0x01;

        dataOut.writeByte(flags);
        dataOut.writeByte(type);
        dataOut.writeByte(priority);
        dataOut.writeByte(seqNr);
        dataOut.writeInt(dest);
        dataOut.writeInt(src);
        dataOut.writeInt(msglen);
    }

    public abstract boolean checkTimsMsgHead();

    public abstract void readTimsMsgBody(InputStream in) throws IOException;

    public abstract void writeTimsMsgBody(OutputStream out) throws IOException;

    public abstract int getDataLen();

    public String toString()
    {
        return (Integer.toHexString(src) + " -> " + Integer.toHexString(dest) + " type: " + type + " msglen: " + msglen);
    }

    /**
     * TimsMsgs are equal if the src, dest and type are equal.
     * 
     * @param o
     *            TimsMsg to compare to.
     * @return true if the specified Object is a TimsMsg with the same src, dest and type
     */
    public boolean equals(Object o)
    {
        try
        {
            TimsMsg msg = (TimsMsg) o;

            return ((src == msg.src) &&
                    (dest == msg.dest) &&
                    (type == msg.type));
        }
        catch (Exception e)
        {
            return false;
        }
    }
}
