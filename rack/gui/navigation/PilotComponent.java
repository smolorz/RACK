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
 *      Christian Brennecke <christian.brennecke@volkswagen.de>
 *      Joerg Langenberg    <joerg.langenberg@gmx.net>
 *
 */
package rack.gui.navigation;

import java.awt.*;

import javax.swing.*;

import rack.main.defines.Point2d;
import rack.main.defines.PolarSpline;
import rack.navigation.PilotDataMsg;

public class PilotComponent extends JComponent
{
    protected PilotDataMsg controlInfo = null;
    protected int maxDistance;
    protected boolean updateIsNeeded = false;

    public double xMapCenter = 0.0;
    public double yMapCenter = 0.0;
    public double mmToPixel = 1;
    public int xWindowCenter = 0;
    public int yWindowCenter = 0;
    public int RectangleX = 0;
    public int RectangleY = 0;
    public int RectangleWidth = 0;
    public int RectangleHeight = 0;
    public int chassisLength = 500;
    public int chassisWidth = 500;
    public boolean RectangleDraw = false;

    private static final long serialVersionUID = 1L;

    /** Creates a new instance of ScanComponent */
    public PilotComponent(int maxDistance)
    {
        setBackground(Color.LIGHT_GRAY);
        this.maxDistance = maxDistance;
        setDoubleBuffered(true);
    }

    public void updateData(PilotDataMsg newPilotInfo)
    {
        this.controlInfo = newPilotInfo;
        this.repaint();
    }

    public void setMaxDistance(double factor)
    {
        this.maxDistance = (int) ((double) this.maxDistance * factor);

        if (factor >= 1.0)
        {
            updateIsNeeded = true;
        }
        this.repaint();
    }

    public void setCenter(Point focus)
    {
        xMapCenter = transformToXMap(focus.x, focus.y);
        yMapCenter = transformToYMap(focus.x, focus.y);
        updateIsNeeded = true;
        this.repaint();
    }

    public void drawRec(Point topPoint, Point underPoint)
    {
        int diffX = underPoint.x - topPoint.x;
        int diffY = underPoint.y - topPoint.y;

        if (diffX >= 0)
        {
            if (diffY >= 0)
            {
                this.RectangleX = topPoint.x;
                this.RectangleY = topPoint.y;
                this.RectangleWidth = underPoint.x - topPoint.x;
                this.RectangleHeight = underPoint.y - topPoint.y;
            }
            else
            {
                this.RectangleX = topPoint.x;
                this.RectangleY = underPoint.y;
                this.RectangleWidth = underPoint.x - topPoint.x;
                this.RectangleHeight = topPoint.y - underPoint.y;
            }
        }
        else
        {
            if (diffY >= 0)
            {
                this.RectangleX = underPoint.x;
                this.RectangleY = topPoint.y;
                this.RectangleWidth = topPoint.x - underPoint.x;
                this.RectangleHeight = underPoint.y - topPoint.y;
            }
            else
            {
                this.RectangleX = underPoint.x;
                this.RectangleY = underPoint.y;
                this.RectangleWidth = topPoint.x - underPoint.x;
                this.RectangleHeight = topPoint.y - underPoint.y;
            }
        }
        this.RectangleDraw = true;
        this.repaint();
    }

    public void select(Point topPoint, Point bottomPoint)
    {
        int width = this.getSize().width;
        int height = this.getSize().height;
        int widthReduction = width / this.RectangleWidth;
        int heightReduction = height / this.RectangleHeight;

        if ((topPoint.x != bottomPoint.x) || (topPoint.y != bottomPoint.y))
        {
            xMapCenter = transformToXMap(this.RectangleX +
                                         this.RectangleWidth / 2,
                                         this.RectangleY +
                                         this.RectangleHeight / 2);
            yMapCenter = transformToYMap(this.RectangleX +
                                         this.RectangleWidth / 2,
                                         this.RectangleY +
                                         this.RectangleHeight / 2);

            if (widthReduction < heightReduction)
            {
                this.maxDistance = this.maxDistance / widthReduction;
            }
            else
            {
                this.maxDistance = this.maxDistance / heightReduction;
            }

            this.RectangleDraw = false;
            this.repaint();
        }
    }

    public void right()
    {
        yMapCenter = yMapCenter - 10 / mmToPixel;
        updateIsNeeded = true;
        this.repaint();
    }

    public void down()
    {
        xMapCenter = xMapCenter - 10 / mmToPixel;
        updateIsNeeded = true;
        this.repaint();
    }

