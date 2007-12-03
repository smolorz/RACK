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
import java.awt.geom.*;
import java.util.*;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.*;
import rack.main.*;
import rack.drivers.GpsDataMsg;
import rack.drivers.GpsProxy;

import rack.main.defines.Position3d;
import rack.navigation.PositionDataMsg;
import rack.navigation.PositionProxy;

public class PositionGui extends RackModuleGui implements MapViewInterface
{
    protected JButton         manualUpdateButton = new JButton("Manual Update");
    protected JButton         gpsUpdateButton[];

    protected JDialog         updateDialog       = new JDialog();

    protected JLabel          xNameLabel         = new JLabel("X [mm]", SwingConstants.RIGHT);
    protected JLabel          yNameLabel         = new JLabel("Y [mm]", SwingConstants.RIGHT);
    protected JLabel          zNameLabel         = new JLabel("Z [mm]", SwingConstants.RIGHT);
    protected JLabel          phiNameLabel       = new JLabel("Phi [deg]", SwingConstants.RIGHT);
    protected JLabel          psiNameLabel       = new JLabel("Psi [deg]", SwingConstants.RIGHT);
    protected JLabel          rhoNameLabel       = new JLabel("Rho [deg]", SwingConstants.RIGHT);

    protected JLabel          xLabel             = new JLabel("-0000000");
    protected JLabel          yLabel             = new JLabel("-0000000");
    protected JLabel          zLabel             = new JLabel("-0000000");
    protected JLabel          phiLabel           = new JLabel("-000.00");
    protected JLabel          psiLabel           = new JLabel("-000.00");
    protected JLabel          rhoLabel           = new JLabel("-000.00");

    protected JLabel          varXNameLabel      = new JLabel("VarX [mm]", SwingConstants.RIGHT);
    protected JLabel          varYNameLabel      = new JLabel("VarY [mm]", SwingConstants.RIGHT);
    protected JLabel          varRhoNameLabel    = new JLabel("VarRho [deg]", SwingConstants.RIGHT);

    protected JLabel          varXLabel          = new JLabel("-0000000");
    protected JLabel          varYLabel          = new JLabel("-0000000");
    protected JLabel          varRhoLabel        = new JLabel("-000.00");

    protected PositionProxy   position;
    protected GpsProxy[]      gpsProxy;
    protected String[]        gpsName;

    protected GeneralPath     positionPath;

    protected boolean         mapViewIsShowing;
    protected MapViewGui      mapViewGui;
    
    public PositionGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        position = (PositionProxy) proxy;

