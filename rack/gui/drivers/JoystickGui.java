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
import rack.gui.main.RackModuleGui;
import rack.drivers.JoystickDataMsg;
import rack.drivers.JoystickProxy;

public class JoystickGui extends RackModuleGui
{
    protected JoystickProxy joystick;

    protected JLabel        xLabel     = new JLabel("-000 %");
    protected JLabel        yLabel     = new JLabel("-000 %");

    protected JLabel        xNameLabel = new JLabel("X", SwingConstants.RIGHT);
    protected JLabel        yNameLabel = new JLabel("Y", SwingConstants.RIGHT);

    public JoystickGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        joystick = (JoystickProxy) proxy;

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));
        JPanel buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));
        JPanel labelPanel = new JPanel(new GridLayout(0, 2, 8, 0));

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);

        northPanel.add(new JLabel("joystick"), BorderLayout.CENTER);
        northPanel.add(buttonPanel, BorderLayout.SOUTH);

        labelPanel.add(xNameLabel);
        labelPanel.add(xLabel);
        labelPanel.add(yNameLabel);
        labelPanel.add(yLabel);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(labelPanel, BorderLayout.CENTER);
        
        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        xNameLabel.setEnabled(enabled);
        yNameLabel.setEnabled(enabled);
        xLabel.setEnabled(enabled);
        yLabel.setEnabled(enabled);
    }

    protected void updateData()
    {
        JoystickDataMsg data;

        data = joystick.getData();
        if (data != null)
        {
            xLabel.setText(data.position.x + " %");
            yLabel.setText(data.position.y + " %");

            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
    }
}
