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

import rack.main.tims.EndianDataInputStream;

public class Position3d
{
    /** Position, Reihenfolge im Datenpacket: x, y, z, phi, psi, rho */
    public int x = 0; // 4 Byte
    public int y = 0; // 4 Byte
    public int z = 0; // 4 Byte
    public float phi = 0.0f; // 4 Byte
    public float psi = 0.0f; // 4 Byte
    public float rho = 0.0f; // 4 Byte ==> 24 Byte

    public Position3d(int x, int y, int z, float phi, float psi, float rho)
    {
        this.x = x;
        this.y = y;
        this.z = z;
        this.phi = phi;
        this.psi = psi;
        this.rho = rho;
    }

    public Position3d(Position2d position2d)
    {
        this.x   = position2d.x;
        this.y   = position2d.y;
        this.z   = 0;
        this.phi = 0.0f;
        this.psi = 0.0f;
        this.rho = position2d.rho;
    }

    public Position3d()
    {
        this.x = 0;
        this.y = 0;
        this.z = 0;
        this.phi = 0.0f;
        this.psi = 0.0f;
        this.rho = 0.0f;
    }
    
    static public int getDataLen()
    {
        return (24);
    }

    public void readData(EndianDataInputStream dataIn) throws IOException 
    {
        x   = dataIn.readInt();
        y   = dataIn.readInt();
        z   = dataIn.readInt();
        phi = dataIn.readFloat();
        psi = dataIn.readFloat();
        rho = dataIn.readFloat();
    }

    public void writeData(DataOutputStream dataOut) throws IOException 
    {
        dataOut.writeInt(x);
        dataOut.writeInt(y);
        dataOut.writeInt(z);
        dataOut.writeFloat(phi);
        dataOut.writeFloat(psi);
        dataOut.writeFloat(rho);        
    }    
}
