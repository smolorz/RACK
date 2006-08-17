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

public class Point2D implements Serializable
{
    public int x = 0;
    public int y = 0;

    private static final long serialVersionUID = 1L;

    /**
     * Data-length Point2D.
     */
    static public int getDataLen()
    {
        return (8);
    }

    /**
     * Constructor Point2D.
     */
    public Point2D()
    {
    }

    /**
     * Constructor Point2D.
     * @param x
     * @param y
     */
    public Point2D(int x, int y)
    {
        this.x    = x;
        this.y    = y;
    }

    /**
     * read Point2D.
     */
    public void readData(EndianDataInputStream dataIn) throws IOException
    {
        x   = dataIn.readInt();
        y   = dataIn.readInt();
    }

    /**
     * write Point2D.
     */
    public void writeData(DataOutputStream dataOut) throws IOException
    {
        dataOut.writeInt(x);
        dataOut.writeInt(y);
    }

    /** Method getDistance
     * @param p2
     * @return distance to p2
     */
    public int getDistance(Point2D p2)
    {
        return ((int) Math.round(Math.sqrt((x - p2.x) * (x - p2.x) +
                                           (y - p2.y) * (y - p2.y))));
    }

    /** Method coordTrafo
     * @param ang Drehwinkel
     * @return Punkt in neuen Koordinaten
     */
    public Point2D coordTrafo(float ang)
    {
        double s = Math.sin(ang);
        double c = Math.cos(ang);

        return new Point2D((int) Math.round(x * c - y * s),
                           (int) Math.round(x * s + y * c));
    }

    /** Method coordTrafo
     * @param ang Drehwinkel
     * @param p0 translatorische Verschiebung
     * @return Punkt in neuen Koordinaten
     */
    public Point2D coordTrafo(float ang, Point2D p0)
    {
        double s = Math.sin(ang);
        double c = Math.cos(ang);

        return new Point2D((int) Math.round(x * c - y * s) + p0.x,
                           (int) Math.round(x * s + y * c) + p0.y);
    }

    /** Method coordTrafo
     * @param ang Drehwinkel
     * @param ptransX translatorische Verschiebung x
     * @param ptransY translatorische Verschiebung y
     * @return Punkt in neuen Koordinaten
     */
     public Point2D coordTrafo(float ang, int ptransX, int ptransY)
     {
         double s = Math.sin(ang);
         double c = Math.cos(ang);

         return new Point2D((int) Math.round(x * c - y * s) + ptransX,
                            (int) Math.round(x * s + y * c) + ptransY);
    }
}
