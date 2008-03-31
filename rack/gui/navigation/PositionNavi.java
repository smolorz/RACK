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
 *      Oliver Wulf      <oliver.wulf@web.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.gui.navigation;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.MapViewActionEvent;
import rack.gui.main.MapViewGraphics;
import rack.gui.main.MapViewGui;
import rack.gui.main.MapViewInterface;
import rack.gui.main.NaviComponent;
import rack.gui.main.RackModuleGui;

import rack.main.defines.Position3d;
import rack.navigation.PositionDataMsg;
import rack.navigation.PositionProxy;

public class PositionNavi extends RackModuleGui implements MapViewInterface
{
    protected PositionProxy   position;
    protected Position3d      pos;

    protected NaviComponent   navi;
    protected Position3d[]    destList;
    protected int             destIndex;
    protected int             destTurnover;
    
    protected JButton         setDestinationButton = new JButton("Set Destination");
    protected ActionListener  setDestinationAction;

    protected boolean         mapViewIsShowing;

    public PositionNavi(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        position = (PositionProxy) proxy;

        JPanel buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));

        setDestinationAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                String s = (String) JOptionPane.showInputDialog(null, "Set Destination:\n" + "x, y",
                        "Navi", JOptionPane.PLAIN_MESSAGE, null, null, "0,0");
                if ((s != null) && (s.length() > 0))
                {
                    StringTokenizer st = new StringTokenizer(s, ",");
                    if (st.countTokens() == 2)
                    {
                        Position3d dest = new Position3d(
                                Integer.parseInt(st.nextToken()),
                                Integer.parseInt(st.nextToken()), 0, 0.0f, 0.0f, 0.0f);
                        
                        destList = new Position3d[1];
                        destList[0] = dest;
                        destIndex = 0;
                        navi.setDestination(dest);
                    }
                }
            }
        };
        setDestinationButton.addActionListener(setDestinationAction);

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);

        rootPanel.add(buttonPanel, BorderLayout.NORTH);
        rootPanel.add(setDestinationButton, BorderLayout.SOUTH);

        destList = new Position3d[4];
        destList[0] = new Position3d(10000, 10000, 0, 0, 0, 0);
        destList[1] = new Position3d(-7000, 18000, 0, 0, 0, 0);
        destList[2] = new Position3d(-5000, 0, 0, 0, 0, 0);
        destList[3] = new Position3d(0, 0, 0, 0, 0, 0);
        destIndex = 0;
        destTurnover = 2000;

        navi = new NaviComponent(Color.RED, true);
        navi.setDestination(destList[0]);
        rootPanel.add(navi, BorderLayout.CENTER);

        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        navi.setEnabled(enabled);
        setDestinationButton.setEnabled(enabled);
    }
    
    protected void runStart()
    {
        MapViewGui mapViewGui = MapViewGui.findMapViewGui(ge);
        if(mapViewGui != null)
        {
            mapViewGui.addMapView(this);
        }
    }

    protected void runStop()
    {
        MapViewGui mapViewGui = MapViewGui.findMapViewGui(ge);
        if(mapViewGui != null)
        {
            mapViewGui.removeMapView(this);
        }

        setDestinationButton.removeActionListener(setDestinationAction);
    }
    
    protected boolean needsRunData()
    {
        return (super.needsRunData() || mapViewIsShowing);
    }
    
    protected void runData()
    {
        PositionDataMsg data;

        data = position.getData();

        if (data != null)
        {
            synchronized(this)
            {
                pos = data.pos;
            }

            navi.setPosition(data.pos);

            if(destIndex < destList.length - 1)
            {
                double dx = destList[destIndex].x - data.pos.x;
                double dy = destList[destIndex].y - data.pos.y;
                double dist = Math.sqrt(dx * dx + dy * dy);
                if(dist < destTurnover)
                {
                    destIndex++;
                    navi.setDestination(destList[destIndex]);
                }
            }
            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
    }
    
    public synchronized void paintMapView(MapViewGraphics mvg)
    {
        Graphics2D g = mvg.getWorldGraphics();

        g.setColor(Color.RED);

        synchronized(this)
        {
            if((pos != null) & (destList != null) & (destList.length > 0))
            {
                g.drawLine(pos.x, pos.y, destList[destIndex].x, destList[destIndex].y);
                
                for(int i = destIndex + 1; i < destList.length; i++)
                {
                    g.drawLine(destList[i-1].x, destList[i-1].y, destList[i].x, destList[i].y);
                }
            }
        }
    }

    public void mapViewActionPerformed(MapViewActionEvent event)
    {
    }
}
