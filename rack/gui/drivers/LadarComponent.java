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
package rack.gui.drivers;

import java.awt.*;
import javax.swing.*;

import rack.drivers.LadarDataMsg;

public class LadarComponent extends JComponent
{
    protected LadarDataMsg ladarData = null;
    protected int maxDistance;

    public double xScanCenter = 0.0;
    public double yScanCenter = 0.0;
    public double mmToPixel = 1;
    public int xWindowCenter = 0;
    public int yWindowCenter = 0;

    private static final long serialVersionUID = 1L;

    /** Creates a new instance of LadarComponent */
    public LadarComponent(int maxDistance)
    {
        setBackground(Color.lightGray);
        this.maxDistance = maxDistance;
        setDoubleBuffered(true);
    }

    public void updateData(LadarDataMsg ladarData)
    {
        this.ladarData = ladarData;
        this.repaint();
    }

    public void setMaxDistance(int distance)
    {
        this.maxDistance = distance;
        this.repaint();

    }

    public void setCenter(Point focus)
    {
        xScanCenter = transformToXScan(focus.x, focus.y);
        yScanCenter = transformToYScan(focus.x, focus.y);
        this.repaint();
    }

    public void drawRec(Point topPoint, Point underPoint)
    {
        // this.topPoint = topPoint;
        // this.underPoint =underPoint;
        // this.drawRect = true;
        // this.paint(Graphics g);

    }

    public void select(Point topPoint, Point buttonPoint)
    {
    }

    public void right()
    {
        yScanCenter = yScanCenter + 10 / mmToPixel;
        this.repaint();
    }

    public void down()
    {
        xScanCenter = xScanCenter + 10 / mmToPixel;
        this.repaint();
    }

    public void left()
    {
        yScanCenter = yScanCenter - 10 / mmToPixel;
        this.repaint();
    }

    public void up()
    {
        xScanCenter = xScanCenter - 10 / mmToPixel;
        this.repaint();
    }

    public Dimension getPreferredSize()
    {
        return (new Dimension(400, 400));
    }

    public int transformToXWindow(double xScan, double yScan)
    {
        return ((int) ((yScanCenter - yScan) * mmToPixel) + xWindowCenter);
    }

    public int transformToYWindow(double xScan, double yScan)
    {
        return ((int) ((xScanCenter - xScan) * mmToPixel) + yWindowCenter);
    }

    public double transformToXScan(int xWindow, int yWindow)
    {
        return (xScanCenter - ((double) (yWindow - yWindowCenter)) / mmToPixel);
    }

    public double transformToYScan(int xWindow, int yWindow)
    {
        return (yScanCenter - ((double) (xWindow - xWindowCenter)) / mmToPixel);
    }

    public void paint(Graphics g)
    {
        if (ladarData != null)
        {

            int width = this.getSize().width;
            int height = this.getSize().height;
            xWindowCenter = width / 2;
            yWindowCenter = height / 2;
            mmToPixel = ((double) this.getSize().width / (double) (2 * maxDistance));
            int mToPixel = (int) (1000 * mmToPixel);
            double xScan;
            double yScan;
            int xWindow;
            int yWindow;

            if (mToPixel >= 20)
            { // don't draw narrow grid when mToPix<20,that means a meter = 20
                // Pix
                for (xWindow = 0; xWindow < Math.max(transformToXWindow(0.0,
                        0.0), width); xWindow++)
                {

                    if ((xWindow % 5) == 0)
                    {
                        g.setColor(Color.black);
                    }
                    else
                    {
                        g.setColor(Color.gray);
                    }

                    g.drawLine(transformToXWindow(0.0, 0.0) + xWindow
                            * mToPixel, 0, transformToXWindow(0.0, 0.0)
                            + xWindow * mToPixel, height);
                    g.drawLine(transformToXWindow(0.0, 0.0) - xWindow
                            * mToPixel, 0, transformToXWindow(0.0, 0.0)
                            - xWindow * mToPixel, height);
                }

                for (yWindow = 0; yWindow < Math.max(transformToYWindow(0.0,
                        0.0), height); yWindow++)
                {

                    if ((yWindow % 5) == 0)
                    {
                        g.setColor(Color.black);
                    }
                    else
                    {
                        g.setColor(Color.gray);
                    }

                    g.drawLine(0,
                            transformToYWindow(0, 0) + yWindow * mToPixel,
                            width, transformToYWindow(0, 0) + yWindow
                                    * mToPixel);
                    g.drawLine(0,
                            transformToYWindow(0, 0) - yWindow * mToPixel,
                            width, transformToYWindow(0, 0) - yWindow
                                    * mToPixel);
                }
            }

            // g.setColor(Color.red);
            float angle = ladarData.startAngle;
            for (int i = 0; i < ladarData.distanceNum; i++)
            {
                if (ladarData.distance[i] >= 0)
                {
                    xScan = (double) ladarData.distance[i] * Math.cos(angle);
                    yScan = -(double) ladarData.distance[i] * Math.sin(angle);
                    g.setColor(Color.red);
                }
                else
                {
                    xScan = -(double) ladarData.distance[i] * Math.cos(angle);
                    yScan = (double) ladarData.distance[i] * Math.sin(angle);
                    g.setColor(Color.blue);
                }

                // xScan = (double)ladarData.distance[i]*Math.cos(angle);
                // yScan = -(double)ladarData.distance[i]*Math.sin(angle);

                xWindow = transformToXWindow(xScan, yScan);
                yWindow = transformToYWindow(xScan, yScan);

                if (mToPixel >= 40)
                { // draw big points when mToPix>40,that means a meter = 40
                    // Pix
                    g.fillRect(xWindow - 2, yWindow - 2, 5, 5);
                }
                else
                {
                    g.fillRect(xWindow - 1, yWindow - 1, 3, 3);
                }
                angle += ladarData.angleResolution;
            }

            // draw scan position
            g.setColor(Color.black);
            xWindow = transformToXWindow(0.0, 0.0);
            yWindow = transformToYWindow(0.0, 0.0);
            g.fillRect(xWindow - 1, yWindow - 1, 3, 3);

            g.setColor(Color.gray);
            g.fillRect(xWindowCenter - 2, yWindowCenter - 2, 4, 4);

        }
        else
        {
            // System.out.println("LadarComponent: no data to paint");
        }
    }
}
