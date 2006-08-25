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
import java.awt.geom.*;
import java.util.*;
import javax.swing.*;

import rack.gui.main.*;
import rack.main.*;
import rack.drivers.GpsDataMsg;
import rack.drivers.GpsProxy;

import rack.main.defines.Position3D;
import rack.main.defines.Position2D;
import rack.main.tims.Tims;
import rack.navigation.PositionDataMsg;
import rack.navigation.PositionProxy;

public class PositionGui extends RackModuleGui implements ActionListener
{
    protected JButton onButton;
    protected JButton offButton;
    protected JButton manualUpdateButton = new JButton("Manual Update");
    protected JButton gpsUpdateButton = new JButton("GPS Update");
    private JCheckBox drawPosPathCheckBox;

    protected JPanel panel;

    protected JDialog updateDialog = new JDialog();

    protected JLabel xNameLabel = new JLabel("X [mm]", SwingConstants.RIGHT);
    protected JLabel yNameLabel = new JLabel("Y [mm]", SwingConstants.RIGHT);
    protected JLabel zNameLabel = new JLabel("Z [mm]", SwingConstants.RIGHT);
    protected JLabel phiNameLabel = new JLabel("Phi [deg]", SwingConstants.RIGHT);
    protected JLabel psiNameLabel = new JLabel("Psi [deg]", SwingConstants.RIGHT);
    protected JLabel rhoNameLabel = new JLabel("Rho [deg]", SwingConstants.RIGHT);

    protected JLabel xLabel = new JLabel("-0000000");
    protected JLabel yLabel = new JLabel("-0000000");
    protected JLabel zLabel = new JLabel("-0000000");
    protected JLabel phiLabel = new JLabel("-000.00");
    protected JLabel psiLabel = new JLabel("-000.00");
    protected JLabel rhoLabel = new JLabel("-000.00");

    protected PositionProxy position;
    protected RackProxy[] rackProxyList;
    private MapViewActionList mapViewActionList;
    private GeneralPath positionPath;
    private boolean drawPositionPath = false;

