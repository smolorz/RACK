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
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.MapViewActionEvent;
import rack.gui.main.MapViewComponent;
import rack.gui.main.MapViewGraphics;
import rack.gui.main.MapViewInterface;
import rack.gui.main.RackModuleGui;
import rack.main.defines.LadarPoint;
import rack.drivers.LadarDataMsg;
import rack.drivers.LadarProxy;

public class LadarGui extends RackModuleGui implements MapViewInterface
{
    protected LadarDataMsg     ladarData;
    protected LadarProxy       ladar;
    protected MapViewComponent mapComponent;

    public LadarGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);
        
        ladar = (LadarProxy) proxy;

        updateTime = 200;

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));
        JPanel wButtonPanel = new JPanel(new GridLayout(1, 0, 4, 2));

        mapComponent = new MapViewComponent();
        mapComponent.addMapView(this);

        onButton.addKeyListener(mapComponent.keyListener);
        offButton.addKeyListener(mapComponent.keyListener);

        wButtonPanel.add(onButton);
        wButtonPanel.add(offButton);

        northPanel.add(wButtonPanel, BorderLayout.WEST);
        northPanel.add(mapComponent.zoomPanel, BorderLayout.EAST);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(mapComponent, BorderLayout.CENTER);
        
        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        mapComponent.setEnabled(enabled);
    }
    
    protected void runData()
    {
        LadarDataMsg data = null;

        data = ladar.getData();

        if (data != null)
        {
            synchronized(this)
            {
                ladarData = data;
            }
            mapComponent.repaint();
            
            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
    }
    
    protected void runStop()
    {
        mapComponent.removeListener();
        mapComponent = null;
        
        synchronized(this)
        {
            ladarData = null;
        }
    }

    public void mapViewActionPerformed(MapViewActionEvent action)
    {
    }

    public synchronized void paintMapView(MapViewGraphics mvg)
    {
        if(ladarData == null)
            return;
        
        Graphics2D g = mvg.getWorldGraphics();

        int x, y;
        
        for (int i = 0; i < ladarData.pointNum; i++)
        {
            
        	if (ladarData.point[i].type == LadarPoint.TYPE_TRANSPARENT)
        	{
        		g.setColor(Color.DARK_GRAY);
        	}
        	else if (ladarData.point[i].type == LadarPoint.TYPE_RAIN)
        	{
        		g.setColor(Color.MAGENTA);
        	}
        	else if (ladarData.point[i].type == LadarPoint.TYPE_DIRT)
        	{
        		g.setColor(Color.CYAN);
        	}
        	else if (ladarData.point[i].type == LadarPoint.TYPE_INVALID)
        	{
        		g.setColor(Color.GRAY);
        	}
        	else if (ladarData.point[i].type == LadarPoint.TYPE_REFLECTOR)
        	{
        		g.setColor(Color.YELLOW);
        	}        	
            else
            {
                g.setColor(Color.RED);
            }
        	
            x = (int)((double)ladarData.point[i].distance * Math.cos(ladarData.point[i].angle));
            y = (int)((double)ladarData.point[i].distance * Math.sin(ladarData.point[i].angle));

            g.fillRect(x - 25, y - 25, 50, 50);
        }
    }
}
