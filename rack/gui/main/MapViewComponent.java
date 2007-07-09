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
 *      Oliver Wulf <oliver.wulf@web.de>
 *
 */
package rack.gui.main;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GridLayout;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JPanel;

import rack.navigation.PositionDataMsg;

public class MapViewComponent extends JComponent
{
    private static final long          serialVersionUID = 1L;

    public static double               ZOOM_FACTOR = 1.141;
    public static double               VISIBLE_RANGE = 5000.0;

    protected Vector<MapViewInterface> mapViews         = new Vector<MapViewInterface>();

    protected Vector<PositionDataMsg>  robotPosition;
    protected double                   visibleRange;
    protected double                   centerX;
    protected double                   centerY;
    
    public JPanel                      zoomPanel;
    public JButton                     zoomInButton;
    public JButton                     zoomOutButton;
    public JButton                     zoomCenterButton;

    protected Point                    mousePressed;

    public MouseListener               mouseListener = new MapViewComponentMouseListener();
    public KeyListener                 keyListener = new MapViewComponentKeyListener();
    
    public MapViewComponent()
    {
        this.setDoubleBuffered(true);
        this.setBackground(Color.WHITE);
        this.setPreferredSize(new Dimension(400, 400));
        this.visibleRange = VISIBLE_RANGE;

        this.robotPosition = new Vector<PositionDataMsg>();
        robotPosition.add(new PositionDataMsg());
        
        this.addKeyListener(keyListener);
        this.addMouseListener(mouseListener);
        this.addMouseMotionListener((MouseMotionListener)mouseListener);
        this.addMouseWheelListener((MouseWheelListener)mouseListener);
        
        zoomInButton = new JButton("zoom in");
        zoomInButton.addKeyListener(keyListener);
        zoomInButton.addActionListener( new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                zoomIn();
            }
        });

        zoomOutButton = new JButton("zoom out");
        zoomOutButton.addKeyListener(keyListener);
        zoomOutButton.addActionListener( new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                zoomOut();
            }
        });
        
        zoomCenterButton = new JButton("center");
        zoomCenterButton.addKeyListener(keyListener);
        zoomCenterButton.addActionListener( new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                zoomCenter();
            }
        });

        zoomPanel = new JPanel(new GridLayout(1, 0, 4, 2));
        zoomPanel.add(zoomInButton);
        zoomPanel.add(zoomOutButton);
        zoomPanel.add(zoomCenterButton);
    }

    public void setEnabled(boolean enabled)
    {
        zoomInButton.setEnabled(enabled);
        zoomOutButton.setEnabled(enabled);
        zoomCenterButton.setEnabled(enabled);
    }
    
    public void addMapView(MapViewInterface mapView)
    {
        this.mapViews.add(mapView);
    }
    
    public void removeMapView(MapViewInterface mapView)
    {
        this.mapViews.remove(mapView);
    }

    public Vector<PositionDataMsg> getRobotPositionVector()
    {
        return robotPosition;
    }
    
    protected void paintGrid(Graphics2D g, Color color, int num, int step)
    {
        g.setColor(color);

        for (int x = -num; x <= num; x++)
        {
            g.drawLine(x * step, -num * step, x * step, num * step);
        }

        for (int y = -num; y <= num; y++)
        {
            g.drawLine(-num * step, y * step, num * step, y * step);
        }
    }

    protected void paintComponent(Graphics g)
    {
        if (isOpaque()) // paint background
        {
            g.setColor(getBackground());
            g.fillRect(0, 0, getWidth(), getHeight());
        }

        Graphics2D frame = (Graphics2D)g.create();
        Graphics2D world = (Graphics2D)g.create();

        Dimension size = this.getSize();

        world.translate(centerX, centerY);
        world.rotate(-Math.PI/2.0);

        double scale;
        if(size.height > size.width)
        {
            scale = (double)size.width / 2.0 / visibleRange;
        }
        else
        {
            scale = (double)size.height / 2.0 / visibleRange;
        }
        world.scale(scale, scale);

        paintGrid(world, Color.LIGHT_GRAY, 100, 1000);  // 100 * 1m
        paintGrid(world, Color.BLACK, 10, 10000);  // 10 * 10m

        for(int i = 0; i < mapViews.size(); i++)
        {
            mapViews.get(i).paintMapView(new MapViewGraphics(frame, world, robotPosition));
        }
        
        world.dispose(); // clean up
    }

    public void zoomIn()
    {
        visibleRange = visibleRange / ZOOM_FACTOR;

        Dimension size = this.getSize();

        double windowCenterX = (double)size.width / 2.0;
        double windowCenterY = (double)size.height / 2.0;

        centerX = ((centerX - windowCenterX) * ZOOM_FACTOR) + windowCenterX;
        centerY = ((centerY - windowCenterY) * ZOOM_FACTOR) + windowCenterY;

        repaint();
    }
    
    public void zoomOut()
    {
        visibleRange = visibleRange * ZOOM_FACTOR;

        Dimension size = this.getSize();

        double windowCenterX = (double)size.width / 2.0;
        double windowCenterY = (double)size.height / 2.0;

        centerX = ((centerX - windowCenterX) / ZOOM_FACTOR) + windowCenterX;
        centerY = ((centerY - windowCenterY) / ZOOM_FACTOR) + windowCenterY;

        repaint();
    }
    
    public void zoomCenter()
    {
        visibleRange = VISIBLE_RANGE;

        Dimension size = this.getSize();

        double windowCenterX = (double)size.width / 2.0;
        double windowCenterY = (double)size.height / 2.0;

        centerX = windowCenterX;
        centerY = windowCenterY;

        repaint();
    }
    
    protected class MapViewComponentMouseListener extends MouseAdapter
            implements MouseMotionListener, MouseWheelListener
    {
        public void mouseClicked(MouseEvent e)
        {
            //Point mouse = e.getPoint();
        }

        public void mousePressed(MouseEvent e)
        {
            mousePressed = e.getPoint();
        }

        public void mouseReleased(MouseEvent e)
        {
            mousePressed = null;
        }

        public void mouseDragged(MouseEvent e)
        {
            if(mousePressed != null)
            {
                Point mouse = e.getPoint();
                centerX += (double)(mouse.x - mousePressed.x);
                centerY += (double)(mouse.y - mousePressed.y);
                mousePressed = mouse;
                repaint();
            }
        }

        public void mouseMoved(MouseEvent e)
        {
        }
        
        public void mouseWheelMoved(MouseWheelEvent e)
        {
            if(e.getWheelRotation() > 0)
            {
                zoomIn();
            }
            else if(e.getWheelRotation() < 0)
            {
                zoomOut();
            }
        }
    };

    protected class MapViewComponentKeyListener extends KeyAdapter
    {
        public void keyPressed(KeyEvent e)
        {
            if (e.getKeyCode() == KeyEvent.VK_UP)
            {
                centerY += 10;
                repaint();
            }
            if (e.getKeyCode() == KeyEvent.VK_DOWN)
            {
                centerY -= 10;
                repaint();
            }
            if (e.getKeyCode() == KeyEvent.VK_LEFT)
            {
                centerX += 10;
                repaint();
            }
            if (e.getKeyCode() == KeyEvent.VK_RIGHT)
            {
                centerX -= 10;
                repaint();
            }
            if (e.getKeyCode() == KeyEvent.VK_PLUS)
            {
                zoomIn();
            }
            if (e.getKeyCode() == KeyEvent.VK_MINUS)
            {
                zoomOut();
            }
            if (e.getKeyCode() == KeyEvent.VK_HOME)
            {
                centerX = 0;
                centerY = 0;
                repaint();
            }
        }
    };
}
