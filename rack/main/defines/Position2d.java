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
import java.text.DecimalFormat;

import rack.main.tims.EndianDataInputStream;

public class Position2d implements Serializable, Cloneable
{
    /** Reihenfolge im Datenpacket: x, y, rho */
    public int   x   = 0;
    public int   y   = 0;
    public float rho = 0.0f;

    private static final long serialVersionUID = 1L;
    
    static public int getDataLen()
    {
        return (12);
    }
    
    /**
     * Constructor Position2D.
     * @param x
     * @param y
     * @param rho
     */
    public Position2d(int x, int y, float rho) 
    {
        this.x = x;
        this.y = y;
        this.rho = rho;
    }
  
    public Position2d(Position3d position3d)
    {
        this.x   = position3d.x;
        this.y   = position3d.y;
        this.rho = position3d.rho;
    }

    /**
     * Constructor Position2D.
     */
    public Position2d() 
    {
        this.x   = 0;
        this.y   = 0;
        this.rho = 0.0f;
    }

    /**
     * read Position2D.
     */
    public void readData(EndianDataInputStream dataIn) throws IOException 
    {
        x   = dataIn.readInt();
        y   = dataIn.readInt();
        rho = dataIn.readFloat();
    }
  
    /**
     * write Position2D.
     */
    public void writeData(DataOutputStream dataOut) throws IOException 
    {
        dataOut.writeInt(x);
        dataOut.writeInt(y);
        dataOut.writeFloat(rho);
    }
  
    public String toString()
    {
        DecimalFormat rhoFormat = new DecimalFormat("0.0");
        return("x: "     + x + ", y: " + y + ", rho: " + 
               rhoFormat.format(Math.toDegrees(rho)));
    }

    /** Method getDistance
     * @param p2
     * @return distance to p2
     */
    public int getDistance(Position2d p2) 
    {
        return((int) Math.round(Math.sqrt((x - p2.x)*(x - p2.x) + 
                                          (y - p2.y)*(y - p2.y))));
    }

    /** Method getDistance
     * @param p2
     * @return distance to p2
     */
    public int getDistance(Point2d p2) 
    {
        return((int) Math.round(Math.sqrt((x - p2.x)*(x - p2.x) + 
                                          (y - p2.y)*(y - p2.y))));
    }

    /** Method coordTrafo
     * @param ang  - Drehwinkel
     * @return Punkt in neuen Koordinaten
     */
    public Position2d coordTrafo(float ang) 
    {
        double s = Math.sin(ang);
        double c = Math.cos(ang);
        
        return new Position2d((int) Math.round(x * c - y * s),
                              (int) Math.round(x * s + y * c), rho - ang);
    }

    /** Method coordTrafo
     * @param ang  - Drehwinkel
     * @param p0 - translatorische Verschiebung
     * @return Punkt in neuen Koordinaten
     */
    public Position2d coordTrafo(float ang, Position2d p0) 
    {
        double s = Math.sin(ang);
        double c = Math.cos(ang);
        
        return new Position2d((int) Math.round(x * c - y * s) + p0.x,
                              (int) Math.round(x * s + y * c) + p0.y, 
                              rho - ang);
    }

    /** Method coordTrafo
     * @param ang  - Drehwinkel
     * @param ptransX - translatorische Verschiebung x
     * @param ptransY - translatorische Verschiebung y
     * @return Punkt in neuen Koordinaten
     */
    public Position2d coordTrafo(float ang, int ptransX, int ptransY) 
    {
        double s = Math.sin(ang);
        double c = Math.cos(ang);
        
        return new Position2d((int) Math.round(x * c - y * s) + ptransX,
                              (int) Math.round(x * s + y * c) + ptransY, 0.0f);
    }

    // das hab ich so gemacht, weil ich es nicht besser wusste. wer mehr 
    // ahnung vom klonen hat, moege es gegebenenfalls korrigieren.   
    // arne, 5.2.2004
    public Object clone()
    {
        return ((Object) new Position2d(x, y, rho));
    }
}
