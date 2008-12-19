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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
package rack.main.defines;

import java.io.DataOutputStream;
import java.io.IOException;

import rack.main.tims.EndianDataInputStream;

public class Waypoint2d
{
    public int  x           = 0;
    public int  y           = 0;
    public int  speed       = 0;
    public int  maxRadius   = 0;
    public int  type        = 0;
    public int  request     = 0;
    public int  lbo         = 0;
    public int  id          = 0;
    public int  wayId       = 0;

    static public int getDataLen()
    {
        return (72);
    }

    public Waypoint2d()
    {
    }

    public Waypoint2d(int x, int y, int speed, int maxRadius, int type,
                      int request, int lbo, int id, int wayId)
    {
        this.x          = x;
        this.y          = y;
        this.speed      = speed;
        this.maxRadius  = maxRadius;
        this.type       = type;
        this.request    = request;
        this.lbo        = lbo;
        this.id         = id;
        this.wayId      = wayId;
    }

    public void readData(EndianDataInputStream dataIn) throws IOException
    {
        x           = dataIn.readInt();
        y           = dataIn.readInt();
        speed       = dataIn.readInt();
        maxRadius   = dataIn.readInt();
        type        = dataIn.readInt();
        request     = dataIn.readInt();
        lbo         = dataIn.readInt();
        id          = dataIn.readInt();
        wayId       = dataIn.readInt();
    }

    public void writeData(DataOutputStream dataOut) throws IOException
    {
        dataOut.writeInt(x);
        dataOut.writeInt(y);
        dataOut.writeInt(speed);
        dataOut.writeInt(maxRadius);
        dataOut.writeInt(type);
        dataOut.writeInt(request);
        dataOut.writeInt(lbo);
        dataOut.writeInt(id);
        dataOut.writeInt(wayId);
    }
}
