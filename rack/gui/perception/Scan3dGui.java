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
 *      Oliver Wulf  <oliver.wulf@gmx.de>
 *
 */
package rack.gui.perception;

import java.awt.*;
import java.awt.event.*;

import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.RackModuleGui;
import rack.perception.Scan3dProxy;
import rack.perception.Scan3dRangeImageMsg;

public class Scan3dGui extends RackModuleGui
{
    protected Scan3dProxy     scan3d;
    protected Scan3dComponent scan3dComponent;
    protected JButton         storeButton;
    protected JTextField      distanceField;
    protected JLabel          distanceLabel;
    protected JButton          storeContOnButton;
    protected JButton          storeContOffButton;
    protected boolean          contStoring = false;

    protected ActionListener   storeContOnAction;
    protected ActionListener   storeContOffAction;

    public Scan3dGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        scan3d = (Scan3dProxy) proxy;

        rootPanel = new JPanel(new BorderLayout());

        JPanel topPanel = new JPanel(new BorderLayout(4, 4));
        topPanel.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));

        JPanel buttonPanel = new JPanel(new GridLayout(1, 0, 4, 4));
        storeButton = new JButton("Store");
        distanceField = new JTextField("10", 4);
        distanceLabel = new JLabel("Max Distance (m)", SwingConstants.RIGHT);

        scan3dComponent = new Scan3dComponent(10000); // max distance 10m
        rootPanel.add(scan3dComponent, BorderLayout.CENTER);

        storeButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                scan3d.storeDataToFile("scan.3d");
            }
        });

        storeContOnButton = new JButton("StoreOn");
        storeContOffButton = new JButton("StoreOff");

        storeContOnAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                contStoring = true;
            }
        };
        storeContOnButton.addActionListener(storeContOnAction);

        storeContOffAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                contStoring = false;
            }
        };
        storeContOffButton.addActionListener(storeContOffAction);


        distanceField.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                try
                {
                    scan3dComponent.setMaxDistance(Integer.parseInt(distanceField.getText()) * 1000);
                }
                catch (NumberFormatException nfe)
                {
                    scan3dComponent.setMaxDistance(10000);
                    distanceField.setText("10");
                }
            }
        });

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);
        buttonPanel.add(storeButton);
        buttonPanel.add(storeContOnButton);
        buttonPanel.add(storeContOffButton);
        topPanel.add(buttonPanel, BorderLayout.WEST);
        topPanel.add(distanceLabel, BorderLayout.CENTER);
        topPanel.add(distanceField, BorderLayout.EAST);
        rootPanel.add(topPanel, BorderLayout.NORTH);

        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        storeButton.setEnabled(enabled);
        distanceField.setEnabled(enabled);
        distanceLabel.setEnabled(enabled);
        if(contStoring)
        {
            storeContOnButton.setEnabled(false);
            storeContOffButton.setEnabled(true);
        }
        else
        {
            storeContOnButton.setEnabled(true);
            storeContOffButton.setEnabled(false);
        }

    }

    protected void runData()
    {
        Scan3dRangeImageMsg data;

        data = scan3d.getRangeImage();

        if (data != null)
        {
            scan3dComponent.updateData(data);

            if(contStoring)
            {
                scan3d.storeDataToFile("scan3d-" + System.currentTimeMillis() + ".3d");
            }
            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
    }
}
