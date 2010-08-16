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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
package rack.gui.navigation;

import java.awt.*;
import java.awt.event.*;

import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.RackModuleGui;
import rack.navigation.GridMapDataMsg;
import rack.navigation.GridMapProxy;

public class GridMapGui extends RackModuleGui
{
    protected JButton          storeButton;
    protected JButton          storeContOnButton;
    protected JButton          storeContOffButton;

    protected GridMapComponent gridMapComponent;

    protected JLabel           contStoringLabel;
    protected JComboBox        zoomRateComboBox;
    protected String[]         possibleZoomRates = { "25", "50", "100", "200", "300", "400" };
    protected int              zoomRate;
    protected int              contStoring       = 0;
    protected GridMapProxy     gridMap;

    public GridMapGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        gridMap = (GridMapProxy) proxy;

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));
        JPanel buttonPanel = new JPanel(new GridLayout(0, 5, 4, 2));

        storeButton = new JButton("Store");
        storeContOnButton = new JButton("StoreOn");
        storeContOffButton = new JButton("StoreOff");
        contStoringLabel = new JLabel();
        contStoringLabel.setText("Cont.storing off.");

        storeContOnButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                contStoring = 1;
                contStoringLabel.setText("Cont.storing on.");
            }
        });

        storeContOffButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                contStoring = 0;
                contStoringLabel.setText("Cont.storing off.");
            }
        });

        storeButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                gridMap.storeDataToFile("gridMap.png");
            }
        });

        zoomRateComboBox = new JComboBox(possibleZoomRates);
        zoomRateComboBox.setSelectedIndex(2);

        zoomRateComboBox.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                int boxValue = new Integer(possibleZoomRates[zoomRateComboBox.getSelectedIndex()]).intValue();
                if (boxValue >= 100)
                {
                    zoomRate = boxValue / 100;
                }
                else
                {
                    zoomRate = (-1) * 100 / boxValue;
                }
            }
        });

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);
        buttonPanel.add(storeButton);
        buttonPanel.add(storeContOnButton);
        buttonPanel.add(storeContOffButton);
        buttonPanel.add(zoomRateComboBox);
        buttonPanel.add(contStoringLabel);
        northPanel.add(buttonPanel, BorderLayout.CENTER);

        gridMapComponent = new GridMapComponent();

        JScrollPane scrollPane = new JScrollPane(gridMapComponent);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(scrollPane, BorderLayout.CENTER);

        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        storeButton.setEnabled(enabled);
        storeContOnButton.setEnabled(enabled);
        storeContOffButton.setEnabled(enabled);
        zoomRateComboBox.setEnabled(enabled);
        contStoringLabel.setEnabled(enabled);
    }

    protected void runData()
    {
        GridMapDataMsg data = new GridMapDataMsg();

        gridMap.getData(data);

        if ((data.gridNumX > 0) & (data.gridNumY > 0))
        {
            gridMapComponent.transformImage(zoomRate, data);

            setEnabled(true);

            if (contStoring == 1)
            {
                gridMap.storeDataToFile("gridMap" + System.currentTimeMillis() + ".png");
            }
        }
        else
        {
            setEnabled(false);
        }
    }
}
