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

    public void mapViewActionPerformed(MapViewActionEvent action)
    {
    }

    public void paintMapView(MapViewGraphics mvg)
    {
        if(ladarData == null)
            return;
        
        Graphics2D g = mvg.getWorldGraphics();

        int x, y;
        float angle = ladarData.startAngle;
        
        for (int i = 0; i < ladarData.distanceNum; i++)
        {
            if (ladarData.distance[i] >= 0)
            {
                g.setColor(Color.RED);
            }
            else
            {
                ladarData.distance[i] = -ladarData.distance[i];
                g.setColor(Color.BLUE);
            }

            x = (int)((double)ladarData.distance[i] * Math.cos(angle));
            y = (int)((double)ladarData.distance[i] * Math.sin(angle));

            g.fillRect(x - 25, y - 25, 50, 50);
            
            angle += ladarData.angleResolution;
        }
    }
}