    public void left()
    {
        yMapCenter = yMapCenter + 10 / mmToPixel;
        updateIsNeeded = true;
        this.repaint();
    }

    public void up()
    {
        xMapCenter = xMapCenter + 10 / mmToPixel;
        updateIsNeeded = true;
        this.repaint();
    }

    public Dimension getPreferredSize()
    {
        return (new Dimension(100, 100));
    }

    public int transformToXWindow(double xMap, double yMap)
    {
        return ((int) ((yMapCenter - yMap) * mmToPixel) + xWindowCenter);
    }

    public int transformToYWindow(double xMap, double yMap)
    {
        return ((int) ((xMapCenter - xMap) * mmToPixel) + yWindowCenter);
    }

    public double transformToXMap(int xWindow, int yWindow)
    {
        return (xMapCenter - ((double) (yWindow - yWindowCenter)) / mmToPixel);
    }

    public double transformToYMap(int xWindow, int yWindow)
    {
        return (yMapCenter - ((double) (xWindow - xWindowCenter)) / mmToPixel);
    }

    public boolean updateNeeded()
    {
        if (updateIsNeeded == true)
        {
            updateIsNeeded = false;
            return (true);
        }
        else
        {
            return (false);
        }
    }

    /** Normiert den Winkel auf Werte zwischen 0 und 2Pi */
    public static double normAngle(double x)
    {
        while (x < 0)
            x = x + 2 * Math.PI;
        while (x > 2 * Math.PI)
            x = x - 2 * Math.PI;
        return x;
    }

    public static void drawLine(Graphics g, int lineWidth, int startX,
            int startY, int endX, int endY)
    {
        int offset = (lineWidth / 2);
        for (int i = 0; i < lineWidth; i++)
        {
            for (int j = 0; j < lineWidth; j++)
            {
                g.drawLine(startX + (i - offset),
                           startY + (j - offset),
                           endX + (i - offset),
                           endY + (j - offset));
            }
        }
    }

    public static void drawArc(Graphics g, int lineWidth, int startX,
            int startY, int sizeX, int sizeY, int angleStart, int angleOpen)
    {
        int offset = (lineWidth / 2);
        for (int i = 0; i < lineWidth; i++)
        {
            for (int j = 0; j < lineWidth; j++)
            {
                g.drawArc(startX + (i - offset), startY + (j - offset), sizeX,
                          sizeY, angleStart, angleOpen);
            }
        }
    }

    public double normaliseAngle(double angle)
    {
        if (angle < 0.0)
        {
            return (normaliseAngle(angle + 2.0 * Math.PI));
        }
        else if (angle >= 2.0 * Math.PI)
        {
            return (normaliseAngle(angle - 2.0 * Math.PI));
        }
        else
        {
            return (angle);
        }
    }

    public double arcusTangens(double Y, double X)
    {
        double WinkelXY;

        if (X != 0)
        {
            WinkelXY = Math.atan(Y / X);
        }
        else if (Y >= 0.0)
        {
            WinkelXY = Math.PI * 0.5;
        }
        else
        {
            WinkelXY = -Math.PI * 0.5;
        }

        if (X < 0)
        {
            WinkelXY += Math.PI;
        }

        return (normaliseAngle(WinkelXY));
    }

