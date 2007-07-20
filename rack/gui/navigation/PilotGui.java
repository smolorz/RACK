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
package rack.gui.navigation;

import java.awt.*;
import java.awt.event.*;
import java.util.StringTokenizer;

import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.MapViewActionEvent;
import rack.gui.main.MapViewComponent;
import rack.gui.main.MapViewGraphics;
import rack.gui.main.MapViewGui;
import rack.gui.main.MapViewInterface;
import rack.gui.main.RackModuleGui;
import rack.main.defines.Position3d;
import rack.navigation.PilotDataMsg;
import rack.navigation.PilotProxy;
import rack.navigation.PilotDestMsg;

public class PilotGui extends RackModuleGui implements MapViewInterface
{
    protected PilotDataMsg     pilotData;
    protected PilotProxy       pilot;
    protected MapViewComponent mapComponent;

    protected JButton          destinationButton;
    protected PilotDestMsg     pilotDest   = new PilotDestMsg();

    protected String           setDestinationCommand;

    protected boolean          mapViewIsShowing;
    protected MapViewGui       mapViewGui;
    
    public PilotGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        pilot = (PilotProxy) proxy;

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));
        JPanel buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));

        mapComponent = new MapViewComponent();
        mapComponent.addMapView(this);
        mapComponent.setPreferredSize(new Dimension(200,200));
        mapComponent.setDefaultVisibleRange(5000.0);

        destinationButton = new JButton("Set destination");
        destinationButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                String s = (String) JOptionPane.showInputDialog(null, "Destination is:\n" + "x, y, rho, moveDir",
                        "Destination", JOptionPane.PLAIN_MESSAGE, null, null, "0,0,0.0,0");
                if ((s != null) && (s.length() > 0))
                {
                    StringTokenizer st = new StringTokenizer(s, ",");
                    if (st.countTokens() == 4)
                    {
                        Position3d destination = new Position3d(Integer.parseInt(st.nextToken()), Integer.parseInt(st
                                .nextToken()), 0, 0.0f, 0.0f, (float) Math.toRadians(Float.parseFloat(st.nextToken())));

                        pilotDest.pos = destination;

                        if (Integer.parseInt(st.nextToken()) == -1)
                            pilotDest.moveDir = (float) Math.PI;
                        else
                        {
                            pilotDest.moveDir = 0.0f;
                        }
                        pilot.setDestination(pilotDest);
                    }
                }
            }
        });

        destinationButton.addKeyListener(mapComponent.keyListener);
        onButton.addKeyListener(mapComponent.keyListener);
        offButton.addKeyListener(mapComponent.keyListener);

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);
        buttonPanel.add(destinationButton);
        northPanel.add(new JLabel("pilot"), BorderLayout.NORTH);
        northPanel.add(buttonPanel, BorderLayout.CENTER);
        northPanel.add(destinationButton, BorderLayout.SOUTH);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(mapComponent, BorderLayout.CENTER);
        
        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        mapComponent.setEnabled(enabled);
        destinationButton.setEnabled(enabled);
    }
    
    protected void runStart()
    {
        mapViewGui = MapViewGui.findMapViewGui(ge);
        if(mapViewGui != null)
        {
            mapViewGui.addMapView(this);
            setDestinationCommand = this.getName() + " - set destination";
            mapViewGui.addMapViewAction(setDestinationCommand, this);
        }
    }

    protected void runStop()
    {
        if(mapViewGui != null)
        {
            mapViewGui.removeMapView(this);
            mapViewGui.removeMapViewActions(this);
        }
    }
    
    protected boolean needsRunData()
    {
        return (super.needsRunData() || mapViewIsShowing);
    }
    
    protected void runData()
    {
        PilotDataMsg data;
        
        data = pilot.getData();
        
        if (data != null)
        {
            synchronized (this)
            {
                pilotData = data;
            }
            mapComponent.repaint();
            
            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
        mapViewIsShowing = false;
    }

    public void mapViewActionPerformed(MapViewActionEvent event)
    {
        String command = event.getActionCommand();
        
        if(command.equals(setDestinationCommand))
        {
            Position3d destination = new Position3d(event.getWorldCursorPos());

            pilot.setDestination(destination);
        }
    }

    public synchronized void paintMapView(MapViewGraphics mvg)
    {
        double openAngle;
        double startAngle;
        double angleOffset;
        int	  r;
    	
        mapViewIsShowing = true;

        if (pilotData == null)
            return;
    
        Graphics2D g = mvg.getWorldGraphics();        
        g.setStroke(new BasicStroke(100.0f));
        
        // draw path
        g.setColor(Color.BLUE);
        for (int i = 0; i < pilotData.splineNum; i++)
        {
            // direct line
            if (pilotData.spline[i].radius == 0)
            {
            	g.drawLine(pilotData.spline[i].startPos.x, pilotData.spline[i].startPos.y,
            			   pilotData.spline[i].endPos.x, pilotData.spline[i].endPos.y);
            }

            // curved spline
            else
            {
                openAngle = (float)Math.abs((double)pilotData.spline[i].length /
                  						    (double)pilotData.spline[i].radius);
                
                if (pilotData.spline[i].length < 0)
                {
                	if (pilotData.spline[i].radius > 0)
                	{
                		startAngle = (float)Math.toDegrees(
                                 	 normAngle(Math.PI / 2 -
                                     pilotData.spline[i].startPos.rho));
                	}
                	else
                	{
                		startAngle = (float)Math.toDegrees(
                                 	  normAngle(
                                      -pilotData.spline[i].endPos.rho - Math.PI / 2));
                	}
                }
                else
                {
                	if (pilotData.spline[i].radius > 0)
                	{
                		startAngle = (float) Math.toDegrees(
                                 	  normAngle(Math.PI / 2 -
                                      pilotData.spline[i].startPos.rho -
                                      openAngle));
                	}
                	else
                	{
                		startAngle = (float)Math.toDegrees(
                                 	  normAngle(
                                      -pilotData.spline[i].endPos.rho -
                                      openAngle - Math.PI / 2));
                	}
                }
                r = Math.abs(pilotData.spline[i].radius);
                g.drawArc(pilotData.spline[i].centerPos.x - r,
                          pilotData.spline[i].centerPos.y - r, 2 * r, 2 * r,
                          (int) Math.round(startAngle),
                          (int) Math.round((float)Math.toDegrees(normAngle(openAngle))));
            }
        }        

        // draw current robot movement
        g = mvg.getRobotGraphics();        
        g.setStroke(new BasicStroke(100.0f));        
        g.setColor(Color.GREEN);

        float radius = 1.0f / pilotData.curve;
        if ((radius > 100000) | (radius < -100000))
        {
            radius = 0;
        }

        // curved spline
        if (radius != 0)
        {
            openAngle = Math.abs(pilotData.speed * pilotData.curve);

            if (openAngle > Math.PI)
                openAngle = Math.PI;

            if (pilotData.curve > 0)
                angleOffset = Math.PI * 0.5;
            else
                angleOffset = Math.PI * 1.5;

            // backward movement
            if (pilotData.speed < 0)
            {
                if (pilotData.curve > 0)
                    startAngle = (float)Math.toDegrees(
                                    	normAngle(Math.PI / 2 -
                                        pilotData.pos.rho));
                else
                    startAngle = (float)Math.toDegrees(
                                        normAngle(-pilotData.pos.rho -
                                        openAngle - Math.PI / 2));
            }
            // forward movement
            else
            {
                if (pilotData.curve > 0)
                {
                    startAngle = (float)Math.toDegrees(
                                        normAngle(Math.PI / 2 -
                                        pilotData.pos.rho - openAngle));
                }
                else
                {
                    startAngle = (float)Math.toDegrees(
                                        normAngle(-pilotData.pos.rho - Math.PI / 2));
                }
            }

            r = (int)Math.round(Math.abs(radius));
            g.drawArc(pilotData.pos.x + (int)(Math.cos(angleOffset +
                      (double)pilotData.pos.rho) / Math.abs(pilotData.curve)) - r,
                      pilotData.pos.y + (int)(Math.sin(angleOffset +
                      (double)pilotData.pos.rho) / Math.abs(pilotData.curve)) - r,
                       r * 2, r * 2, (int)Math.round(startAngle),
                      (int)Math.toDegrees(normAngle(openAngle)));
        }
        // direct line
        else
        {
            g.setColor(Color.GREEN);
            g.drawLine(pilotData.pos.x, pilotData.pos.y,
                       pilotData.pos.x + (int)(pilotData.speed * Math.cos(pilotData.pos.rho)),
                       pilotData.pos.y + (int)(pilotData.speed * Math.sin(pilotData.pos.rho)));
        }        
    }
    
    public static double normAngle(double x)
    {
        while (x < 0)
            x = x + 2 * Math.PI;
        while (x > 2 * Math.PI)
            x = x - 2 * Math.PI;
        return x;
    }
}
