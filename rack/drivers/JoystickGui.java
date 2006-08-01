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
package rack.drivers;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import rack.main.gui.*;
import rack.main.proxy.*;

public class JoystickGui extends RackModuleGui
{
    protected JoystickProxy joystick;
    protected boolean terminate = false;

    protected JPanel panel;
    protected JPanel buttonPanel;
    protected JPanel labelPanel;

    protected JButton onButton;
    protected JButton offButton;

    protected JLabel xLabel = new JLabel("-000 %");
    protected JLabel yLabel = new JLabel("-000 %");

    protected JLabel xNameLabel = new JLabel("X", SwingConstants.RIGHT);
    protected JLabel yNameLabel = new JLabel("Y", SwingConstants.RIGHT);

    public JoystickGui(JoystickProxy proxy)
    {
        joystick = proxy;

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
                joystick.on();
            }
        });

        offButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                joystick.off();
            }
        });

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);

        northPanel.add(new JLabel("joystick"), BorderLayout.CENTER);
        northPanel.add(buttonPanel, BorderLayout.SOUTH);

        labelPanel.add(xNameLabel);
        labelPanel.add(xLabel);
        labelPanel.add(yNameLabel);
        labelPanel.add(yLabel);

        panel.add(northPanel, BorderLayout.NORTH);
        panel.add(labelPanel, BorderLayout.CENTER);
    }

    public JComponent getComponent()
    {
        return (panel);
    }

    public String getModuleName()
    {
        return ("joystick");
    }

    public RackProxy getProxy()
    {
        return (joystick);
    }

    public void run()
    {
        JoystickDataMsg data;

        while (terminate == false)
        {
            if (panel.isShowing())
            {
                data = joystick.getData();
                if (data != null)
                {
                    xLabel.setText(data.position.x + " %");
                    yLabel.setText(data.position.y + " %");

                    xNameLabel.setEnabled(true);
                    yNameLabel.setEnabled(true);
                    xLabel.setEnabled(true);
                    yLabel.setEnabled(true);
                }
                else
                {
                    xNameLabel.setEnabled(false);
                    yNameLabel.setEnabled(false);
                    xLabel.setEnabled(false);
                    yLabel.setEnabled(false);
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
        joystick.off();
        terminate = true;
    }
}
