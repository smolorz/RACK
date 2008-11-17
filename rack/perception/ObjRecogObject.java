/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2007 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Marko Reimer <reimer@rts.uni-hannover.de>
 *
 */
package rack.perception;

import java.io.*;

import rack.main.tims.*;
import rack.main.defines.*;

/**
 * @author wulf
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class ObjRecogObject
{

    public int objectId = 0;
    public Position3d pos = new Position3d();
    public Position3d vel = new Position3d();
    public Point3d    dim = new Point3d();
    public float     prob = 0.0f;
    public ImageRect imageArea = new ImageRect();
 
    public static int getDataLen()
    {
        return (4 + 4 + 2 * Position3d.getDataLen() + ImageRect.getDataLen()+ Point3d.getDataLen());
    }
    
    public ObjRecogObject()
    {
    }
    
    /**
     * @param dataIn
     */
    public ObjRecogObject(EndianDataInputStream dataIn)
            throws IOException
    {
        objectId = dataIn.readInt();
        pos.readData(dataIn);
        vel.readData(dataIn);
        dim.readData(dataIn);
        prob = dataIn.readFloat();
        imageArea.readData(dataIn);
    }

    /**
     * @param dataOut
     */
    public void writeDataOut(DataOutputStream dataOut) throws IOException
    {
        dataOut.writeInt(objectId);
        pos.writeData(dataOut);
        vel.writeData(dataOut);
        dim.writeData(dataOut);
        dataOut.writeFloat(prob);
        imageArea.writeData(dataOut);
    }


    public String toString()
    {
        return objectId + " pos " + pos + " vel " + vel + " dim " + dim + " prob " + prob + " imageArea " + imageArea;
    }
}
