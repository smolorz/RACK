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
package rack.gui.drivers;

import java.awt.*;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.MapViewActionEvent;
import rack.gui.main.MapViewGraphics;
import rack.gui.main.MapViewGui;
import rack.gui.main.MapViewInterface;
import rack.gui.main.RackModuleGui;
import rack.drivers.GpsDataMsg;
import rack.drivers.GpsProxy;

public class GpsGui extends RackModuleGui implements MapViewInterface
{
    protected GpsDataMsg gpsData;
    protected GpsProxy   gps;

    protected JLabel     modeLabel        = new JLabel();
    protected JLabel     modeNameLabel    = new JLabel("Mode", SwingConstants.RIGHT);
    protected JLabel     latLabel         = new JLabel();
    protected JLabel     latNameLabel     = new JLabel("Latitude", SwingConstants.RIGHT);
    protected JLabel     longLabel        = new JLabel();
    protected JLabel     longNameLabel    = new JLabel("Longitude", SwingConstants.RIGHT);
    protected JLabel     altLabel         = new JLabel();
    protected JLabel     altNameLabel     = new JLabel("Altitude", SwingConstants.RIGHT);
    protected JLabel     headLabel        = new JLabel();
    protected JLabel     headNameLabel    = new JLabel("Heading", SwingConstants.RIGHT);
    protected JLabel     speedLabel       = new JLabel();
    protected JLabel     speedNameLabel   = new JLabel("Speed", SwingConstants.RIGHT);
    protected JLabel     satNumLabel      = new JLabel();
    protected JLabel     satNumNameLabel  = new JLabel("Satellites", SwingConstants.RIGHT);
    protected JLabel     pdopLabel        = new JLabel();
    protected JLabel     pdopNameLabel    = new JLabel("PDOP", SwingConstants.RIGHT);
    protected JLabel     utcTimeLabel     = new JLabel();
    protected JLabel     utcTimeNameLabel = new JLabel("UTC Time", SwingConstants.RIGHT);
    protected JLabel     xGKLabel         = new JLabel();
    protected JLabel     xGKNameLabel     = new JLabel("X", SwingConstants.RIGHT);
    protected JLabel     yGKLabel         = new JLabel();
    protected JLabel     yGKNameLabel     = new JLabel("Y", SwingConstants.RIGHT);
    protected JLabel     zGKLabel         = new JLabel();
    protected JLabel     zGKNameLabel     = new JLabel("Z", SwingConstants.RIGHT);
    protected JLabel     rhoGKLabel       = new JLabel();
    protected JLabel     rhoGKNameLabel   = new JLabel("Rho", SwingConstants.RIGHT);
    protected JLabel     varXYLabel       = new JLabel();
    protected JLabel     varXYNameLabel   = new JLabel("Variance XY", SwingConstants.RIGHT);
    protected JLabel     varZLabel        = new JLabel();
    protected JLabel     varZNameLabel    = new JLabel("Variance Z", SwingConstants.RIGHT);
    protected JLabel     varRhoLabel      = new JLabel();
    protected JLabel     varRhoNameLabel  = new JLabel("Variance Rho", SwingConstants.RIGHT);

    protected boolean    mapViewIsShowing;
    protected MapViewGui mapViewGui;

    public GpsGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        gps = (GpsProxy) proxy;

