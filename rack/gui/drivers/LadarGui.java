/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2007 University of Hannover
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
import java.awt.event.*;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.RackModuleGui;
import rack.drivers.LadarDataMsg;
import rack.drivers.LadarProxy;

public class LadarGui extends RackModuleGui
{
    public int               maxDistance = 10000; // 10m
    public LadarDataMsg      ladarData;
    protected LadarProxy     ladar;
    protected LadarComponent ladarComponent;

    protected JButton        zoomOutButton;
    protected JButton        zoomInButton;
    protected Point          aktuellPoint;
    protected Point          mousePressedPoint;
    protected Point          mouseReleasedPoint;
    protected Point          mouseClickedPoint;

    public LadarGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);
        
        ladar = (LadarProxy) proxy;

        updateTime = 200;

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));
        JPanel wButtonPanel = new JPanel(new GridLayout(1, 0, 4, 2));
        JPanel eButtonPanel = new JPanel(new GridLayout(1, 0, 4, 2));

        zoomOutButton = new JButton("Zoom out");
        zoomInButton = new JButton("Zoom in");

        ladarComponent = new LadarComponent(maxDistance);

        ladarComponent.addMouseMotionListener(new MouseMotionListener() {
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

        ladarComponent.addMouseListener(new MouseListener() {
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

        onButton.addKeyListener(new myKeyListener());
        offButton.addKeyListener(new myKeyListener());

        zoomOutButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                maxDistance = maxDistance * 2;
                ladarComponent.setMaxDistance(maxDistance);
            }
        });

        zoomOutButton.addKeyListener(new myKeyListener());

        zoomInButton.addActionListener(new ActionListener() {
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

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(ladarComponent, BorderLayout.CENTER);
        
        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        zoomInButton.setEnabled(enabled);
        zoomOutButton.setEnabled(enabled);
    }
    
    protected void updateData()
    {
        LadarDataMsg data = null;

        data = ladar.getData();
        if (data != null)
        {
            ladarData = data;
            ladarComponent.updateData(data);
            
            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
    }

    class myKeyListener extends KeyAdapter
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
    }
}
