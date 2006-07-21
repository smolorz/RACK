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

import rack.main.tims.streams.EndianDataInputStream;

public class Position2D implements Serializable, Cloneable
{
    /** Reihenfolge im Datenpacket: x, y, phi */
    public int   x   = 0;
    public int   y   = 0;
    public float phi = 0.0f;

    private static final long serialVersionUID = 1L;
    
    static public int getDataLen()
    {
        return (12);
    }
    
    /**
     * Constructor Position2D.
     * @param x
     * @param y
     * @param phi
     */
    public Position2D(int x, int y, float phi) 
    {
        this.x = x;
        this.y = y;
        this.phi = phi;
    }
  
    /**
     * Constructor Position2D.
     * @param x
     * @param y
     */
    public Position2D(int x, int y) 
    {
        this.x   = x;
        this.y   = y;
        this.phi = 0.0f;
    }
  
    /**
     * Constructor Position2D.
     */
    public Position2D() 
    {
        this.x   = 0;
        this.y   = 0;
        this.phi = 0.0f;
    }

    /**
     * read Position2D.
     */
    public void readData(EndianDataInputStream dataIn) throws IOException 
    {
        x   = dataIn.readInt();
        y   = dataIn.readInt();
        phi = dataIn.readFloat();
    }
  
    /**
     * write Position2D.
     */
    public void writeData(DataOutputStream dataOut) throws IOException 
    {
        dataOut.writeInt(x);
        dataOut.writeInt(y);
        dataOut.writeFloat(phi);
    }
  
    public String toString()
    {
        DecimalFormat phiFormat = new DecimalFormat("0.0");
        return("x: "     + x + ", y: " + y + ", phi: " + 
               phiFormat.format(Math.toDegrees(phi)));
    }

    /** Method getDistance
     * @param Position2D p2
     * @return distance to p2
     */
    public int getDistance(Position2D p2) 
    {
        return((int) Math.round(Math.sqrt((x - p2.x)*(x - p2.x) + 
                                          (y - p2.y)*(y - p2.y))));
    }

    /** Method getDistance
     * @param Point2D p2
     * @return distance to p2
     */
    public int getDistance(Point2D p2) 
    {
        return((int) Math.round(Math.sqrt((x - p2.x)*(x - p2.x) + 
                                          (y - p2.y)*(y - p2.y))));
    }

    /** Method coordTrafo
     * @param float ang  - Drehwinkel
     * @return Point2D   Punkt in neuen Koordinaten
     */
    public Position2D coordTrafo(float ang) 
    {
        double s = Math.sin(ang);
        double c = Math.cos(ang);
        
        return new Position2D((int) Math.round(x * c - y * s),
                              (int) Math.round(x * s + y * c), phi - ang);
    }

    /** Method coordTrafo
     * @param float ang  - Drehwinkel
     * @param Position2D p0 - translatorische Verschiebung
     * @return Position2D   Punkt in neuen Koordinaten
     */
    public Position2D coordTrafo(float ang, Position2D p0) 
    {
        double s = Math.sin(ang);
        double c = Math.cos(ang);
        
        return new Position2D((int) Math.round(x * c - y * s) + p0.x,
                              (int) Math.round(x * s + y * c) + p0.y, 
                              phi - ang);
    }

    /** Method coordTrafo
     * @param float ang  - Drehwinkel
     * @param int ptransX - translatorische Verschiebung x
     * @param int ptransY - translatorische Verschiebung y
     * @return Position2D   Punkt in neuen Koordinaten
     */
    public Position2D coordTrafo(float ang, int ptransX, int ptransY) 
    {
        double s = Math.sin(ang);
        double c = Math.cos(ang);
        
        return new Position2D((int) Math.round(x * c - y * s) + ptransX,
                              (int) Math.round(x * s + y * c) + ptransY);
    }

    // das hab ich so gemacht, weil ich es nicht besser wusste. wer mehr 
    // ahnung vom klonen hat, moege es gegebenenfalls korrigieren.   
    // arne, 5.2.2004
    public Object clone()
    {
        return ((Object) new Position2D(x, y, phi));
    }
}
