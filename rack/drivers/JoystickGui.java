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

    protected JPanel steeringPanel;
    protected JButton forwardButton = new JButton("/\\");
    protected JButton backwardButton = new JButton("\\/");
    protected JButton leftButton = new JButton("<");
    protected JButton rightButton = new JButton(">");
    protected JButton stopButton = new JButton("stop");
    protected JoystickDataMsg outputData = new JoystickDataMsg();

    public synchronized void forward()
    {
        outputData.position.x += 10;
        if (outputData.position.x > 100)
        {
            outputData.position.x = 100;
        }
        joystick.setData(outputData);
    }

    public synchronized void backward()
    {
        outputData.position.x -= 10;
        if (outputData.position.x < -100)
        {
            outputData.position.x = -100;
        }
        joystick.setData(outputData);
    }

    public synchronized void left()
    {
        outputData.position.y -= 10;
        if (outputData.position.y < -100)
        {
            outputData.position.y = -100;
        }
        joystick.setData(outputData);
    }

    public synchronized void right()
    {
        outputData.position.y += 10;
        if (outputData.position.y > 100)
        {
            outputData.position.y = 100;
        }
        joystick.setData(outputData);
    }

    public synchronized void zero()
    {
        outputData.position.x = 0;
        outputData.position.y = 0;
        joystick.setData(outputData);
    }

    public synchronized void streight()
    {
        outputData.position.y = 0;
        joystick.setData(outputData);
    }

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

        outputData.position.x = 0;
        outputData.position.y = 0;

        forwardButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                forward();
            }
        });

        backwardButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                backward();
            }
        });

        leftButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                left();
            }
        });

        rightButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                right();
            }
        });

        stopButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                zero();
            }
        });

        steeringPanel = new JPanel(new GridLayout(3, 3));
        steeringPanel.add(new JLabel());
        steeringPanel.add(forwardButton);
        steeringPanel.add(new JLabel());
        steeringPanel.add(leftButton);
        steeringPanel.add(stopButton);
        steeringPanel.add(rightButton);
        steeringPanel.add(new JLabel());
        steeringPanel.add(backwardButton);
        steeringPanel.add(new JLabel());
        // steeringPanel.setMaximumSize(new Dimension(200,200));

        // panel.add(Box.createRigidArea(new Dimension(0,20)));
        panel.add(northPanel, BorderLayout.NORTH);
        panel.add(labelPanel, BorderLayout.CENTER);
        panel.add(steeringPanel, BorderLayout.SOUTH);

        KeyListener keyListener = new KeyListener()
        {
            public void keyPressed(KeyEvent e)
            {
                switch (e.getKeyCode())
                {
                    case KeyEvent.VK_UP:
                    case KeyEvent.VK_KP_UP:
                    case KeyEvent.VK_NUMPAD8:
                        forward();
                        break;
                    case KeyEvent.VK_DOWN:
                    case KeyEvent.VK_KP_DOWN:
                    case KeyEvent.VK_NUMPAD2:
                        backward();
                        break;
                    case KeyEvent.VK_LEFT:
                    case KeyEvent.VK_KP_LEFT:
                    case KeyEvent.VK_NUMPAD4:
                        left();
                        break;
                    case KeyEvent.VK_RIGHT:
                    case KeyEvent.VK_KP_RIGHT:
                    case KeyEvent.VK_NUMPAD6:
                        right();
                        break;
                    case KeyEvent.VK_SHIFT:
                    case KeyEvent.VK_NUMPAD5:
                        streight();
                        break;
                    default:
                        zero();
                }
            }

            public void keyReleased(KeyEvent e)
            {
            }

            public void keyTyped(KeyEvent e)
            {
            }
        };

        onButton.addKeyListener(keyListener);
        offButton.addKeyListener(keyListener);
        forwardButton.addKeyListener(keyListener);
        backwardButton.addKeyListener(keyListener);
        leftButton.addKeyListener(keyListener);
        rightButton.addKeyListener(keyListener);
        stopButton.addKeyListener(keyListener);
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
                    outputData.position.x = data.position.x;
                    outputData.position.y = data.position.y;

                    xNameLabel.setEnabled(true);
                    yNameLabel.setEnabled(true);
                    xLabel.setEnabled(true);
                    yLabel.setEnabled(true);
                    forwardButton.setEnabled(true);
                    backwardButton.setEnabled(true);
                    leftButton.setEnabled(true);
                    rightButton.setEnabled(true);
                    stopButton.setEnabled(true);
                }
                else
                {
                    xNameLabel.setEnabled(false);
                    yNameLabel.setEnabled(false);
                    xLabel.setEnabled(false);
                    yLabel.setEnabled(false);
                    /*
                     * outputData.position.x = 0; outputData.position.y = 0;
                     */
                    forwardButton.setEnabled(false);
                    backwardButton.setEnabled(false);
                    leftButton.setEnabled(false);
                    rightButton.setEnabled(false);
                    stopButton.setEnabled(false);
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
