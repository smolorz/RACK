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
package rack.gui.drivers;

import java.awt.*;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.image.MemoryImageSource;
import javax.swing.*;

import rack.drivers.CameraDataMsg;
import rack.main.defines.ImageRect;

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
        this.addMouseListener(new MyMouseListener(positionLabel));
        this.addMouseMotionListener(new MyMouseListener(positionLabel));
    }

    public void transformImage(int zoomRate, int switchRotate,
            CameraDataMsg dataMsg)
    {
        CameraDataMsg switchRotatedDataMsg;
        CameraDataMsg zoomDataMsg;
        int newWidth, newHeight; 

        switchRotatedDataMsg = switchAndRotate(switchRotate, dataMsg);
        zoomDataMsg = switchRotatedDataMsg;// zoom(zoomRate,
                                                    // switchRotatedDataMsg);

        synchronized(this)
        {
            img = createImage(new MemoryImageSource(zoomDataMsg.width,
                    zoomDataMsg.height, zoomDataMsg.imageRawData, 0,
                    zoomDataMsg.width));

            newWidth  = zoomDataMsg.width; 
            newHeight = zoomDataMsg.height;
    
            if (zoomRate > 0)
            {
            	newWidth  = zoomDataMsg.width  * zoomRate;
            	newHeight = zoomDataMsg.height * zoomRate;
                img = img.getScaledInstance(newWidth,
                        newHeight, Image.SCALE_FAST);
                //zoomDataMsg.width = zoomDataMsg.width * zoomRate;
                //zoomDataMsg.height = zoomDataMsg.height * zoomRate;
                //img = img.getScaledInstance(zoomDataMsg.width,
                //        zoomDataMsg.height, Image.SCALE_FAST);
            }
            if (zoomRate < 0)
            {
                newWidth  = zoomDataMsg.width  / -zoomRate;
                newHeight = zoomDataMsg.height / -zoomRate;
                img = img.getScaledInstance(newWidth,
                        newHeight, Image.SCALE_FAST);
            }
        }

        this.setPreferredSize(new Dimension(newWidth,
                newHeight));
        this.setSize(new Dimension(newWidth,
                newHeight));

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
        switchRotateData.recordingTime = dataMsg.recordingTime;
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

            zoomData.recordingTime = dataMsg.recordingTime;
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


    public void setRects(int number, ImageRect[] newRects)
    {
        rectNumber = number;
        rects        = newRects;
    }

    public void paintComponent(Graphics g)
    {
    	Graphics2D g2 = (Graphics2D)g;
    	
        synchronized(this)
        {
            if (img != null)
            {
                g2.drawImage(img, 0, 0, this);
    
                g2.setColor(Color.GREEN);
                g2.setStroke(new BasicStroke(5));
                for (int a = 0; a < rectNumber; a++)
                {
                    g2.drawRect(rects[a].x, rects[a].y,
                                rects[a].width, rects[a].height);
                }
            }
        }
    }

    public class MyMouseListener implements MouseListener, MouseMotionListener
    {

        JLabel outputLabel;
        int mx, my; // the mouse coordinates
        boolean isButtonPressed = false;

        public MyMouseListener(JLabel positionLabel)
        {
            outputLabel = positionLabel;
        }

        public void mouseEntered(MouseEvent e)
        {
            // called when the pointer enters the applet's rectangular area
        }

        public void mouseExited(MouseEvent e)
        {
            // called when the pointer leaves the applet's rectangular area
        }

        public void mouseClicked(MouseEvent e)
        {
            // called after a press and release of a mouse button
            // with no motion in between
            // (If the user presses, drags, and then releases, there will be
            // no click event generated.)
        }

        public void mousePressed(MouseEvent e)
        { // called after a button is pressed down
            isButtonPressed = true;
            // "Consume" the event so it won't be processed in the
            // default manner by the source which generated it.
            e.consume();
        }

        public void mouseReleased(MouseEvent e)
        { // called after a button is released
            isButtonPressed = false;
            e.consume();
        }

        public void mouseMoved(MouseEvent e)
        { // called during motion when no buttons are down
            mx = e.getX();
            my = e.getY();
            outputLabel.setText("Mouse at (" + mx + "," + my + ")");
            // showStatus( "Mouse at (" + mx + "," + my + ")" );
            e.consume();
        }

        public void mouseDragged(MouseEvent e)
        { // called during motion with buttons down
            mx = e.getX();
            my = e.getY();
            outputLabel.setText("Mouse at (" + mx + "," + my + ")");
            // showStatus( "Mouse at (" + mx + "," + my + ")" );
            e.consume();
        }

    }

    protected Image img;
    protected ImageRect[] rects;
    protected int rectNumber = 0;

    private static final long serialVersionUID = 1L;
}
