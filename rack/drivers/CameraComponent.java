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
 *      Marko Reimer     <reimer@l3s.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.drivers;

import java.awt.*;
import java.awt.image.MemoryImageSource;
import javax.swing.*;
import rack.main.defines.RackMouseListener;

public class CameraComponent extends JComponent
{

    public CameraComponent()
    {
        this.setPreferredSize(new Dimension(320, 240));
        this.setBackground(Color.lightGray);
        setDoubleBuffered(true);
    }

    public void setMousePositionLabel(JLabel positionLabel)
    {
        this.addMouseListener(new RackMouseListener(positionLabel));
        this.addMouseMotionListener(new RackMouseListener(positionLabel));
    }

    public void transformImage(int zoomRate, int switchRotate,
            CameraDataMsg dataMsg)
    {
        CameraDataMsg switchRotatedDataMsg;
        CameraDataMsg zoomDataMsg;

        switchRotatedDataMsg = switchAndRotate(switchRotate, dataMsg);
        zoomDataMsg = switchRotatedDataMsg;// zoom(zoomRate,
                                                    // switchRotatedDataMsg);

        img = createImage(new MemoryImageSource(zoomDataMsg.width,
                zoomDataMsg.height, zoomDataMsg.imageRawData, 0,
                zoomDataMsg.width));

        if (zoomRate > 0)
        {
            zoomDataMsg.width = zoomDataMsg.width * zoomRate;
            zoomDataMsg.height = zoomDataMsg.height * zoomRate;
            img = img.getScaledInstance(zoomDataMsg.width,
                    zoomDataMsg.height, Image.SCALE_FAST);
        }
        if (zoomRate < 0)
        {
            zoomDataMsg.width = zoomDataMsg.width / -zoomRate;
            zoomDataMsg.height = zoomDataMsg.height / -zoomRate;
            img = img.getScaledInstance(zoomDataMsg.width,
                    zoomDataMsg.height, Image.SCALE_FAST);
        }

        this.setPreferredSize(new Dimension(zoomDataMsg.width,
                zoomDataMsg.height));
        this.setSize(new Dimension(zoomDataMsg.width,
                zoomDataMsg.height));

        this.repaint();
    }

    public CameraDataMsg switchAndRotate(int switchRotate,
            CameraDataMsg dataMsg)
    {

        CameraDataMsg switchRotateData;
        int lineCounter, colCounter;

        switch (switchRotate)
        {
            case 0: // none
            default:
                // nothing to do...
                switchRotateData = dataMsg;
                break;
            case 1: // 90 right
                // dimensions are changed width<->height
                switchRotateData = new CameraDataMsg(dataMsg.height,
                        dataMsg.width);

                for (lineCounter = 0; lineCounter < dataMsg.height; lineCounter++)
                {
                    for (colCounter = 0; colCounter < dataMsg.width; colCounter++)
                    {
                        // read in order of input image. process the old image
                        // line wise.
                        switchRotateData.imageRawData[(dataMsg.height
                                - lineCounter - 1)
                                + (colCounter * dataMsg.height)] = dataMsg.imageRawData[lineCounter
                                * dataMsg.width + colCounter];
                    }
                }
                break;
            case 2: // hor flip
                // dimensions are stable
                switchRotateData = new CameraDataMsg(dataMsg.width,
                        dataMsg.height);

                for (lineCounter = 0; lineCounter < dataMsg.height; lineCounter++)
                {
                    for (colCounter = 0; colCounter < dataMsg.width; colCounter++)
                    {
                        switchRotateData.imageRawData[(lineCounter * dataMsg.width)
                                + (dataMsg.width - colCounter - 1)] = dataMsg.imageRawData[lineCounter
                                * dataMsg.width + colCounter];
                    }
                }
                break;
            case 3: // 90 right and hor flip
                // dimensions are changed width<->height
                switchRotateData = new CameraDataMsg(dataMsg.height,
                        dataMsg.width);

                for (lineCounter = 0; lineCounter < dataMsg.height; lineCounter++)
                {
                    for (colCounter = 0; colCounter < dataMsg.width; colCounter++)
                    {
                        // read in order of input image. process the old image
                        // line wise backward.
                        switchRotateData.imageRawData[(dataMsg.height
                                - lineCounter - 1)
                                + (colCounter * dataMsg.height)] = dataMsg.imageRawData[(dataMsg.height
                                - lineCounter - 1)
                                * dataMsg.width + (colCounter)];
                    }
                }
                break;
            case 4: // 90 right and ver flip
                // dimensions are changed width<->height
                switchRotateData = new CameraDataMsg(dataMsg.height,
                        dataMsg.width);

                for (lineCounter = 0; lineCounter < dataMsg.height; lineCounter++)
                {
                    for (colCounter = 0; colCounter < dataMsg.width; colCounter++)
                    {
                        // read in order of input image. process the old image
                        // line wise backward.
                        switchRotateData.imageRawData[(dataMsg.height
                                - lineCounter - 1)
                                + (colCounter * dataMsg.height)] = dataMsg.imageRawData[lineCounter
                                * dataMsg.width
                                + (dataMsg.width - colCounter - 1)];
                    }
                }
                break;
        }
        switchRotateData.recordingtime = dataMsg.recordingtime;
        switchRotateData.mode = dataMsg.mode;
        switchRotateData.depth = dataMsg.depth;

        return switchRotateData;
    }

    public CameraDataMsg zoom(int scale, CameraDataMsg dataMsg)
    {
        if ((scale == 0) | (scale == 1))
        {
            return dataMsg;
        }
        else
        {
            int newWidth = 0;
            int newHeight = 0;

            if (scale < 0)
            {
                newWidth = dataMsg.width / ((-1) * scale);
                newHeight = dataMsg.height / ((-1) * scale);
            }
            else
            {
                newWidth = dataMsg.width * scale;
                newHeight = dataMsg.height * scale;
            }

            CameraDataMsg zoomData = new CameraDataMsg(newWidth,
                    newHeight);

            zoomData.recordingtime = dataMsg.recordingtime;
            zoomData.mode = dataMsg.mode;
            zoomData.depth = dataMsg.depth;

            if (scale > 1) // we have to zoom in (display one value several
                            // times)
            {
                for (int h = 0; h < newHeight; h++)
                {
                    for (int w = 0; w < newWidth; w++)
                    {
                        zoomData.imageRawData[h * newWidth + w] = dataMsg.imageRawData[h
                                / scale * dataMsg.width + w / scale];
                    }
                }
            }
            else
            {// scale < 0
                for (int h = 0; h < newHeight; h++)
                {
                    for (int w = 0; w < newWidth; w++)
                    {
                        zoomData.imageRawData[h * newWidth + w] = dataMsg.imageRawData[h
                                * (-1)
                                * scale
                                * dataMsg.width
                                + w
                                * (-1)
                                * scale];
                    }
                }
            }
            return zoomData;
        }
    }

    public void paintComponent(Graphics g)
    {
        if (img != null)
        {
            g.drawImage(img, 0, 0, this);
        }
    }

    protected Image img;
}
