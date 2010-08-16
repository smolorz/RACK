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
package rack.gui.perception;

import java.awt.*;
import javax.swing.*;

import rack.main.defines.ScanPoint;
import rack.perception.Scan3dDataMsg;
import rack.perception.Scan3dRangeImageMsg;

public class Scan3dComponent extends JComponent {

  protected Scan3dRangeImageMsg rangeImage = null;
  protected int maxDistance;

  private static final long serialVersionUID = 1L;

  /** Creates a new instance of ScanComponent */
  public Scan3dComponent() {
    setBackground(Color.LIGHT_GRAY );
    this.maxDistance = 20000;
    setDoubleBuffered(true);
  }

  public Scan3dComponent(int maxDistance) {
    setBackground(Color.LIGHT_GRAY );
    this.maxDistance = maxDistance;
  }

  public void updateData(Scan3dRangeImageMsg rangeImage) {
    this.rangeImage = rangeImage;
    this.repaint();
  }

  public void setMaxDistance(int distance) {
    this.maxDistance = distance;
    this.repaint();
  }

  public Dimension getPreferredSize()
  {
    return(new Dimension(600, 300));
  }

  public Color createColorRGB(int value, int minValue, int maxValue)
  {
    float v = (float)(value - minValue) / (float)(maxValue - minValue);

    if (v < 0.0f) {

      return(new Color(1.0f, 0.0f, 0.0f));

    } else if (v < 0.25f) {

      v = v * 4;
      return(new Color(1.0f, v, 0.0f));

    } else if (v < 0.5f) {

      v = (0.5f - v) * 4;
      return(new Color(v, 1.0f, 0.0f));

    } else if (v < 0.75f) {
      v = (v - 0.5f) * 4;
      return(new Color(0.0f, 1.0f, v));

    } else if (v < 1.0f) {

      v = (1.0f - v) * 4;
      return(new Color(0.0f, v, 1.0f));

    } else {
      return(new Color(0.0f, 0.0f, 1.0f));
    }
  }

  public Color createColorGray(int value, int minValue, int maxValue)
  {
    float v = (float)(value - minValue) / (float)(maxValue - minValue);

    if (v < 0.0f) {

      return (new Color(0.9f, 0.9f, 0.9f));

    } else if (v < 1.0f) {

      v = 0.9f - v * 0.9f;
      return (new Color(v, v, v));

    } else {

      return (new Color(0.0f, 0.0f, 0.0f));

    }
  }

  public Color createColorRangeType(int value, int minValue, int maxValue, int type)
  {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    float v = (float)(value - minValue) / (float)(maxValue - minValue);

    int typeMask = type & ScanPoint.TYPE_MASK;

    if ((type & ScanPoint.TYPE_REFLECTOR) != 0) {
      r = 1.0f;  // red
      g = 0.0f;
      b = 0.0f;
    } else if (typeMask == ScanPoint.TYPE_GROUND) {
      r = 0.0f;  // green
      g = 0.8f;
      b = 0.2f;
    } else if (typeMask == ScanPoint.TYPE_CEILING) {
      r = 0.5f;  // blue
      g = 0.5f;
      b = 1.0f;
    } else  {    // TYPE_OBJECT or TYPE_UNKNOWN
      r = 1.0f;
      g = 1.0f;
      b = 0.6f;

      if (((type & ScanPoint.TYPE_HOR_EDGE) != 0) |
          ((type & ScanPoint.TYPE_VER_EDGE) != 0)) {
        v = v + 0.2f;
/*
        r = 0.8f;
        g = 0.8f;
        b = 0.4f;
*/
      }
    }

    if((type & ScanPoint.TYPE_MAX_RANGE) != 0) {
        return(new Color(0.0f, 0.0f, 0.0f));
      }

    if ((type & ScanPoint.TYPE_INVALID) != 0) {
      return(new Color(1.0f, 1.0f, 1.0f));
    }

    if (v < 0.0f) {

      return(new Color(r * 0.9f, g * 0.9f, b * 0.9f));

    } else if(v < 1.0f) {

      v = 0.9f - v * 0.8f;
      return(new Color(r * v, g * v, b * v));

    } else {

      return(new Color(r * 0.1f, g * 0.1f, b * 0.1f));

    }
  }