        JPanel buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));
        JPanel labelPanel = new JPanel(new GridLayout(0, 2, 8, 0));
        JPanel northPanel = new JPanel(new BorderLayout(2, 2));

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);
        northPanel.add(new JLabel("gps"), BorderLayout.NORTH);
        northPanel.add(buttonPanel, BorderLayout.CENTER);

        labelPanel.add(modeNameLabel);
        labelPanel.add(modeLabel);
        labelPanel.add(latNameLabel);
        labelPanel.add(latLabel);
        labelPanel.add(longNameLabel);
        labelPanel.add(longLabel);
        labelPanel.add(altNameLabel);
        labelPanel.add(altLabel);
        labelPanel.add(headNameLabel);
        labelPanel.add(headLabel);
        labelPanel.add(speedNameLabel);
        labelPanel.add(speedLabel);
        labelPanel.add(satNumNameLabel);
        labelPanel.add(satNumLabel);
        labelPanel.add(pdopNameLabel);
        labelPanel.add(pdopLabel);
        labelPanel.add(utcTimeNameLabel);
        labelPanel.add(utcTimeLabel);
        labelPanel.add(xGKNameLabel);
        labelPanel.add(xGKLabel);
        labelPanel.add(yGKNameLabel);
        labelPanel.add(yGKLabel);
        labelPanel.add(zGKNameLabel);
        labelPanel.add(zGKLabel);
        labelPanel.add(rhoGKNameLabel);
        labelPanel.add(rhoGKLabel);
        labelPanel.add(varXYNameLabel);
        labelPanel.add(varXYLabel);
        labelPanel.add(varZNameLabel);
        labelPanel.add(varZLabel);
        labelPanel.add(varRhoNameLabel);
        labelPanel.add(varRhoLabel);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(labelPanel, BorderLayout.CENTER);
        
        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        modeNameLabel.setEnabled(enabled);
        modeLabel.setEnabled(enabled);
        latNameLabel.setEnabled(enabled);
        latLabel.setEnabled(enabled);
        longNameLabel.setEnabled(enabled);
        longLabel.setEnabled(enabled);
        altNameLabel.setEnabled(enabled);
        altLabel.setEnabled(enabled);
        headNameLabel.setEnabled(enabled);
        headLabel.setEnabled(enabled);
        speedNameLabel.setEnabled(enabled);
        speedLabel.setEnabled(enabled);
        satNumNameLabel.setEnabled(enabled);
        satNumLabel.setEnabled(enabled);
        pdopNameLabel.setEnabled(enabled);
        pdopLabel.setEnabled(enabled);
        utcTimeNameLabel.setEnabled(enabled);
        utcTimeLabel.setEnabled(enabled);
        xGKNameLabel.setEnabled(enabled);
        xGKLabel.setEnabled(enabled);
        yGKNameLabel.setEnabled(enabled);
        yGKLabel.setEnabled(enabled);
        zGKNameLabel.setEnabled(enabled);
        zGKLabel.setEnabled(enabled);
        rhoGKNameLabel.setEnabled(enabled);
        rhoGKLabel.setEnabled(enabled);
        varXYNameLabel.setEnabled(enabled);
        varXYLabel.setEnabled(enabled);
        varZNameLabel.setEnabled(enabled);
        varZLabel.setEnabled(enabled);
        varRhoNameLabel.setEnabled(enabled);
        varRhoLabel.setEnabled(enabled);
    }

    protected void runStart()
    {
        mapViewGui = MapViewGui.findMapViewGui(ge);
        if(mapViewGui != null)
        {
            mapViewGui.addMapView(this);
        }
    }

    protected void runStop()
    {
        if(mapViewGui != null)
        {
            mapViewGui.removeMapView(this);
        }
    }
    
    protected boolean needsRunData()
    {
        return (super.needsRunData() || mapViewIsShowing);
    }
    
    protected void runData()
    {
        GpsDataMsg data;

        float latitude;
        float longitude;
        float heading;
        float altitude;
        float speed;

        data = gps.getData();

        if (data != null)
        {
            synchronized(this)
            {
                gpsData = data;
            }
            
            latitude = (float) Math.toDegrees(data.latitude);
            longitude = (float) Math.toDegrees(data.longitude);
            heading = (float) Math.toDegrees(data.heading);
            altitude = (float) data.altitude / 1000;
            speed = (float) data.speed / 1000;

            if (data.mode == GpsDataMsg.MODE_2D)
            {
                modeLabel.setText("2D");
            }
            if (data.mode == GpsDataMsg.MODE_3D)
            {
                modeLabel.setText("3D");
            }
            else
            {
                modeLabel.setText("invalid");
            }

            if (latitude >= 0.0)
            {
                latLabel.setText(latitude + " deg N");
            }
            else
            {
                latLabel.setText(latitude + " deg S");
            }

            if (longitude >= 0.0)
            {
                longLabel.setText(longitude + " deg E");
            }
            else
            {
                longLabel.setText(longitude + " deg W");
            }

            altLabel.setText(altitude + " m");
            headLabel.setText(heading + " deg");
            speedLabel.setText(speed + " m/s");

            satNumLabel.setText(data.satelliteNum + "");
            pdopLabel.setText(data.pdop + "");
            utcTimeLabel.setText(data.utcTime + " s");
            xGKLabel.setText(data.posGK.x + " mm");
            yGKLabel.setText(data.posGK.y + " mm");
            zGKLabel.setText(data.posGK.z + " mm");
            rhoGKLabel.setText((float) Math.toDegrees(data.posGK.rho) + " deg");
            varXYLabel.setText(data.varXY + " mm");
            varZLabel.setText(data.varZ + " mm");
            varRhoLabel.setText((float) Math.toDegrees(data.varRho) + " deg");

            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
        mapViewIsShowing = false;
    }

    public void paintMapView(MapViewGraphics mvg)
    {
        mapViewIsShowing = true;

        if (gpsData == null)
            return;

        Graphics2D worldGraphics = mvg.getWorldGraphics();

        worldGraphics.setColor(Color.ORANGE);
        worldGraphics.drawArc(gpsData.posGK.x - 10000, gpsData.posGK.y - 10000, 20000, 20000, 0, 360);
        worldGraphics.drawLine(gpsData.posGK.x, gpsData.posGK.y, gpsData.posGK.x
                + (int) (10000 * Math.cos(gpsData.posGK.rho)), gpsData.posGK.y
                + (int) (10000 * Math.sin(gpsData.posGK.rho)));
    }

    public void mapViewActionPerformed(MapViewActionEvent action)
    {
    }
}
