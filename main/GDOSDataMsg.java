/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.main;

import java.io.*;

import rack.main.tims.*;

/** Zum Empfang von GDOS-Nachrichten. */
public class GDOSDataMsg extends TimsMsg
{
    public String message;

    public int getDataLen()
    {
        return msglen - HEAD_LEN;
    }

    public GDOSDataMsg(String message)
    {
        this.message = message;
        this.msglen = HEAD_LEN + message.length();
    }

    public GDOSDataMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public GDOSDataMsg(TimsRawMsg p) throws TimsException
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
        int dataLen = msglen - HEAD_LEN;
        int stringLen = dataLen;

        if (bodyByteorder == BIG_ENDIAN)
        {
            dataIn = new BigEndianDataInputStream(in);
        }
        else
        {
            dataIn = new LittleEndianDataInputStream(in);
        }

        bodyByteorder = BIG_ENDIAN;
        byte[] byteArray = new byte[dataLen];

        for (int i = 0; i < dataLen; i++)
        {
            byteArray[i] = dataIn.readByte();
            if (byteArray[i] == 0)
            {
                stringLen = i;
                break;
            }
        }

        message = "";

        for (int i = 0; i < stringLen; i++)
        {
            if (byteArray[i] == '%')
            {
                i++;
                switch (byteArray[i])
                {
                    case 'd': // integer with decimal output
                    case 'i': // integer with decimal output
                    case 'u': // unsigned integer with decimal output
                        message = message + Integer.toString(dataIn.readInt());
                        break;
                    case 'x': // integer with hexadecimal output
                        message = message
                                + Integer.toHexString(dataIn.readInt())
                                        .toLowerCase();
                        break;
                    case 'X': // integer with hexadeximal output capital
                                // letters
                        message = message
                                + Integer.toHexString(dataIn.readInt())
                                        .toUpperCase();
                        break;
                    case 'p': // integer with pointer val
                        message = message
                                + "0x"
                                + Integer.toHexString(dataIn.readInt())
                                        .toLowerCase();
                        break;
                    case 'P': // integer with pointer val
                        message = message
                                + "0x"
                                + Integer.toHexString(dataIn.readInt())
                                        .toUpperCase();
                        break;
                    case 'b': // integer with binary output
                        message = message
                                + Integer.toBinaryString(dataIn.readInt());
                        break;
                    case 'n': // mbx name string output
                        message = message
                                + RackName.nameString(dataIn.readInt());
                        break;
                    case 'f': // double, floatingpoint output
                        message = message
                                + Math.rint(dataIn.readDouble() * 1000.0)
                                / 1000.0;
                        break;
                    case 'a': // angle given in rad, output in deg
                        message = message
                                + Math.rint(Math.toDegrees(
                                        dataIn.readDouble()) * 100.0) / 100.0
                                + "";
                        break;
/*
TODO
                    case 's':   // string
                        StringBuffer buffer = new StringBuffer();
                        byte ch;
                        while ((ch = dataIn.readByte()) != 0) {
                            buffer.append(ch);
                        }
                        message = message
                                + buffer.toString();
                        break;
*/
                    case '%': // print character %
                        message = message + '%';
                        break;
                    case 'L': // 64Bit integer for d,i,u,x,X
                        message = "Parameter %L is not yet implemented \""
                                + new String(byteArray, 0, stringLen) + "\"";
                        return;

                    default:
                        message = "Unknown parameter %"
                                + new String(byteArray, i, 1) + " \""
                                + new String(byteArray, 0, stringLen) + "\"";
                        return;
                }

            }
            else if ((byteArray[i] != 10) & (byteArray[i] != 13))
            {
                message = message + new String(byteArray, i, 1);
            }
        }
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        byte[] byteArray = new byte[message.length()];
        byteArray = message.getBytes();
        dataOut.write(byteArray);
    }
}