  public Color createColorRangeEdge(int value, int minValue, int maxValue, int type)
  {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    float v = (float)(value - minValue) / (float)(maxValue - minValue);

    if (((type & ScanPoint.TYPE_HOR_EDGE) != 0) &
        ((type & ScanPoint.TYPE_VER_EDGE) != 0)) {
      r = 0.0f;
      g = 1.0f;
      b = 0.0f;
    } else if((type & ScanPoint.TYPE_VER_EDGE) != 0) {
      r = 1.0f;
      g = 0.0f;
      b = 0.0f;
    } else if((type & ScanPoint.TYPE_HOR_EDGE) != 0) {
      r = 0.0f;
      g = 0.0f;
      b = 1.0f;
    } else {
      r = 0.8f;
      g = 0.8f;
      b = 0.8f;
    }

    if(v < 0.0f) {

      return(new Color(r * 0.9f, g * 0.9f, b * 0.9f));

    } else if(v < 1.0f) {

      v = 0.9f - v * 0.8f;
      return(new Color(r * v, g * v, b * v));

    } else {

      return(new Color(r * 0.1f, g * 0.1f, b * 0.1f));

    }
  }

  public Color createColorRangeSegment(int value, int minValue, int maxValue, int type)
  {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    float v = (float)(value - minValue) / (float)(maxValue - minValue);

    if(type == 0) {
      r = 0.8f;
      g = 0.8f;
      b = 0.8f;
    } else {
      type = (type - 1) % 6;

      switch (type) {
        case 0:
          r = 1.0f;
          g = 0.0f;
          b = 0.0f;
          break;
        case 1:
          r = 0.0f;
          g = 1.0f;
          b = 0.0f;
          break;
        case 2:
          r = 0.0f;
          g = 0.0f;
          b = 1.0f;
          break;
        case 3:
          r = 1.0f;
          g = 0.0f;
          b = 1.0f;
          break;
        case 4:
          r = 1.0f;
          g = 1.0f;
          b = 0.0f;
          break;
        case 5:
          r = 0.0f;
          g = 1.0f;
          b = 1.0f;
          break;
      }
    }

    if(v < 0.0f) {

      return(new Color(r * 0.9f, g * 0.9f, b * 0.9f));

    } else if(v < 1.0f) {

      v = 0.9f - v * 0.8f;
      return(new Color(r * v, g * v, b * v));

    } else {

      return(new Color(r * 0.1f, g * 0.1f, b * 0.1f));

    }
  }

