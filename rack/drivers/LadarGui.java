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
package rack.drivers;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import rack.main.gui.*;
import rack.main.proxy.*;

public class LadarGui extends RackModuleGui
{
    protected boolean terminate = false;
    public int maxDistance = 10000; // 10m
    public LadarDataMsg ladarData;
    protected LadarProxy ladar;
    protected LadarComponent ladarComponent;

    protected JButton onButton;
    protected JButton offButton;
    protected JButton zoomOutButton;
    protected JButton zoomInButton;
    protected JPanel panel;
    protected JPanel wButtonPanel;
    protected JPanel eButtonPanel;
    protected JPanel northPanel;
    protected Point aktuellPoint;
    protected Point mousePressedPoint;
    protected Point mouseReleasedPoint;
    protected Point mouseClickedPoint;

    public LadarGui(LadarProxy proxy)
    {
        ladar = proxy;
        panel = new JPanel(new BorderLayout(2, 2));
        panel.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));

        northPanel = new JPanel(new BorderLayout(2, 2));
        wButtonPanel = new JPanel(new GridLayout(1, 0, 4, 2));
        eButtonPanel = new JPanel(new GridLayout(1, 0, 4, 2));
        onButton = new JButton("On");
        offButton = new JButton("Off");
        zoomOutButton = new JButton("Zoom out");
        zoomInButton = new JButton("Zoom in");

        ladarComponent = new LadarComponent(maxDistance);

        ladarComponent.addMouseMotionListener(new MouseMotionListener()
        {
            public void mouseDragged(MouseEvent e)
            {
                // if(e.getButton()==MouseEvent.BUTTON1 ){
                aktuellPoint = e.getPoint();
                ladarComponent.drawRec(mousePressedPoint, aktuellPoint);
                // }
            }

            public void mouseMoved(MouseEvent e)
            {
            }
        });

        ladarComponent.addMouseListener(new MouseListener()
        {
            public void mouseClicked(MouseEvent e)
            {
                // if(e.getButton()==MouseEvent.BUTTON1 ){
                mouseClickedPoint = e.getPoint();
                ladarComponent.setCenter(mouseClickedPoint);
                // }
            }

            public void mousePressed(MouseEvent e)
            {
                // if(e.getButton()==MouseEvent.BUTTON1 ){
                mousePressedPoint = e.getPoint();
                // }
            }

            public void mouseReleased(MouseEvent e)
            {
                // if(e.getButton()==MouseEvent.BUTTON1 ){
                mouseReleasedPoint = e.getPoint();
                ladarComponent.select(mousePressedPoint, mouseReleasedPoint);
                // }
            }

            public void mouseEntered(MouseEvent e)
            {
            }

            public void mouseExited(MouseEvent e)
            {
            }
        });

        ladarComponent.addKeyListener(new myKeyListener());

        onButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                ladar.on();
            }
        });

        onButton.addKeyListener(new myKeyListener());

        offButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                ladar.off();
            }
        });

        offButton.addKeyListener(new myKeyListener());

        zoomOutButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                maxDistance = maxDistance * 2;
                ladarComponent.setMaxDistance(maxDistance);
            }
        });

        zoomOutButton.addKeyListener(new myKeyListener());

        zoomInButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                maxDistance = maxDistance / 2;
                ladarComponent.setMaxDistance(maxDistance);
            }
        });

        zoomInButton.addKeyListener(new myKeyListener());

        wButtonPanel.add(onButton);
        wButtonPanel.add(offButton);
        eButtonPanel.add(zoomOutButton);
        eButtonPanel.add(zoomInButton);

        // northPanel.add(new JLabel("ladar"),BorderLayout.NORHT);
        northPanel.add(wButtonPanel, BorderLayout.WEST);
        northPanel.add(eButtonPanel, BorderLayout.EAST);

        panel.add(northPanel, BorderLayout.NORTH);
        panel.add(ladarComponent, BorderLayout.CENTER);
    }

    public JComponent getComponent()
    {
        return panel;
    }

    public String getModuleName()
    {
        return ("Ladar");
    }

    public RackProxy getProxy()
    {
        return (ladar);
    }

    public void run()
    {
        LadarDataMsg data = null;

        while (terminate == false)
        {
            if (panel.isShowing())
            {
                data = ladar.getData();
                if (data != null)
                {
                    ladarData = data;
                    ladarComponent.updateData(data);
                }
            }
            try
            {
                Thread.sleep(1000);
            }
            catch (InterruptedException e)
            {

            }
            catch (Throwable t)
            {
                System.out.println(t);
                t.printStackTrace();
            }
        }
    }

    public void terminate()
    {
        terminate = true;
    }

    class myKeyListener implements KeyListener
    {
        public void keyPressed(KeyEvent e)
        {
            if (e.getKeyCode() == KeyEvent.VK_RIGHT)
                ladarComponent.right();
            if (e.getKeyCode() == KeyEvent.VK_DOWN)
                ladarComponent.down();
            if (e.getKeyCode() == KeyEvent.VK_LEFT)
                ladarComponent.left();
            if (e.getKeyCode() == KeyEvent.VK_UP)
                ladarComponent.up();
        }

        public void keyReleased(KeyEvent e)
        {
            System.out.println("just you!");
        }

        public void keyTyped(KeyEvent e)
        {
        }

    }
}
