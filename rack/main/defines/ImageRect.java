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
package rack.main.defines;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.Serializable;

import rack.main.tims.streams.EndianDataInputStream;

public class ImageRect implements Serializable
{
    public int x = 0;
    public int y = 0;
    public int width  = 0;
    public int height = 0;

    /**
     * Data-length ImageRect.
     */
    static public int getDataLen()
    {
        return (16);
    }

    /**
     * Constructor Point2D.
     */
    public ImageRect()
    {
    }

    /**
     * Constructor ImageRect.
     * @param x
     * @param y
     * @param width
     * @param height
     */
    public ImageRect(int x, int y, int width, int height)
    {
        this.x    = x;
        this.y    = y;
        this.width     = width;
        this.height    = height;
    }

    /**
     * read ImageRect.
     */
    public void readData(EndianDataInputStream dataIn) throws IOException
    {
        x   = dataIn.readInt();
        y   = dataIn.readInt();
        width    = dataIn.readInt();
        height   = dataIn.readInt();
    }

    /**
     * write ImageRect
     */
    public void writeData(DataOutputStream dataOut) throws IOException
    {
        dataOut.writeInt(x);
        dataOut.writeInt(y);
        dataOut.writeInt(width);
        dataOut.writeInt(height);
    }

}