    public PositionGui(Integer moduleIndex, RackProxy[] proxyList, RackModuleGui[] guiList, Tims tims)
    {
        position = (PositionProxy)proxyList[moduleIndex.intValue()];
        rackProxyList = proxyList;

        panel = new JPanel(new BorderLayout(2, 2));
        panel.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));
        JPanel southPanel = new JPanel(new GridLayout(0, 1, 4, 2));

        JPanel buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));
        JPanel labelPanel = new JPanel(new GridLayout(0, 2, 8, 0));

        onButton = new JButton("On");
        offButton = new JButton("Off");

        onButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                position.on();
            }
        });

        offButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                position.off();
            }
        });

        manualUpdateButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                String s = (String) JOptionPane.showInputDialog(null,
                        "The robot position is:\n" + "x,y,rho",
                        "Estimated robot position", JOptionPane.PLAIN_MESSAGE,
                        null, null, "0,0,0.0");
                if ((s != null) && (s.length() > 0))
                {
                    StringTokenizer st = new StringTokenizer(s, ",");
                    if (st.countTokens() == 3)
                    {
                        Position3D robotPosition = new Position3D(
                            Integer.parseInt(st.nextToken()),
                            Integer.parseInt(st.nextToken()),
                            0,
                            0.0f,
                            0.0f,
                            (float) Math.toRadians(Float.parseFloat(st.nextToken())));
                        position.update(robotPosition, 0);
                    }
                }
            }
        });

        gpsUpdateButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                for(int i = 0; i < rackProxyList.length; i++)
                {
                    if(rackProxyList[i].getCommandMbx() == RackName.create(RackName.GPS, 0))
                    {
                        GpsProxy gps = (GpsProxy)rackProxyList[i];
                        GpsDataMsg gpsData = gps.getData();
                        if(gpsData != null)
                        {
                            position.update(gpsData.posGK, 0);
                        }
                        else
                        {
                            System.out.println("Can't read data from GPS(0)");
                        }
                    }
                }
            }
        });

        drawPosPathCheckBox = new JCheckBox("Draw path", drawPositionPath);
        drawPosPathCheckBox.setActionCommand("drawPath");
        drawPosPathCheckBox.addActionListener(this);

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

        southPanel.add(drawPosPathCheckBox);
        southPanel.add(manualUpdateButton);
        southPanel.add(gpsUpdateButton);

        panel.add(northPanel, BorderLayout.NORTH);
        panel.add(labelPanel, BorderLayout.CENTER);
        panel.add(southPanel, BorderLayout.SOUTH);

        positionPath = new GeneralPath();
        positionPath.moveTo(0, 0);

        mapViewActionList = new MapViewActionList(getModuleName());
        mapViewActionList.addItem("set position", "manualUpdate");
    }

    public JComponent getComponent()
    {
        return panel;
    }

    public String getModuleName()
    {
        return "Position";
    }

    public RackProxy getProxy()
    {
        return position;
    }

    public boolean hasMapView()
    {
        return true;
    }

    public void paintMapView(MapViewDrawContext drawContext)
    {
        Graphics2D robotGraphics = drawContext.getRobotGraphics();

        if (drawPositionPath)
        {
            Graphics2D worldGraphics = drawContext.getWorldGraphics();
            worldGraphics.setColor(robotGraphics.getColor());
            worldGraphics.draw(positionPath);
        }
    }

    public MapViewActionList getMapViewActionList()
    {
        return mapViewActionList;
    }

    public void mapViewActionPerformed(MapViewActionEvent event)
    {
        if (event.getActionCommand().equals("manualUpdate"))
        {
            Position2D newPosition2D = event.getWorldCursorPos();
            Position3D newPosition3D;

            newPosition3D = new Position3D(newPosition2D.x,
                        newPosition2D.y, 0, 0, 0, newPosition2D.phi);
            position.update(newPosition3D, 0);
        }
    }

    public void actionPerformed(ActionEvent event)
    {
        if (event.getActionCommand().equals("drawPath"))
        {
            drawPositionPath = drawPosPathCheckBox.isSelected();
            if (!drawPositionPath)
            {
                positionPath.reset();
                positionPath.moveTo(0, 0);
            }
        }
    }

    public void run()
    {
        PositionDataMsg data;
        while (terminate == false)
        {
            if (panel.isShowing())
            {
                data = position.getData();

                if (data != null)
                {
                    xLabel.setText(data.pos.x + "");
                    yLabel.setText(data.pos.y + "");
                    zLabel.setText(data.pos.z + "");
                    phiLabel.setText(
                        Math.rint(Math.toDegrees(data.pos.phi)) + "");
                    psiLabel.setText(
                        Math.rint(Math.toDegrees(data.pos.psi)) + "");
                    rhoLabel.setText(
                        Math.rint(Math.toDegrees(data.pos.rho)) + "");

                    xNameLabel.setEnabled(true);
                    yNameLabel.setEnabled(true);
                    zNameLabel.setEnabled(true);
                    phiNameLabel.setEnabled(true);
                    psiNameLabel.setEnabled(true);
                    rhoNameLabel.setEnabled(true);

                    xLabel.setEnabled(true);
                    yLabel.setEnabled(true);
                    zLabel.setEnabled(true);
                    phiLabel.setEnabled(true);
                    psiLabel.setEnabled(true);
                    rhoLabel.setEnabled(true);

                    drawPosPathCheckBox.setEnabled(true);
                    manualUpdateButton.setEnabled(true);
                    gpsUpdateButton.setEnabled(true);

                    if (drawPositionPath)
                    {
                        positionPath.lineTo(data.pos.x, data.pos.y);
                    }
                    else
                    {
                        positionPath.moveTo(data.pos.x, data.pos.y);
                    }
                }
                else
                {
                    xNameLabel.setEnabled(false);
                    yNameLabel.setEnabled(false);
                    zNameLabel.setEnabled(false);
                    phiNameLabel.setEnabled(false);
                    psiNameLabel.setEnabled(false);
                    rhoNameLabel.setEnabled(false);

                    xLabel.setEnabled(false);
                    yLabel.setEnabled(false);
                    zLabel.setEnabled(false);
                    phiLabel.setEnabled(false);
                    psiLabel.setEnabled(false);
                    rhoLabel.setEnabled(false);

                    drawPosPathCheckBox.setEnabled(false);
                    manualUpdateButton.setEnabled(false);
                    gpsUpdateButton.setEnabled(false);
                }
            }

            try
            {
                Thread.sleep(100);
            }
            catch (InterruptedException e)
            {
            }
        }
    }
}
