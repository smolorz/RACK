/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Oliver Wulf <oliver.wulf@gmx.de>
 *
 */
package rack.gui.navigation;

import java.awt.*;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.*;
import rack.navigation.MclDataMsg;
import rack.navigation.MclDataPoint;
import rack.navigation.MclFilenameMsg;
import rack.navigation.MclProxy;

public class MclGui extends RackModuleGui implements MapViewInterface
{
    protected MclProxy         mcl;
    protected MapViewComponent mapComponent;
    protected MclDataMsg       mclData;

    protected Point            actualPoint      = new Point(0, 0);
    protected Point            mousePressedPoint;
    protected Point            mouseReleasedPoint;
    protected Point            mouseClickedPoint;

    protected MclFilenameMsg   mclFilename      = new MclFilenameMsg();

    protected boolean          mapViewIsShowing = false;
    protected MapViewGui       mapViewGui;

    public MclGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        mcl = (MclProxy) proxy;

        JPanel leftButtonPanel = new JPanel(new GridLayout(0, 1));
        JPanel westPanel = new JPanel(new BorderLayout());
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
        westPanel.add(leftButtonPanel, BorderLayout.NORTH);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(mapComponent, BorderLayout.CENTER);

        rootPanel.add(westPanel, BorderLayout.WEST);

        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        mapComponent.setEnabled(enabled);
    }

    @Override
	protected void runStart()
    {
        mapViewGui = MapViewGui.findMapViewGui(ge);
        if(mapViewGui != null)
        {
            mapViewGui.addMapView(this);
        }
    }

    @Override
	public void runStop()
    {
        if(mapViewGui != null)
        {
            mapViewGui.removeMapView(this);
        }
    }

    @Override
	protected boolean needsRunData()
    {
        return (super.needsRunData() || mapViewIsShowing);
    }

    @Override
	protected void runData()
    {
        MclDataMsg data;

        data = mcl.getData();

        if (data != null)
        {
            synchronized (this)
            {
                mclData = data;
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

    // MapView
    public void mapViewActionPerformed(MapViewActionEvent event)
    {
    }

    public synchronized void paintMapView(MapViewGraphics mvg)
    {
        mapViewIsShowing = true;

        if (mclData == null)
            return;

        Graphics2D g = mvg.getWorldGraphics();
        Graphics2D g2 = mvg.getWorldGraphics();

        // draw complete MCL
        for (int i = 0; i < mclData.pointNum; i++)
        {
            if (mclData.point[i].type == MclDataPoint.LINE_FEATURE)
            {
                g.setColor(Color.BLUE);
                if(mclData.point[i].layer == 1)
                {
                    g.setColor(Color.MAGENTA);
                }
                g.setStroke(new BasicStroke(100));
                g.drawLine(mclData.point[i].x, mclData.point[i].y, mclData.point[i + 1].x, mclData.point[i + 1].y);
                i++;
            }
            else if (mclData.point[i].type == MclDataPoint.SAMPLE)
            {
                g2.setColor(Color.RED);
                if(mclData.point[i].layer == 1)
                {
                    g.setColor(Color.ORANGE);
                }
                g2.fillRect(mclData.point[i].x-100, mclData.point[i].y-100,200,200);
            }
            else if (mclData.point[i].type == MclDataPoint.MEASUREMENT)
            {
                g2.setColor(Color.GREEN);
                g2.fillRect(mclData.point[i].x-250, mclData.point[i].y-250,500,500);
            }
            else if (mclData.point[i].type == MclDataPoint.REFLECTOR)
            {
                g2.setColor(Color.DARK_GRAY);
                g2.fillRect(mclData.point[i].x-250, mclData.point[i].y-250,500,500);
            }
        }
    }
}
