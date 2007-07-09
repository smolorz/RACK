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
import java.awt.event.*;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.RackModuleGui;
import rack.drivers.OdometryDataMsg;
import rack.drivers.OdometryProxy;

public class OdometryGui extends RackModuleGui
{
    protected JButton       resetButton  = new JButton("Data Reset");

    protected JLabel        xNameLabel   = new JLabel("X [mm]", SwingConstants.RIGHT);
    protected JLabel        yNameLabel   = new JLabel("Y [mm]", SwingConstants.RIGHT);
    protected JLabel        zNameLabel   = new JLabel("Z [mm]", SwingConstants.RIGHT);
    protected JLabel        phiNameLabel = new JLabel("Phi", SwingConstants.RIGHT);
    protected JLabel        psiNameLabel = new JLabel("Psi", SwingConstants.RIGHT);
    protected JLabel        rhoNameLabel = new JLabel("Rho", SwingConstants.RIGHT);

    protected JLabel        xLabel       = new JLabel("-0000000");
    protected JLabel        yLabel       = new JLabel("-0000000");
    protected JLabel        zLabel       = new JLabel("-0000000");
    protected JLabel        phiLabel     = new JLabel("-000.00 ");
    protected JLabel        psiLabel     = new JLabel("-000.00 ");
    protected JLabel        rhoLabel     = new JLabel("-000.00 ");

    protected OdometryProxy odometry;

    public OdometryGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        odometry = (OdometryProxy) proxy;

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));
        JPanel buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));
        JPanel labelPanel = new JPanel(new GridLayout(0, 2, 8, 0));

        resetButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                odometry.reset();
            }
        });

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);

        northPanel.add(new JLabel("odometry"), BorderLayout.CENTER);
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

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(labelPanel, BorderLayout.CENTER);
        rootPanel.add(resetButton, BorderLayout.SOUTH);
        
        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        resetButton.setEnabled(enabled);

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
    }
    
    protected void runData()
    {
        OdometryDataMsg data;

        data = odometry.getData();

        if (data != null)
        {
            xLabel.setText(data.position.x + "");
            yLabel.setText(data.position.y + "");
            zLabel.setText(data.position.z + "");
            phiLabel.setText(Math.rint(Math.toDegrees(data.position.phi)) + " ");
            psiLabel.setText(Math.rint(Math.toDegrees(data.position.psi)) + " ");
            rhoLabel.setText(Math.rint(Math.toDegrees(data.position.rho)) + " ");

            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
    }
}