        int i;
        for(i = 0; i < 4; i++)
        {
            if(mainGui.getGuiElement(RackName.GPS, i) == null)
            {
                break;
            }
        }
        gpsProxy = new GpsProxy[i];
        gpsName = new String[i];
        for(i = 0; i < gpsProxy.length; i++)
        {
            GuiElementDescriptor ge = mainGui.getGuiElement(RackName.GPS, i);
            gpsProxy[i] = (GpsProxy)ge.getProxy();
            gpsName[i] = ge.getName();
        }

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));
        JPanel southPanel = new JPanel(new GridLayout(0, 1, 4, 2));

        JPanel buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));
        JPanel labelPanel = new JPanel(new GridLayout(0, 2, 8, 0));

        manualUpdateButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                String s = (String) JOptionPane.showInputDialog(null, "The robot position is:\n" + "x,y,rho",
                        "Estimated robot position", JOptionPane.PLAIN_MESSAGE, null, null, "0,0,0.0");
                if ((s != null) && (s.length() > 0))
                {
                    StringTokenizer st = new StringTokenizer(s, ",");
                    if (st.countTokens() == 3)
                    {
                        Position3d robotPosition = new Position3d(Integer.parseInt(st.nextToken()), Integer.parseInt(st
                                .nextToken()), 0, 0.0f, 0.0f, (float) Math.toRadians(Float.parseFloat(st.nextToken())));
                        position.update(new PositionDataMsg(robotPosition));
                    }
                }
            }
        });

        ActionListener gpsActionListener = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                int gps = Integer.parseInt(e.getActionCommand());
                
                GpsDataMsg gpsData = gpsProxy[gps].getData();
                if (gpsData != null)
                {
                    position.update(new PositionDataMsg(gpsData.pos, gpsData.var));
                }
                else
                {
                    System.out.println("Can't read data from GPS");
                }
            }
        };

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);

        northPanel.add(new JLabel("position"), BorderLayout.CENTER);
        northPanel.add(buttonPanel, BorderLayout.SOUTH);

        labelPanel.add(xNameLabel);
        labelPanel.add(xLabel);
        labelPanel.add(yNameLabel);
        labelPanel.add(yLabel);
        labelPanel.add(zNameLabel);
        labelPanel.add(zLabel);
        labelPanel.add(phiNameLabel);
        labelPanel.add(phiLabel);
        labelPanel.add(psiNameLabel);
        labelPanel.add(psiLabel);
        labelPanel.add(rhoNameLabel);
        labelPanel.add(rhoLabel);

        labelPanel.add(varXNameLabel);
        labelPanel.add(varXLabel);
        labelPanel.add(varYNameLabel);
        labelPanel.add(varYLabel);
        labelPanel.add(varRhoNameLabel);
        labelPanel.add(varRhoLabel);

        southPanel.add(manualUpdateButton);

        gpsUpdateButton = new JButton[gpsProxy.length];
        for(i = 0; i < gpsProxy.length; i++)
        {
            gpsUpdateButton[i] = new JButton(gpsName[i] + " Update");
            gpsUpdateButton[i].setActionCommand(Integer.toString(i));
            gpsUpdateButton[i].addActionListener(gpsActionListener);
            southPanel.add(gpsUpdateButton[i]);
        }

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(labelPanel, BorderLayout.CENTER);
        rootPanel.add(southPanel, BorderLayout.SOUTH);

        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        xNameLabel.setEnabled(enabled);
        yNameLabel.setEnabled(enabled);
        zNameLabel.setEnabled(enabled);
        phiNameLabel.setEnabled(enabled);
        psiNameLabel.setEnabled(enabled);
        rhoNameLabel.setEnabled(enabled);

        xLabel.setEnabled(enabled);
        yLabel.setEnabled(enabled);
        zLabel.setEnabled(enabled);
        phiLabel.setEnabled(enabled);
        psiLabel.setEnabled(enabled);
        rhoLabel.setEnabled(enabled);

        varXNameLabel.setEnabled(enabled);
        varYNameLabel.setEnabled(enabled);
        varRhoNameLabel.setEnabled(enabled);
        varXLabel.setEnabled(enabled);
        varYLabel.setEnabled(enabled);
        varRhoLabel.setEnabled(enabled);

        manualUpdateButton.setEnabled(enabled);
        
        for(int i = 0; i < gpsProxy.length; i++)
        {
            gpsUpdateButton[i].setEnabled(enabled);
        }
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
        PositionDataMsg data;

        data = position.getData();

        if (data != null)
        {
            xLabel.setText(data.pos.x + "");
            yLabel.setText(data.pos.y + "");
            zLabel.setText(data.pos.z + "");
            phiLabel.setText(Math.rint(Math.toDegrees(data.pos.phi)) + "");
            psiLabel.setText(Math.rint(Math.toDegrees(data.pos.psi)) + "");
            rhoLabel.setText(Math.rint(Math.toDegrees(data.pos.rho)) + "");

            varXLabel.setText(data.var.x + "");
            varYLabel.setText(data.var.y + "");
            varRhoLabel.setText(Math.rint(Math.toDegrees(data.var.rho)) + "");

            if(positionPath != null)
            {
                positionPath.lineTo(data.pos.x, data.pos.y);
            }
            else
            {
                positionPath = new GeneralPath();
                positionPath.moveTo(data.pos.x, data.pos.y);
            }

            setEnabled(true);

        }
        else
        {
            setEnabled(false);
        }
    }
    
    public void paintMapView(MapViewGraphics mvg)
    {
        if(positionPath == null)
            return;
        
        Graphics2D g = mvg.getWorldGraphics();

        switch (position.getInstanceId())
        {
        case 1:
            g.setColor(Color.BLUE);
            break;
        case 2:
            g.setColor(Color.RED);
            break;
        case 3:
            g.setColor(Color.GREEN);
            break;

        default:
            g.setColor(Color.BLACK);
        }

        g.draw(positionPath);
    }

    public void mapViewActionPerformed(MapViewActionEvent event)
    {
    }
}
