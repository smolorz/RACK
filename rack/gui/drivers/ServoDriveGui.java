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
package rack.gui.drivers;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.RackModuleGui;
import rack.main.*;
import rack.drivers.ServoDriveDataMsg;
import rack.drivers.ServoDriveProxy;

public class ServoDriveGui extends RackModuleGui
{
    protected JButton         plusButton;
    protected JButton         minusButton;
    protected JButton         stopButton;
    protected JButton         homeButton;
    protected JButton         gotoButton;
    protected JTextField      gotoField;
    protected JButton         velButton;
    protected JTextField      velField;

    protected JLabel          positionLabel     = new JLabel("-000.00 deg");
    protected JLabel          positionNameLabel = new JLabel("Position", SwingConstants.RIGHT); ;
    protected ServoDriveProxy servoDrive;

    public ServoDriveGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        servoDrive = (ServoDriveProxy) proxy;

        JPanel buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);

        plusButton = new JButton("+");
        minusButton = new JButton("-");
        stopButton = new JButton("stop");
        homeButton = new JButton("home");
        gotoButton = new JButton("goto");
        gotoField = new JTextField("", 5);
        velButton = new JButton("vel");
        velField = new JTextField(", 5");

        plusButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                ServoDriveDataMsg data;
                data = servoDrive.getData();

                int modifiers = e.getModifiers() & (ActionEvent.CTRL_MASK | ActionEvent.SHIFT_MASK);
                if (modifiers == ActionEvent.CTRL_MASK)
                {
                    servoDrive.movePos((data.position + 100.0f), 0, 0, 200);
                }
            }
        });

        minusButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                ServoDriveDataMsg data;
                data = servoDrive.getData();

                int modifiers = e.getModifiers() & (ActionEvent.CTRL_MASK | ActionEvent.SHIFT_MASK);
                if (modifiers == ActionEvent.CTRL_MASK)
                {
                    servoDrive.movePos((data.position - 100.0f), 0, 0, 201);
                }
            }
        });

        stopButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                servoDrive.moveVel(0.0f);
            }
        });

        homeButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                int modifiers = e.getModifiers() & (ActionEvent.CTRL_MASK | ActionEvent.SHIFT_MASK);
                if (modifiers == (ActionEvent.CTRL_MASK | ActionEvent.SHIFT_MASK))
                {
                    servoDrive.home();
                }
            }
        });

        gotoButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                servoDrive.getData();
                int modifiers = e.getModifiers() & (ActionEvent.CTRL_MASK | ActionEvent.SHIFT_MASK);
                if (modifiers == ActionEvent.CTRL_MASK)
                {
                    try
                    {
                        float position = Float.parseFloat(gotoField.getText());
                        servoDrive.movePos(position, 0, 0, 200);
                    }
                    catch (NumberFormatException nfe)
                    {
                    }
                }
            }
        });

        velButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                int modifiers = e.getModifiers() & (ActionEvent.CTRL_MASK | ActionEvent.SHIFT_MASK);
                if (modifiers == ActionEvent.CTRL_MASK)
                {
                    try
                    {
                        float velocity = Float.parseFloat(velField.getText());
                        servoDrive.moveVel(velocity);
                    }
                    catch (NumberFormatException nfe)
                    {
                    }
                }
            }
        });


        buttonPanel.add(homeButton);
        buttonPanel.add(plusButton);
        buttonPanel.add(stopButton);
        buttonPanel.add(minusButton);
        buttonPanel.add(gotoButton);
        buttonPanel.add(gotoField);
        buttonPanel.add(velButton);
        buttonPanel.add(velField);

        buttonPanel.add(positionNameLabel);
        buttonPanel.add(positionLabel);

        rootPanel.add(new JLabel(RackName.nameString(servoDrive.getCommandMbx())), BorderLayout.NORTH);
        rootPanel.add(buttonPanel, BorderLayout.CENTER);

        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        plusButton.setEnabled(enabled);
        minusButton.setEnabled(enabled);
        stopButton.setEnabled(true);
        homeButton.setEnabled(enabled);
        gotoButton.setEnabled(enabled);
        gotoField.setEnabled(enabled);
        velButton.setEnabled(enabled);
        velField.setEnabled(enabled);

        positionLabel.setEnabled(enabled);
        positionNameLabel.setEnabled(enabled);
    }

    protected void runData()
    {
        ServoDriveDataMsg data;

        data = servoDrive.getData();

        if (data != null)
        {
            // positionLabel.setText(Math.rint(Math.toDegrees(data.position)*100.0)/100.0 + " deg";
            positionLabel.setText((data.position) + "mm");
            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
    }
}
