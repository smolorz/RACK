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
 *      Marko Reimer     <reimer@l3s.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.drivers;

import java.io.*;
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;
import rack.main.tims.streams.*;

public class CameraFormatMsg extends TimsMsg
{
    public int width = 0;
    public int height = 0;
    public int depth = 0;
    public int mode = 0;

    public int getDataLen()
    {
        return (16);
    }

    public CameraFormatMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public CameraFormatMsg(TimsDataMsg p) throws MsgException
    {
        msglen = HEAD_LEN + getDataLen();
        readTimsDataMsg(p);
    }

    public CameraFormatMsg(int width, int height)
    {
        msglen = HEAD_LEN + getDataLen();
        this.width = width;
        this.height = height;
        this.mode = -1;
        this.depth = -1;
    }

    public CameraFormatMsg(int mode)
    {
        msglen = HEAD_LEN + getDataLen();
        switch (mode)
        {
            case CameraDataMsg.CAMERA_MODE_MONO8:
                this.mode = mode;
                this.depth = 8;
                break;
            case CameraDataMsg.CAMERA_MODE_MONO12:
                this.mode = mode;
                this.depth = 16;
                break;
            case CameraDataMsg.CAMERA_MODE_MONO16:
                this.mode = mode;
                this.depth = 16;
                break;
            case CameraDataMsg.CAMERA_MODE_RGB24:
                this.mode = mode;
                this.depth = 24;
                break;
            case CameraDataMsg.CAMERA_MODE_YUV422:
                this.mode = mode;
                this.depth = 16;
                break;
            case CameraDataMsg.CAMERA_MODE_RAW8:
                this.mode = mode;
                this.depth = 8;
                break;
            case CameraDataMsg.CAMERA_MODE_RAW12:
                this.mode = mode;
                this.depth = 16;
                break;
            case CameraDataMsg.CAMERA_MODE_RAW16:
                this.mode = mode;
                this.depth = 16;
                break;
            case CameraDataMsg.CAMERA_MODE_RGB565:
                this.mode = mode;
                this.depth = 16;
                break;
            default:
                this.mode = -1;
                this.depth = -1;
                break;
        }
        this.width = -1;
        this.height = -1;
    }

    protected boolean checkTimsMsgHead()
    {
        if (type == RackMsgType.MSG_DATA)
        {
            return true;
        }
        else
            return false;
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
        width = dataIn.readInt();
        height = dataIn.readInt();
        depth = dataIn.readInt();
        mode = dataIn.readInt();

        bodyByteorder = BIG_ENDIAN;
    }

    protected void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(width);
        dataOut.writeInt(height);
        dataOut.writeInt(depth);
        dataOut.writeInt(mode);
    }

    public String toString()
    {
        return "width " + width + " height " + height + " mode " + mode
                + " depth " + depth;
    }
}