  public void paint(Graphics g)
  {
    if(rangeImage != null) {
      int width=this.getSize().width;
      int height=this.getSize().height;
      float pixelHeight;
      float pixelWidth;

      switch( rangeImage.getScanMode()) {
        case Scan3dDataMsg.PITCH:
          pixelHeight = (float)height / (float)rangeImage.scanNum;
          pixelWidth = (float)width / (float)rangeImage.scanPointNum;

          for(int i = 0; i < rangeImage.scanNum; i++) {
            for(int j = 0; j < rangeImage.scanPointNum; j++) {
              if (rangeImage.point[i][j].range == 0) {
                g.setColor(Color.BLACK);
              } else {
                if(rangeImage.point[i][j].type == 0) {
                  g.setColor(createColorGray(rangeImage.point[i][j].range,
                             maxDistance / 10, maxDistance));
                } else {
                  g.setColor(createColorRGB(rangeImage.point[i][j].range,
                             maxDistance / 10, maxDistance));
                }
              }

              if (rangeImage.isPositiveTurn()) {
                if (rangeImage.isSquareGrid()) {
                  g.fillRect((int)((rangeImage.scanPointNum - j - 1) * pixelWidth),
                             (int)(i * pixelHeight),
                             (int)pixelWidth + 1,
                             (int)pixelHeight + 1);
                } else { // isHexagonalGrid
                  if ((i % 2) == 0) {
                    g.fillRect((int)((rangeImage.scanPointNum - j - 1) * pixelWidth),
                               (int)(i * pixelHeight),
                               (int)pixelWidth + 1,
                               (int)pixelHeight + 1);
                  } else {
                    g.fillRect((int)((rangeImage.scanPointNum - j - 1.5) * pixelWidth),
                               (int)(i * pixelHeight),
                               (int)pixelWidth + 1,
                               (int)pixelHeight + 1);
                  }
                }
              } else { // isNegativeTurn
                if (rangeImage.isSquareGrid()) {
                  g.fillRect((int)((rangeImage.scanPointNum - j - 1) * pixelWidth),
                             (int)((rangeImage.scanNum - i - 1) * pixelHeight),
                             (int)pixelWidth + 1,
                             (int)pixelHeight + 1);
                } else { // isHexagonalGrid
                  if ((i % 2) == 0) {
                    g.fillRect((int)((rangeImage.scanPointNum - j - 1) * pixelWidth),
                               (int)((rangeImage.scanNum - i - 1) * pixelHeight),
                               (int)pixelWidth + 1,
                               (int)pixelHeight + 1);
                  } else {
                    g.fillRect((int)((rangeImage.scanPointNum - j - 1.5) * pixelWidth),
                               (int)((rangeImage.scanNum - i - 1) * pixelHeight),
                               (int)pixelWidth + 1,
                               (int)pixelHeight + 1);
                  }
                }
              }
            }
          }
          break;

        case Scan3dDataMsg.TOP:
          pixelWidth  = (float)width / (float)(rangeImage.scanNum);
          pixelHeight = (float)height / (float)(rangeImage.scanPointNum / 2);

          // paint first segment
          for(int i = 0; i < rangeImage.scanNum; i++) {
            for(int j = 0; j < (rangeImage.scanPointNum); j++) {
              int    scanStart;
              if (rangeImage.isStart180()) {
                scanStart = rangeImage.scanNum;
              } else { // Start0
                scanStart = 0;
              }

              g.setColor(createColorRangeType(rangeImage.point[i][j].range, 0,
                                              maxDistance, rangeImage.point[i][j].type));
//            g.setColor(createColorRangeEdge(rangeImage.point[i][j].range, 0,
//                                            maxDistance, rangeImage.point[i][j].type));

              if (rangeImage.isPositiveTurn()) {
                if (rangeImage.isSquareGrid()) {
                  g.fillRect((int)((i + scanStart) * pixelWidth),
                             (int)((rangeImage.scanPointNum / 2 - j) * pixelHeight),
                             (int)pixelWidth + 1,
                             (int)pixelHeight + 1);
                } else { // isHexagonalGrid
                  if ((i % 2) == 0) {
                    g.fillRect((int)((i + scanStart) * pixelWidth),
                               (int)((rangeImage.scanPointNum / 2 - j) * pixelHeight),
                               (int)pixelWidth + 1,
                               (int)pixelHeight + 1);
                  } else {
                    g.fillRect((int)((i + scanStart) * pixelWidth),
                               (int)((rangeImage.scanPointNum / 2 - j - 0.5) * pixelHeight),
                               (int)pixelWidth + 1,
                               (int)pixelHeight + 1);
                  }
                }
              } else { // isNegativeTurn
                if (rangeImage.isSquareGrid()) {
                  g.fillRect((int)((rangeImage.scanNum - i - 1 + scanStart) * pixelWidth),
                             (int)((rangeImage.scanPointNum / 2 - j) * pixelHeight),
                             (int)pixelWidth + 1,
                             (int)pixelHeight + 1);
              } else { // isHexagonalGrid
                if ((i % 2) == 0) {
                  g.fillRect((int)((rangeImage.scanNum - i - 1 + scanStart) * pixelWidth),
                             (int)((rangeImage.scanPointNum / 2 - j) * pixelHeight),
                             (int)pixelWidth + 1,
                             (int)pixelHeight + 1);
                } else {
                  g.fillRect((int)((rangeImage.scanNum - i - 1 + scanStart) * pixelWidth),
                             (int)((rangeImage.scanPointNum / 2 - j - 0.5) * pixelHeight),
                             (int)pixelWidth + 1,
                             (int)pixelHeight + 1);
                }
              }
            }
          }
        }
/*
        // paint second segment
        for(int i = 0; i < rangeImage.scanNum; i++) {
          for(int j = (rangeImage.scanPointNum / 2); j < rangeImage.scanPointNum; j++)
          {
            int    scanStart;
            if(rangeImage.isStart180())
            {
              scanStart = 0;
            }
            else // Start0
            {
              scanStart = rangeImage.scanNum;
            }

            g.setColor(createColorRangeType(rangeImage.point[i][j].range, 0, maxDistance, rangeImage.point[i][j].type));
//                        g.setColor(createColorRangeEdge(rangeImage.point[i][j].range, 0, maxDistance, rangeImage.point[i][j].type));

                        if(rangeImage.isPositiveTurn())
                        {
                            if(rangeImage.isSquareGrid())
                            {
                                g.fillRect((int)((i + scanStart) * pixelWidth), (int)((j - rangeImage.scanPointNum / 2) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                            }
                            else  // isHexagonalGrid
                            {
                                if((i % 2) == 0)
                                {
                                    g.fillRect((int)((i + scanStart) * pixelWidth), (int)((j - rangeImage.scanPointNum / 2) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                }
                                else
                                {
                                    g.fillRect((int)((i + scanStart) * pixelWidth), (int)((j - rangeImage.scanPointNum / 2 + 0.5) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                }
                            }
                        }
                        else  // isNegativeTurn
                        {
                            if(rangeImage.isSquareGrid())
                            {
                                g.fillRect((int)((rangeImage.scanNum - i - 1 + scanStart) * pixelWidth), (int)((j - rangeImage.scanPointNum / 2) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                            }
                            else  // isHexagonalGrid
                            {
                                if((i % 2) == 0)
                                {
                                    g.fillRect((int)((rangeImage.scanNum- i - 1 + scanStart) * pixelWidth), (int)((j - rangeImage.scanPointNum / 2) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                }
                                else
                                {
                                    g.fillRect((int)((rangeImage.scanNum - i - 1 + scanStart) * pixelWidth), (int)((j - rangeImage.scanPointNum / 2 + 0.5) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                }
                            }
                        }
                    }
                }*/
                break;

                case Scan3dDataMsg.TOP_DOWN:
                    pixelWidth = (float)width / (float)(rangeImage.scanNum * 2);
                    pixelHeight = (float)height / (float)(rangeImage.scanPointNum / 2);

                    // paint first segment
                    for(int i = 0; i < rangeImage.scanNum; i++)
                    {
                        for(int j = 0; j < (rangeImage.scanPointNum / 2); j++)
                        {
                            int    scanStart;
                            if(rangeImage.isStart180())
                            {
                                scanStart = rangeImage.scanNum;
                            }
                            else // Start0
                            {
                                scanStart = 0;
                            }

                            g.setColor(createColorRangeType(rangeImage.point[i][j].range, 0, maxDistance, rangeImage.point[i][j].type));

                            if(rangeImage.isPositiveTurn())
                            {
                                if(rangeImage.isSquareGrid())
                                {
//                                    g.fillRect((int)((i + scanStart) * pixelWidth), (int)((rangeImage.scanPointNum / 2 - j) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                    g.fillRect((int)((i + scanStart) * pixelWidth), (int)(j * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                }
                                else  // isHexagonalGrid
                                {
                                    if((i % 2) == 0)
                                    {
                                        g.fillRect((int)((i + scanStart) * pixelWidth), (int)((rangeImage.scanPointNum / 2 - j) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                    }
                                    else
                                    {
                                        g.fillRect((int)((i + scanStart) * pixelWidth), (int)((rangeImage.scanPointNum / 2 - j - 0.5) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                    }
                                }
                            }
                            else  // isNegativeTurn
                            {
                                if(rangeImage.isSquareGrid())
                                {
                                    g.fillRect((int)((rangeImage.scanNum - i - 1 + scanStart) * pixelWidth), (int)((rangeImage.scanPointNum / 2 - j) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                }
                                else  // isHexagonalGrid
                                {
                                    if((i % 2) == 0)
                                    {
                                        g.fillRect((int)((rangeImage.scanNum - i - 1 + scanStart) * pixelWidth), (int)((rangeImage.scanPointNum / 2 - j) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                    }
                                    else
                                    {
                                        g.fillRect((int)((rangeImage.scanNum - i - 1 + scanStart) * pixelWidth), (int)((rangeImage.scanPointNum / 2 - j - 0.5) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                    }
                                }
                            }
                        }
                    }
                    // paint second segment
                    for(int i = 0; i < rangeImage.scanNum; i++)
                    {
                        for(int j = (rangeImage.scanPointNum / 2); j < rangeImage.scanPointNum; j++)
                        {
                            int    scanStart;
                            if(rangeImage.isStart180())
                            {
                                scanStart = 0;
                            }
                            else // Start0
                            {
                                scanStart = rangeImage.scanNum;
                            }

                            g.setColor(createColorRangeType(rangeImage.point[i][j].range, 0, maxDistance, rangeImage.point[i][j].type));

                            if(rangeImage.isPositiveTurn())
                            {
                                if(rangeImage.isSquareGrid())
                                {
//                                    g.fillRect((int)((i + scanStart) * pixelWidth), (int)((j - rangeImage.scanPointNum / 2) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                    g.fillRect((int)((i + scanStart) * pixelWidth), (int)((rangeImage.scanPointNum - j) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                }
                                else  // isHexagonalGrid
                                {
                                    if((i % 2) == 0)
                                    {
                                        g.fillRect((int)((i + scanStart) * pixelWidth), (int)((j - rangeImage.scanPointNum / 2) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                    }
                                    else
                                    {
                                        g.fillRect((int)((i + scanStart) * pixelWidth), (int)((j - rangeImage.scanPointNum / 2 + 0.5) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                    }
                                }
                            }
                            else  // isNegativeTurn
                            {
                                if(rangeImage.isSquareGrid())
                                {
                                    g.fillRect((int)((rangeImage.scanNum - i - 1 + scanStart) * pixelWidth), (int)((j - rangeImage.scanPointNum / 2) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                }
                                else  // isHexagonalGrid
                                {
                                    if((i % 2) == 0)
                                    {
                                        g.fillRect((int)((rangeImage.scanNum- i - 1 + scanStart) * pixelWidth), (int)((j - rangeImage.scanPointNum / 2) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                    }
                                    else
                                    {
                                        g.fillRect((int)((rangeImage.scanNum - i - 1 + scanStart) * pixelWidth), (int)((j - rangeImage.scanPointNum / 2 + 0.5) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                    }
                                }
                            }
                        }
                    }
                    break;

            case Scan3dDataMsg.YAW:
            default:
                pixelWidth = (float)width / (float)rangeImage.scanNum;
                pixelHeight = (float)height / (float)rangeImage.scanPointNum;

                for(int i = 0; i < rangeImage.scanNum; i++)
                {
                    for(int j = 0; j < rangeImage.scanPointNum; j++)
                    {
                        g.setColor(createColorRangeType(rangeImage.point[i][j].range, 0, maxDistance, rangeImage.point[i][j].type));
//                        g.setColor(createColorRangeEdge(rangeImage.point[i][j].range, 0, maxDistance, rangeImage.point[i][j].type));
//                        g.setColor(createColorRangeSegment(rangeImage.point[i][j].range, 0, maxDistance, rangeImage.point[i][j].type));

                        if(rangeImage.isPositiveTurn())
                        {
                            if(rangeImage.isSquareGrid())
                            {
                                g.fillRect((int)(i * pixelWidth), (int)((rangeImage.scanPointNum - j - 1) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                            }
                            else  // isHexagonalGrid
                            {
                                if((i % 2) == 0)
                                {
                                    g.fillRect((int)(i * pixelWidth), (int)((rangeImage.scanPointNum - j - 1) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                }
                                else
                                {
                                    g.fillRect((int)(i * pixelWidth), (int)((rangeImage.scanPointNum - j - 1.5) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                }
                            }
                        }
                        else  // isNegativeTurn
                        {
                            if(rangeImage.isSquareGrid())
                            {
                                g.fillRect((int)((rangeImage.scanNum - i - 1) * pixelWidth), (int)((rangeImage.scanPointNum - j - 1) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                            }
                            else  // isHexagonalGrid
                            {
                                if((i % 2) == 0)
                                {
                                    g.fillRect((int)((rangeImage.scanNum - i - 1) * pixelWidth), (int)((rangeImage.scanPointNum - j - 1) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                }
                                else
                                {
                                    g.fillRect((int)((rangeImage.scanNum - i - 1) * pixelWidth), (int)((rangeImage.scanPointNum - j - 1.5) * pixelHeight), (int)pixelWidth + 1, (int)pixelHeight + 1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    public static Scan3dComponent matlabMain()
    {
        Scan3dComponent instance = new Scan3dComponent(20000);

        JFrame frame = new JFrame();
        frame.setTitle("Scan3d");
        frame.getContentPane().add(instance);
        frame.setSize(720,360);
        frame.setVisible(true);

        return(instance);
    }

/*    public void plotS3(double[][] s3ListMatlab)
    {
        Scan3dRangeImagePackage ri = new Scan3dRangeImagePackage();

        ri.loadS3ListMatlab(s3ListMatlab);

        updateData(ri);
    }

    public void plotS2(double[][] s2ListMatlab)
    {
        Scan3dRangeImagePackage ri = new Scan3dRangeImagePackage();

        ri.loadS2ListMatlab(s2ListMatlab);

        updateData(ri);
    }
*/

}
