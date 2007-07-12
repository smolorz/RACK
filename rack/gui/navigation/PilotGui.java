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
        mapViewIsShowing = true;

        if (pilotData == null)
            return;
    
        Graphics2D g = mvg.getRobotGraphics(pilotData.recordingTime);

        g.setColor(Color.GREEN);
        g.setStroke(new BasicStroke(200.0f));
        
        if(pilotData.curve > 0.0f)
        {
            int radius = (int)(1.0f / pilotData.curve);
            
            if(pilotData.speed > 0)
            {
                g.drawArc(2*radius,0,2*radius,2*radius,0,180);
            }
            else
            {
                g.drawArc(0,0,2*radius,2*radius,0,180);
            }
        }
        else if(pilotData.curve < 0.0f)
        {
            int radius = (int)(-1.0f / pilotData.curve);

            if(pilotData.speed > 0)
            {
                g.drawArc(2*radius,-2*radius,2*radius,2*radius,0,180);
            }
            else
            {
                g.drawArc(0,-2*radius,2*radius,2*radius,0,180);
            }
        }
        else //pilotData.curve == 0.0f
        {
            g.drawLine(0,0,pilotData.speed,0);
        }
    }
}
