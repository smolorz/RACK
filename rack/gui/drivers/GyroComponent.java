/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Oliver Wulf  <oliver.wulf@gmx.de>
 *
 */
package rack.gui.drivers;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import javax.swing.JComponent;

import rack.drivers.GyroDataMsg;

public class GyroComponent extends JComponent
{
    protected double roll  = 0.0;
    protected double pitch = 0.0;

    private static final long serialVersionUID = 1L;

    public GyroComponent()
    {
        setDoubleBuffered(true);
    }

    public void updateData(GyroDataMsg data)
    {
        roll  = data.roll;
        pitch = data.pitch;

        if (roll > Math.PI)
            roll -= 2.0 * Math.PI;

        if (pitch > Math.PI)
            pitch -= 2.0 * Math.PI;

        this.repaint();
    }

    public void paint(Graphics g)
    {
        draw(g, 0, 0, this.getWidth(), this.getHeight());
    }

    public void draw(Graphics g, int x, int y, int width, int height)
    {
        int[] pX = {0,0,0,0};
        int[] pY = {0,0,0,0};

        int pitchHeight = (int)((double)pitch * 4.0 / Math.PI *
                                (double)height / 2.0) + height / 2;
        int rollHeight;

        if (roll < - Math.PI * 0.45)
        {
            rollHeight = (int)((double)width / 2.0 *
                               Math.tan(-Math.PI * 0.45));
        }
        else if (roll > Math.PI * 0.45)
        {
            rollHeight = (int)((double)width / 2.0 *
                               Math.tan(Math.PI * 0.45));
        }
        else
        {
            rollHeight = (int)((double)width / 2.0 * Math.tan(roll));
        }

        pX[0] = x;
        pX[1] = x;
        pX[2] = x + width;
        pX[3] = x + width;

        pY[0] = y + height;
        pY[1] = y + pitchHeight + rollHeight;
        pY[2] = y + pitchHeight - rollHeight;
        pY[3] = y + height;

        g.setColor(new Color(0.0f, 0.7f, 0.0f));
        g.fillPolygon(pX, pY, 4);

        pX[0] = x;
        pX[1] = x;
        pX[2] = x + width;
        pX[3] = x + width;

        pY[0] = y;
        pY[1] = y + pitchHeight + rollHeight;
        pY[2] = y + pitchHeight - rollHeight;
        pY[3] = y;

        g.setColor(new Color(0.7f, 0.7f, 1.0f));
        g.fillPolygon(pX, pY, 4);

        g.setColor(Color.BLACK);
        g.drawArc(x, y, width - 1, height - 1, 0, 360);

        g.setColor(Color.BLACK);
        g.drawLine(x, y + height / 2,
                   x + width, y + height / 2);
        g.drawLine(x + width / 4, y + height / 4,
                   x + width * 3 / 4, y + height / 4);
        g.drawLine(x + width / 4, y + height * 3 / 4,
                   x + width * 3 / 4, y + height * 3 / 4);

        g.drawLine(x + width * 15 / 32, y + height * 1 / 8,
                   x + width * 17 / 32, y + height * 1 / 8);
        g.drawLine(x + width * 15 / 32, y + height * 3 / 8,
                   x + width * 17 / 32, y + height * 3 / 8);
        g.drawLine(x + width * 15 / 32, y + height * 5 / 8,
                   x + width * 17 / 32, y + height * 5 / 8);
        g.drawLine(x + width * 15 / 32, y + height * 7 / 8,
                   x + width * 17 / 32, y + height * 7 / 8);
    }

    public Dimension getPreferredSize()
    {
        return new Dimension(100,100);
    }
}
