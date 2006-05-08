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
import java.awt.event.*;
import javax.swing.*;

import rack.main.gui.*;
import rack.main.defines.*;
import rack.main.proxy.*;


public class Scan2DGui extends RackModuleGui
{
    protected boolean            terminate=false;
    protected boolean            mapViewIsShowing=false;
    public    int                maxDistance=10000; // 10m
    public    Scan2DDataMsg      scan2DData;
    protected Scan2DProxy        scan2D;
    protected Scan2DComponent    scan2DComponent;

    protected JButton onButton;
    protected JButton offButton;
    protected JButton zoomOutButton;
    protected JButton zoomInButton;

    protected JPanel panel;
    protected JPanel northPanel;
    protected JPanel wButtonPanel;
    protected JPanel eButtonPanel;

    protected Point aktuellPoint;
    protected Point mousePressedPoint;
    protected Point mouseReleasedPoint;
    protected Point mouseClickedPoint;

    public Scan2DGui(Scan2DProxy proxy)
    {
        scan2D = proxy;
        panel       = new JPanel(new BorderLayout(2,2));
        panel.setBorder(BorderFactory.createEmptyBorder(4,4,4,4));

        northPanel   = new JPanel(new BorderLayout(2,2));
        wButtonPanel = new JPanel(new GridLayout(1,0,4,2));
        eButtonPanel = new JPanel(new GridLayout(1,0,4,2));

        onButton      = new JButton("On");
        offButton     = new JButton("Off");
        zoomOutButton = new JButton("Zoom out");
        zoomInButton  = new JButton("Zoom in");

        scan2DComponent = new Scan2DComponent(maxDistance);

        scan2DComponent.addMouseListener(
          new MouseListener() {
            public void mouseClicked(MouseEvent e)
            {
              mouseClickedPoint = e.getPoint();
              scan2DComponent.setCenter(mouseClickedPoint);
            }

            public void mousePressed(MouseEvent e)
            {
              mousePressedPoint = e.getPoint();
            }

            public void mouseReleased(MouseEvent e)
            {
              mouseReleasedPoint = e.getPoint();
              //scan2DComponent.select(mousePressedPoint , mouseReleasedPoint);
            }

            public void mouseEntered(MouseEvent e) {}
            public void mouseExited(MouseEvent e) {}
          }
        );

        onButton.addActionListener(
          new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                scan2D.on();
            }
          }
        );

        onButton.addKeyListener(new myKeyListener());

        offButton.addActionListener(
          new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                scan2D.off();
            }
          }
        );

        offButton.addKeyListener(new myKeyListener());

        zoomOutButton.addActionListener(
          new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
              maxDistance = maxDistance * 2;
              scan2DComponent.setMaxDistance(maxDistance);
            }
          }
        );

        zoomOutButton.addKeyListener(new myKeyListener());

        zoomInButton.addActionListener(
          new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
              maxDistance = maxDistance / 2;
              scan2DComponent.setMaxDistance(maxDistance);
            }
          }
        );

        zoomInButton.addKeyListener(new myKeyListener());

        wButtonPanel.add(onButton);
        wButtonPanel.add(offButton);
        eButtonPanel.add(zoomOutButton);
        eButtonPanel.add(zoomInButton);

        northPanel.add(wButtonPanel,BorderLayout.WEST);
        northPanel.add(eButtonPanel,BorderLayout.EAST);

        panel.add(northPanel,BorderLayout.NORTH);
        panel.add(scan2DComponent,BorderLayout.CENTER);

    }

    public JComponent getComponent()
    {
      return panel;
    }

    public String getModuleName()
    {
      return("Scan2D");
    }


    public RackProxy getProxy()
    {
      return(scan2D);
    }

    public void run()
    {
      Scan2DDataMsg data;

      while (terminate == false) {
        if(panel.isShowing() | (mapViewIsShowing)) {
          data = scan2D.getData();
          if (data != null) {
            scan2DData = data;
            scan2DComponent.updateData(data);
          }
          mapViewIsShowing = false;
        }
        try {
          Thread.sleep(1000);
        } catch(InterruptedException e) {}
      }
    }

    public boolean hasMapView() 
    {
        return true;
    }
    
    public void paintMapView(MapViewDrawContext drawContext) 
    {
        mapViewIsShowing = true;
        
        if (scan2DData == null) return;
        
        Graphics2D robotGraphics = drawContext.getRobotGraphics();

        for (int i = 0; i < scan2DData.pointNum; i++) 
        {
            ScanPoint point = scan2DData.point[i];
            int type = point.type & 0xff;
            int size = 50;
            int dist;            
            
            if(type == 0) 
            { 
                robotGraphics.setColor(Color.red); 
            }
            else 
            {              
                switch((type - 1) % 7) 
                {
                    case 0:
                        robotGraphics.setColor(Color.blue);
                        break;
                    case 1:
                        robotGraphics.setColor(Color.cyan);
                        break;
                    case 2:
                        robotGraphics.setColor(Color.green);
                        break;
                    case 3:
                        robotGraphics.setColor(Color.magenta);
                        break;
                    case 4:
                        robotGraphics.setColor(Color.orange);
                        break;
                    case 5:
                        robotGraphics.setColor(Color.pink);
                        break;
                    case 6:
                        robotGraphics.setColor(Color.yellow);
                        break;
                }
            }
            // draw scanpoints in mm
            dist  = (int)Math.sqrt(point.x * point.x + point.y * point.y);
            size += (int)(dist * 0.025);
            robotGraphics.fillArc(point.x - size/2, point.y - size/2,
                                  size, size, 0, 360);
        }
    }    
    
    public void terminate()
    {
       terminate=true;
    }

    class myKeyListener implements KeyListener {
      public void keyPressed(KeyEvent e)
      {
        if(e.getKeyCode()==KeyEvent.VK_RIGHT) scan2DComponent.right();
        if(e.getKeyCode()==KeyEvent.VK_DOWN)  scan2DComponent.down();
        if(e.getKeyCode()==KeyEvent.VK_LEFT)  scan2DComponent.left();
        if(e.getKeyCode()==KeyEvent.VK_UP)    scan2DComponent.up();
      }

      public void keyReleased(KeyEvent e)
      {
        System.out.println("just you!");
      }

      public void keyTyped(KeyEvent e) {}
    }
}
