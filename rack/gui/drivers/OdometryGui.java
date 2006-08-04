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

import rack.drivers.OdometryDataMsg;
import rack.drivers.OdometryProxy;
import rack.gui.main.RackModuleGui;
import rack.main.proxy.*;

public class OdometryGui extends RackModuleGui
{
    protected boolean terminate = false;
    protected JButton onButton;
    protected JButton offButton;
    protected JButton resetButton = new JButton("Data Reset");
    protected JPanel panel;
    protected JPanel buttonPanel;
    protected JPanel labelPanel;

    protected JLabel xNameLabel = new JLabel("X [mm]", SwingConstants.RIGHT);
    protected JLabel yNameLabel = new JLabel("Y [mm]", SwingConstants.RIGHT);
    protected JLabel zNameLabel = new JLabel("Z [mm]", SwingConstants.RIGHT);
    protected JLabel phiNameLabel = new JLabel("Phi", SwingConstants.RIGHT);
    protected JLabel psiNameLabel = new JLabel("Psi", SwingConstants.RIGHT);
    protected JLabel rhoNameLabel = new JLabel("Rho", SwingConstants.RIGHT);

    protected JLabel xLabel = new JLabel("-0000000");
    protected JLabel yLabel = new JLabel("-0000000");
    protected JLabel zLabel = new JLabel("-0000000");
    protected JLabel phiLabel = new JLabel("-000.00 ");
    protected JLabel psiLabel = new JLabel("-000.00 ");
    protected JLabel rhoLabel = new JLabel("-000.00 ");

    protected OdometryProxy odometry;

    public OdometryGui(OdometryProxy proxy)
    {
        odometry = proxy;

        panel = new JPanel(new BorderLayout(2, 2));
        panel.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));

        buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));

        labelPanel = new JPanel(new GridLayout(0, 2, 8, 0));

        onButton = new JButton("On");
        offButton = new JButton("Off");

        onButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                odometry.on();
            }
        });

        offButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                odometry.off();
            }
        });

        resetButton.addActionListener(new ActionListener()
        {
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

        panel.add(northPanel, BorderLayout.NORTH);
        panel.add(labelPanel, BorderLayout.CENTER);
        panel.add(resetButton, BorderLayout.SOUTH);
    }

    public JComponent getComponent()
    {
        return (panel);
    }

    public String getModuleName()
    {
        return ("Odometry");
    }

    public RackProxy getProxy()
    {
        return (odometry);
    }

    public void run()
    {
        OdometryDataMsg data;

        while (terminate == false)
        {
            if (panel.isShowing())
            {
                data = odometry.getData();

                if (data != null)
                {
                    xLabel.setText(data.position.x + "");
                    yLabel.setText(data.position.y + "");
                    zLabel.setText(data.position.z + "");
                    phiLabel.setText(Math.rint(Math
                            .toDegrees(data.position.phi))
                            + " ");
                    psiLabel.setText(Math.rint(Math
                            .toDegrees(data.position.psi))
                            + " ");
                    rhoLabel.setText(Math.rint(Math
                            .toDegrees(data.position.rho))
                            + " ");

                    resetButton.setEnabled(true);

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

                }
                else
                {
                    resetButton.setEnabled(false);

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
                }
            }

            try
            {
                Thread.sleep(1000);
            }
            catch (InterruptedException e)
            {
            }
        }
    }

    public void terminate()
    {
        terminate = true;
    }

}