    public void paint(Graphics g)
    {
        double angleOffset;

        if (controlInfo != null)
        {
            int width = this.getSize().width;
            int height = this.getSize().height;
            xWindowCenter = width / 2;
            yWindowCenter = height / 2;
            mmToPixel = ((double) this.getSize().width /
                         (double) (2 * maxDistance));
            int mToPixel = (int) (1000 * mmToPixel);
            int xWindow;
            int yWindow;
            double openAngle;

            // zeichnen des Rechteckes beim zoomen
            if (this.RectangleDraw == true)
            {
                g.setColor(Color.BLACK);
                g.drawRect(this.RectangleX, this.RectangleY,
                        this.RectangleWidth, this.RectangleHeight);
            }

            // zeichnen der Skalierung
            // don't draw narrow grid when mToPix < 20,
            // that means a meter = 20 Pix
            if (mToPixel >= 20)
            {
                for (xWindow = 0;
                     xWindow < Math.max(transformToXWindow(0.0, 0.0), width);
                     xWindow++)
                {
                    if ((xWindow % 5) == 0)
                    {
                        g.setColor(Color.BLACK);
                    }
                    else
                    {
                        g.setColor(Color.GRAY);
                    }

                    g.drawLine(transformToXWindow(0.0, 0.0) +
                               xWindow * mToPixel, 0,
                               transformToXWindow(0.0, 0.0) +
                               xWindow * mToPixel, height);
                    g.drawLine(transformToXWindow(0.0, 0.0) -
                               xWindow * mToPixel, 0,
                               transformToXWindow(0.0, 0.0) -
                               xWindow * mToPixel, height);
                }

                for (yWindow = 0;
                     yWindow < Math.max(transformToYWindow(0.0, 0.0), height);
                     yWindow++)
                {
                    if ((yWindow % 5) == 0)
                    {
                        g.setColor(Color.BLACK);
                    }
                    else
                    {
                        g.setColor(Color.GRAY);
                    }
                    g.drawLine(0,
                               transformToYWindow(0, 0) + yWindow * mToPixel,
                               width,
                               transformToYWindow(0, 0) + yWindow * mToPixel);
                    g.drawLine(0,
                               transformToYWindow(0, 0) - yWindow * mToPixel,
                               width,
                               transformToYWindow(0, 0) - yWindow * mToPixel);
                }
            }

            // zeichnen des Roboters in die Karte
            if ((chassisLength != 0) &&
                (chassisWidth != 0))
            {
                Point2d upperLeftCorner = new Point2d();
                Point2d lowerRightCorner = new Point2d();
                Point2d upperRightCorner = new Point2d();
                Point2d lowerLeftCorner = new Point2d();

                double PolarLaenge = Math.sqrt(Math.pow(
                                        (float)chassisLength * 0.5, 2.0) +
                                        Math.pow((float)chassisWidth * 0.5,
                                        2.0));
                double PolarWinkel = arcusTangens((double)chassisWidth,
                                                (double)chassisLength);

                upperLeftCorner.x = transformToXWindow(
                                        controlInfo.pos.x +
                                        (PolarLaenge * Math.cos(
                                        (double)controlInfo.pos.rho -
                                        PolarWinkel)),
                                        -(controlInfo.pos.y +
                                        (PolarLaenge * Math.sin(
                                        (double)controlInfo.pos.rho -
                                        PolarWinkel))));

                upperLeftCorner.y = transformToYWindow(
                                        controlInfo.pos.x +
                                        (PolarLaenge * Math.cos(
                                        (double)controlInfo.pos.rho -
                                        PolarWinkel)),
                                        -(controlInfo.pos.y +
                                        (PolarLaenge * Math.sin(
                                        (double)controlInfo.pos.rho -
                                        PolarWinkel))));

                upperRightCorner.x = transformToXWindow(
                                        controlInfo.pos.x +
                                        (PolarLaenge * Math.cos(
                                        (double)controlInfo.pos.rho +
                                        PolarWinkel)),
                                        -(controlInfo.pos.y +
                                        (PolarLaenge * Math.sin(
                                        (double)controlInfo.pos.rho +
                                        PolarWinkel))));

                upperRightCorner.y = transformToYWindow(
                                        controlInfo.pos.x +
                                        (PolarLaenge * Math.cos(
                                        (double)controlInfo.pos.rho +
                                        PolarWinkel)),
                                        -(controlInfo.pos.y +
                                        (PolarLaenge * Math.sin(
                                        (double)controlInfo.pos.rho +
                                        PolarWinkel))));

                lowerRightCorner.x = transformToXWindow(
                                        controlInfo.pos.x +
                                        (PolarLaenge * Math.cos(
                                        (double)controlInfo.pos.rho -
                                        PolarWinkel + Math.PI)),
                                        -(controlInfo.pos.y +
                                        (PolarLaenge * Math.sin(
                                        (double)controlInfo.pos.rho -
                                        PolarWinkel + Math.PI))));

                lowerRightCorner.y = transformToYWindow(
                                        controlInfo.pos.x +
                                        (PolarLaenge * Math.cos(
                                        (double)controlInfo.pos.rho -
                                        PolarWinkel + Math.PI)),
                                        -(controlInfo.pos.y +
                                        (PolarLaenge * Math.sin(
                                        (double)controlInfo.pos.rho -
                                        PolarWinkel + Math.PI))));

                lowerLeftCorner.x = transformToXWindow(
                                        controlInfo.pos.x +
                                        (PolarLaenge * Math.cos(
                                        (double)controlInfo.pos.rho +
                                        PolarWinkel + Math.PI)),
                                        -(controlInfo.pos.y +
                                        (PolarLaenge * Math.sin(
                                        (double)controlInfo.pos.rho +
                                        PolarWinkel + Math.PI))));

                lowerLeftCorner.y = transformToYWindow(
                                        controlInfo.pos.x +
                                        (PolarLaenge * Math.cos(
                                        (double) controlInfo.pos.rho +
                                        PolarWinkel + Math.PI)),
                                        -(controlInfo.pos.y +
                                        (PolarLaenge * Math.sin(
                                        (double) controlInfo.pos.rho +
                                        PolarWinkel + Math.PI))));

                g.setColor(Color.DARK_GRAY);
                drawLine(g, 1, (int)upperLeftCorner.x,
                        (int)upperLeftCorner.y, (int)upperRightCorner.x,
                        (int)upperRightCorner.y);
                drawLine(g, 1, (int)upperRightCorner.x,
                        (int)upperRightCorner.y, (int)lowerRightCorner.x,
                        (int)lowerRightCorner.y);
                drawLine(g, 1, (int)lowerRightCorner.x,
                        (int)lowerRightCorner.y, (int)lowerLeftCorner.x,
                        (int)lowerLeftCorner.y);
                drawLine(g, 1, (int)lowerLeftCorner.x,
                        (int)lowerLeftCorner.y, (int)upperLeftCorner.x,
                        (int)upperLeftCorner.y);
                drawLine(g, 1,
                        (int)((lowerLeftCorner.x + upperLeftCorner.x) / 2.0),
                        (int)((lowerLeftCorner.y + upperLeftCorner.y) / 2.0),
                        (int)((upperLeftCorner.x + upperRightCorner.x) / 2.0),
                        (int)((upperLeftCorner.y + upperRightCorner.y) / 2.0));
                drawLine(g, 1,
                        (int)((lowerRightCorner.x + upperRightCorner.x) / 2.0),
                        (int)((lowerRightCorner.y + upperRightCorner.y) / 2.0),
                        (int)((upperLeftCorner.x + upperRightCorner.x) / 2.0),
                        (int)((upperLeftCorner.y + upperRightCorner.y) / 2.0));
            }

            PolarSpline route = new PolarSpline();

            // zeichnen der Solltrajektorie, wenn vorhanden, in blau
            for (int i = 0; i < controlInfo.splineNum; i++)
            {
                // direct line
                if (controlInfo.spline[i].radius == 0)
                {
                    // zunaechst alle Elemente der Spline von mm in
                    // Bildschirmkoordinaten transformieren
                    route.startPos.x = transformToXWindow(
                            controlInfo.spline[i].startPos.x,
                            -controlInfo.spline[i].startPos.y);
                    route.startPos.y = transformToYWindow(
                            controlInfo.spline[i].startPos.x,
                            -controlInfo.spline[i].startPos.y);

                    route.endPos.x = transformToXWindow(
                            controlInfo.spline[i].endPos.x,
                            -controlInfo.spline[i].endPos.y);
                    route.endPos.y = transformToYWindow(
                            controlInfo.spline[i].endPos.x,
                            -controlInfo.spline[i].endPos.y);

                    g.setColor(Color.BLUE);
                    drawLine(g, 3, (int) route.startPos.x,
                            (int) route.startPos.y, (int) route.endPos.x,
                            (int) route.endPos.y);
                }

                // curved spline
                else
                {
                    route.centerPos.x = transformToXWindow(
                                        controlInfo.spline[i].centerPos.x,
                                        -controlInfo.spline[i].centerPos.y);
                    route.centerPos.y = transformToYWindow(
                                        controlInfo.spline[i].centerPos.x,
                                        -controlInfo.spline[i].centerPos.y);

                    route.radius = (int) Math.round(Math.abs(
                                        controlInfo.spline[i].radius) *
                                        mmToPixel);

                    openAngle = Math.abs((double)controlInfo.spline[i].length /
                                        (double) controlInfo.spline[i].radius);

                    if (controlInfo.spline[i].length < 0)
                    {
                        if (controlInfo.spline[i].radius > 0)
                        {
                            route.startPos.rho = (float)Math.toDegrees(
                                                 normAngle(Math.PI -
                                                 controlInfo.spline[i].startPos.rho));
                        }
                        else
                        {
                            route.startPos.rho = (float)Math.toDegrees(
                                                 normAngle(
                                                 -controlInfo.spline[i].endPos.rho));
                        }
                    }
                    else
                    {
                        if (controlInfo.spline[i].radius > 0)
                        {
                            route.startPos.rho = (float) Math.toDegrees(
                                                 normAngle(Math.PI -
                                                 controlInfo.spline[i].startPos.rho -
                                                 openAngle));
                        }
                        else
                        {
                            route.startPos.rho = (float)Math.toDegrees(
                                                 normAngle(
                                                 -controlInfo.spline[i].endPos.rho -
                                                 openAngle));
                        }
                    }

                    g.setColor(Color.BLUE);
                    int r = (int) route.radius;
                    drawArc(g, 3, (int) route.centerPos.x - r,
                            (int) route.centerPos.y - r, r * 2, r * 2,
                            (int) Math.round(route.startPos.rho),
                            (int) Math.round((float)Math.toDegrees(normAngle(openAngle))));
                }
            } //


            // zeichen der Sollwerte an den Roboter in gruen
            float radius = 1.0f / controlInfo.curve;
            if ((radius > 100000) | (radius < -100000))
            {
                radius = 0;
            }

            // curved spline
            if (radius != 0)
            {
                openAngle = Math.abs(controlInfo.speed * controlInfo.curve);

                if (openAngle > Math.PI)
                    openAngle = Math.PI;

                if (controlInfo.curve > 0)
                    angleOffset = Math.PI * 0.5;
                else
                    angleOffset = Math.PI * 1.5;

                route.centerPos.x = transformToXWindow(
                                        controlInfo.pos.x +
                                        Math.cos(angleOffset +
                                        (double)controlInfo.pos.rho) /
                                        Math.abs(controlInfo.curve),
                                        -(controlInfo.pos.y +
                                        Math.sin(angleOffset +
                                        (double)controlInfo.pos.rho) /
                                        Math.abs(controlInfo.curve)));
                route.centerPos.y = transformToYWindow(
                                        controlInfo.pos.x +
                                        Math.cos(angleOffset +
                                        (double)controlInfo.pos.rho) /
                                        Math.abs(controlInfo.curve),
                                        -(controlInfo.pos.y +
                                        Math.sin(angleOffset +
                                        (double)controlInfo.pos.rho) /
                                        Math.abs(controlInfo.curve)));

                route.radius = (int)Math.round(Math.abs(mmToPixel /
                                                    controlInfo.curve));

                // backward movement
                if (controlInfo.speed < 0)
                {
                    if (controlInfo.curve > 0)
                        route.startPos.rho = (float)Math.toDegrees(
                                               normAngle(Math.PI -
                                               controlInfo.pos.rho));
                    else
                        route.startPos.rho = (float)Math.toDegrees(
                                               normAngle(-controlInfo.pos.rho -
                                               openAngle));
                }
                // forward movement
                else
                {
                    if (controlInfo.curve > 0)
                        route.startPos.rho = (float)Math.toDegrees(
                                               normAngle(Math.PI -
                                               controlInfo.pos.rho - openAngle));
                    else
                    {
                        route.startPos.rho = (float)Math.toDegrees(
                                                normAngle(-controlInfo.pos.rho));
                    }
                }

                g.setColor(Color.GREEN);
                int r = (int) route.radius;
                drawArc(g, 3, (int) route.centerPos.x - r,
                       (int)route.centerPos.y - r, r * 2, r * 2,
                       (int)Math.round(route.startPos.rho),
                       (int)Math.toDegrees(normAngle(openAngle)));
            }
            // direct line
            else
            {
                int length = controlInfo.speed;

                route.startPos.x = transformToXWindow(
                                     controlInfo.pos.x,
                                     -controlInfo.pos.y);
                route.startPos.y = transformToYWindow(
                                     controlInfo.pos.x,
                                     -controlInfo.pos.y);
                route.endPos.x = transformToXWindow(
                                     controlInfo.pos.x +
                                     length * Math.cos(
                                     (double)controlInfo.pos.rho),
                                     -(controlInfo.pos.y +
                                     length * Math.sin(
                                     (double)controlInfo.pos.rho)));
                route.endPos.y = transformToYWindow(
                                     controlInfo.pos.x +
                                     length* Math.cos(
                                     (double)controlInfo.pos.rho),
                                     -(controlInfo.pos.y +
                                     length * Math.sin(
                                     (double)controlInfo.pos.rho)));

                g.setColor(Color.GREEN);
                drawLine(g, 5, route.startPos.x, route.startPos.y,
                         route.endPos.x, route.endPos.y);
            }

            // draw scan position
            g.setColor(Color.BLACK);
            xWindow = transformToXWindow(0.0, 0.0);
            yWindow = transformToYWindow(0.0, 0.0);
            g.fillRect(xWindow - 2, yWindow - 2, 4, 4);

            g.setColor(Color.GRAY);
            g.fillRect(xWindowCenter - 2, yWindowCenter - 2, 4, 4);
        }
    }
}
