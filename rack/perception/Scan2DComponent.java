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
package rack.perception;

import java.awt.*;

import javax.swing.*;

import rack.main.defines.ScanPoint;

public class Scan2DComponent extends JComponent
{
  protected Scan2DDataMsg scan2DData = null;
  protected int maxDistance;

  public double xScanCenter = 0.0;
  public double yScanCenter = 0.0;
  public double mmToPixel = 1;
  public int xWindowCenter = 0;
  public int yWindowCenter = 0;

  /** Creates a new instance of LadarScan2DComponent */
  public Scan2DComponent(int maxDistance)
  {
    setBackground(Color.lightGray );
    this.maxDistance = maxDistance;
    setDoubleBuffered(true);
  }

  public void updateData(Scan2DDataMsg ladarScan2DData)
  {
    this.scan2DData = ladarScan2DData;
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

  public void drawRec(Point topPoint,Point underPoint)
  {
  }

  public void select(Point topPoint,Point buttonPoint)
  {
  }

  public void right()
  {
    yScanCenter = yScanCenter+10/mmToPixel;
    this.repaint();
  }

  public void down()
  {
    xScanCenter = xScanCenter+10/mmToPixel;
    this.repaint();
  }
  public void left()
  {
    yScanCenter = yScanCenter-10/mmToPixel;
    this.repaint();
  }

  public void up()
  {
    xScanCenter = xScanCenter-10/mmToPixel;
    this.repaint();
  }

  public Dimension getPreferredSize()
  {
    return(new Dimension(400, 400));
  }

  public int transformToXWindow(double xScan, double yScan)
  {
    return((int)((yScanCenter + yScan)*mmToPixel) + xWindowCenter);
  }

  public int transformToYWindow(double xScan, double yScan)
  {
    return((int)((xScanCenter - xScan)*mmToPixel) + yWindowCenter);
  }

  public double transformToXScan(int xWindow, int yWindow)
  {
    return(xScanCenter - ((double)(yWindow - yWindowCenter))/mmToPixel);
  }

  public double transformToYScan(int xWindow, int yWindow)
  {
    return(yScanCenter - ((double)(xWindow - xWindowCenter))/mmToPixel);
  }

  public void paint(Graphics g)
  {
    if (scan2DData != null) {
      int width     = this.getSize().width;
      int height    = this.getSize().height;
      xWindowCenter = width/2;
      yWindowCenter = height/2;
      mmToPixel     = ((double)this.getSize().width / (double)(2 * maxDistance));
      int mToPixel  = (int)(1000*mmToPixel);
      int xWindow;
      int yWindow;

      if (mToPixel >= 20) { // don't draw narrow grid when mToPix<20,that means a meter = 20 Pix
        for(xWindow = 0;
            xWindow< Math.max(transformToXWindow(0.0, 0.0),width) ;
            xWindow++) {

          if ((xWindow % 5) == 0) {
            g.setColor(Color.black);
          } else {
            g.setColor(Color.gray);
          }

          g.drawLine(transformToXWindow(0.0, 0.0) + xWindow*mToPixel, 0 ,
                     transformToXWindow(0.0,0.0) + xWindow*mToPixel,height);
          g.drawLine(transformToXWindow(0.0, 0.0) - xWindow*mToPixel, 0 ,
                     transformToXWindow(0.0,0.0) - xWindow*mToPixel,height);
        }

        for (yWindow=0;
             yWindow<Math.max(transformToYWindow(0.0,0.0),height);
             yWindow++) {

          if ((yWindow % 5) == 0) {
            g.setColor(Color.black);
          } else {
            g.setColor(Color.gray);
          }

          g.drawLine(0, transformToYWindow(0,0) + yWindow*mToPixel, width,
                        transformToYWindow(0,0) + yWindow*mToPixel);
          g.drawLine(0, transformToYWindow(0,0) - yWindow*mToPixel, width,
                        transformToYWindow(0,0) - yWindow*mToPixel);
        }
      }

      // draw scan position
      g.setColor(Color.gray);
      xWindow = transformToXWindow(0.0, 0.0);
      yWindow = transformToYWindow(0.0, 0.0);
//    g.fillRect(xWindow-2,yWindow-2,4,4);
      g.fillRect(xWindow-(int)(300.0 * mmToPixel), yWindow-(int)(420.0 * mmToPixel),
                (int)(600.0 * mmToPixel),(int)(840.0 * mmToPixel));

      ScanPoint point;
      for(int i = 0; i < scan2DData.pointNum; i++) {
        point = scan2DData.point[i];

        if((point.type & ScanPoint.TYPE_INVALID) != 0)
        {               
            g.setColor(Color.gray);
        }
        else if((point.type & ScanPoint.TYPE_REFLECTOR) != 0)
        {
            g.setColor(Color.yellow);
        }
        else if((point.type & ScanPoint.TYPE_MASK) == ScanPoint.TYPE_LANDMARK)
        {
            g.setColor(Color.blue);
        }
        else if((point.type & ScanPoint.TYPE_MASK) == ScanPoint.TYPE_OBSTACLE)
        {
            g.setColor(Color.red);
        }
        else
        {
            if(point.segment > 0)
            {
                switch((point.segment - 1) % 4)
                {
                    case 0:
                        g.setColor(Color.cyan);
                        break;
                    case 1:
                        g.setColor(Color.green);
                        break;
                    case 2:
                        g.setColor(Color.magenta);
                        break;
                    case 3:
                        g.setColor(Color.orange);
                        break;
                }
            }
            else
            {
                g.setColor(Color.black);
            }
        }

        xWindow = transformToXWindow(point.x, point.y);
        yWindow = transformToYWindow(point.x, point.y);

        if (mToPixel >= 40) { // draw big points when mToPix>40,that means a meter = 40 Pix
          g.fillRect(xWindow-2,yWindow-2,5,5);
        } else {
          g.fillRect(xWindow-1,yWindow-1,3,3);
        }
      }

      g.setColor(Color.black);
      g.fillRect(xWindowCenter-2,yWindowCenter-2,4,4);
    }
  }
}
