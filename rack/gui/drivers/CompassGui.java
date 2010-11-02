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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
package rack.gui.drivers;

import java.awt.*;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.RackModuleGui;
import rack.drivers.CompassDataMsg;
import rack.drivers.CompassProxy;

public class CompassGui extends RackModuleGui
{
    protected CompassProxy   compass;

    protected JLabel     orientationLabel          = new JLabel();
    protected JLabel     orientationNameLabel      = new JLabel("orientation", SwingConstants.RIGHT);
    protected JLabel     varOrientationLabel       = new JLabel();
    protected JLabel     varOrientationNameLabel   = new JLabel("varOrientation", SwingConstants.RIGHT);

    public CompassGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        compass = (CompassProxy) proxy;

        JPanel buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));
        JPanel labelPanel = new JPanel(new GridLayout(0, 2, 8, 0));
        JPanel northPanel = new JPanel(new BorderLayout(2, 2));

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);
        northPanel.add(new JLabel("compass"), BorderLayout.NORTH);
        northPanel.add(buttonPanel, BorderLayout.CENTER);

        labelPanel.add(orientationNameLabel);
        labelPanel.add(orientationLabel);
        labelPanel.add(varOrientationNameLabel);
        labelPanel.add(varOrientationLabel);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(labelPanel, BorderLayout.CENTER);

        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        orientationNameLabel.setEnabled(enabled);
        orientationLabel.setEnabled(enabled);
        varOrientationNameLabel.setEnabled(enabled);
        varOrientationLabel.setEnabled(enabled);
    }

    protected void runData()
    {
        CompassDataMsg data;
        double         orientation;
        double         varOrientation;

        data = compass.getData();

        if (data != null)
        {
            orientation    = Math.rint(Math.toDegrees(data.orientation) * 10.0) / 10.0;
            varOrientation = Math.rint(Math.toDegrees(data.varOrientation) * 10.0) / 10.0;
            orientationLabel.setText(orientation + "deg");
            varOrientationLabel.setText(varOrientation + "deg");

            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
    }
}
