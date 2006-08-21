/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Oliver Wulf      <wulf@rts.uni-hannover.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.gui.drivers;

import java.awt.*;
import java.awt.event.*;

import javax.swing.*;

import rack.gui.main.RackDataModuleGui;
import rack.gui.main.RackModuleGui;
import rack.main.*;
import rack.main.tims.*;
import rack.navigation.PilotProxy;
import rack.drivers.ChassisProxy;
import rack.drivers.JoystickDataMsg;
import rack.drivers.JoystickProxy;

/**
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class JoystickSoftware extends RackDataModuleGui
{
    protected JoystickProxy joystickProxy;

    protected JPanel panel;
    protected JPanel buttonPanel;
    protected JPanel labelPanel;

    protected JButton onButton;
    protected JButton offButton;

    protected JPanel centerPanel;

    protected JLabel xLabel = new JLabel("0%");
    protected JLabel yLabel = new JLabel("0%");
    protected JLabel buttonsLabel = new JLabel("00000000");

    protected JLabel xNameLabel = new JLabel("x", SwingConstants.RIGHT);
    protected JLabel yNameLabel = new JLabel("y", SwingConstants.RIGHT);
    protected JLabel buttonsNameLabel = new JLabel("buttons", SwingConstants.RIGHT);

    protected JPanel steeringPanel;
    protected JButton forwardButton = new JButton("/\\");
    protected JButton backwardButton = new JButton("\\/");
    protected JButton leftButton = new JButton("<");
    protected JButton rightButton = new JButton(">");
    protected JButton stopButton = new JButton("stop");
    protected JoystickDataMsg outputData = new JoystickDataMsg();

    protected JPanel pilotPanel;
    protected JButton pilot0Button = new JButton("Pilot(0)");
    protected JButton pilot1Button = new JButton("Pilot(1)");
    protected JButton pilot2Button = new JButton("Pilot(2)");
    protected JButton noPilotButton = new JButton("No Pilot");

    protected RackProxy[] rackProxyList;

    public JoystickSoftware(Integer moduleIndex, RackProxy[] proxyList, RackModuleGui[] guiList)
    {
        super(RackName.create(RackName.JOYSTICK, proxyList[moduleIndex.intValue()].getInstanceId()));
        this.joystickProxy = (JoystickProxy)proxyList[moduleIndex.intValue()];
        this.rackProxyList = proxyList;
        this.periodTime = 100;

        panel = new JPanel(new BorderLayout(2, 2));
        panel.setBorder(BorderFactory.createEmptyBorder(2, 2, 2, 2));

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));

        buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));

        labelPanel = new JPanel(new GridLayout(0, 2, 8, 0));

        onButton = new JButton("On");
        offButton = new JButton("Off");

        onButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                joystickProxy.on();
            }
        });

        offButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                joystickProxy.off();
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
        labelPanel.add(buttonsNameLabel);
        labelPanel.add(buttonsLabel);

        outputData.position.x = 0;
        outputData.position.y = 0;
        outputData.buttons    = 0;

        setEnabled(false);

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

        pilot0Button.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                pilot(0);
            }
        });

        pilot1Button.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                pilot(1);
            }
        });

        pilot2Button.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                pilot(2);
            }
        });

        noPilotButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                pilot(ChassisProxy.INVAL_PILOT);
            }
        });

        pilotPanel = new JPanel(new GridLayout(0, 1, 2, 2));
        pilotPanel.add(pilot0Button);
        pilotPanel.add(pilot1Button);
        pilotPanel.add(pilot2Button);
        pilotPanel.add(noPilotButton);

        centerPanel = new JPanel(new BorderLayout(2, 2));
        centerPanel.add(labelPanel, BorderLayout.NORTH);
        centerPanel.add(steeringPanel, BorderLayout.CENTER);

        // panel.add(Box.createRigidArea(new Dimension(0,20)));
        panel.add(northPanel, BorderLayout.NORTH);
        panel.add(centerPanel, BorderLayout.CENTER);
        panel.add(pilotPanel, BorderLayout.SOUTH);

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
                    case KeyEvent.VK_F1:
                        f1Down();
                        break;
                    case KeyEvent.VK_F2:
                        f2Down();
                        break;
                    case KeyEvent.VK_F3:
                        f3Down();
                        break;
                    case KeyEvent.VK_F4:
                        f4Down();
                        break;
                    default:
                        zero();
                }
            }

            public void keyReleased(KeyEvent e)
            {
                switch (e.getKeyCode())
                {
                    case KeyEvent.VK_F1:
                        f1Up();
                        break;
                    case KeyEvent.VK_F2:
                        f2Up();
                        break;
                    case KeyEvent.VK_F3:
                        f3Up();
                        break;
                    case KeyEvent.VK_F4:
                        f4Up();
                        break;
                }
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

        pilot0Button.addKeyListener(keyListener);
        pilot1Button.addKeyListener(keyListener);
        pilot2Button.addKeyListener(keyListener);
        noPilotButton.addKeyListener(keyListener);
    }

    public synchronized void forward()
    {
        outputData.position.x += 10;
        if (outputData.position.x > 100)
            outputData.position.x = 100;
    }

    public synchronized void backward()
    {
        outputData.position.x -= 10;
        if (outputData.position.x < -100)
            outputData.position.x = -100;
    }

    public synchronized void left()
    {
        outputData.position.y -= 10;
        if (outputData.position.y < -100)
            outputData.position.y = -100;
    }

    public synchronized void right()
    {
        outputData.position.y += 10;
        if (outputData.position.y > 100)
            outputData.position.y = 100;
    }

    public synchronized void zero()
    {
        outputData.position.x = 0;
        outputData.position.y = 0;
    }

    public synchronized void streight()
    {
        outputData.position.y = 0;
    }

    public synchronized void f1Down()
    {
        outputData.buttons |= 0x01;
    }

    public synchronized void f1Up()
    {
        outputData.buttons &= ~0x01;
        outputData.position.x = 0;
        outputData.position.y = 0;
    }

    public synchronized void f2Down()
    {
        outputData.buttons |= 0x02;
    }

    public synchronized void f2Up()
    {
        outputData.buttons &= ~0x02;
        outputData.position.x = 0;
        outputData.position.y = 0;
    }

    public synchronized void f3Down()
    {
        outputData.buttons |= 0x04;
    }

    public synchronized void f3Up()
    {
        outputData.buttons &= ~0x04;
        outputData.position.x = 0;
        outputData.position.y = 0;
    }

    public synchronized void f4Down()
    {
        outputData.buttons |= 0x08;
    }

    public synchronized void f4Up()
    {
        outputData.buttons &= ~0x08;
        outputData.position.x = 0;
        outputData.position.y = 0;
    }

    public synchronized void pilot(int pilot)
    {
        outputData.position.x = 0;
        outputData.position.y = 0;

        // turn off pilots
        for(int i = 0; i < 3; i++)
        {
            for(int j = 0; j < rackProxyList.length; j++)
            {
                if(rackProxyList[j].getCommandMbx() == RackName.create(RackName.PILOT, i))
                {
                    PilotProxy pilotProxy = (PilotProxy)rackProxyList[j];
                    if(i != pilot)
                        pilotProxy.off();
                }
            }
        }

        // set active pilot
        for(int j = 0; j < rackProxyList.length; j++)
        {
            if(rackProxyList[j].getCommandMbx() == RackName.create(RackName.CHASSIS, 0))
            {
                ChassisProxy chassisProxy = (ChassisProxy)rackProxyList[j];
                if(pilot >= 0)
                {
                    chassisProxy.setActivePilot(RackName.create(RackName.PILOT, pilot));
                }
                else
                {
                    chassisProxy.setActivePilot(ChassisProxy.INVAL_PILOT);
                }
            }
        }

        // turn on pilot
        if(pilot >= 0)
        {
            for(int j = 0; j < rackProxyList.length; j++)
            {
                if(rackProxyList[j].getCommandMbx() == RackName.create(RackName.PILOT, pilot))
                {
                    PilotProxy pilotProxy = (PilotProxy)rackProxyList[j];
                    pilotProxy.on();
                }
            }
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see rcc.middleware.module_primitives.ContinuousDataModule#moduleOn()
     */
    public boolean moduleOn()
    {
        setEnabled(true);

        return true;
    }

    /*
     * (non-Javadoc)
     *
     * @see rcc.middleware.module_primitives.ContinuousDataModule#moduleOff()
     */
    public void moduleOff()
    {
        synchronized (this)
        {
            outputData.position.x = 0;
            outputData.position.y = 0;
            outputData.buttons = 0;
        }

        xLabel.setText("0%");
        yLabel.setText("0%");
        buttonsLabel.setText("");

        setEnabled(false);
    }

    /*
     * (non-Javadoc)
     *
     * @see rcc.middleware.module_primitives.ContinuousDataModule#moduleLoop()
     */
    public boolean moduleLoop()
    {
        synchronized (this)
        {
            GDOS.dbgInfo("Writing data ... ", commandMbx, gdosLevel);
            writeWorkMsg(outputData);
        }

        xLabel.setText(outputData.position.x + "%");
        yLabel.setText(outputData.position.y + "%");
        buttonsLabel.setText(Integer.toBinaryString(outputData.buttons));

        try
        {
            Thread.sleep(periodTime);
        }
        catch (InterruptedException e)
        {
        }

        return true;
    }

    public void moduleCleanup()
    {
    }

    public void moduleCommand(TimsDataMsg raw)
    {
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
        return (joystickProxy);
    }

    public void setEnabled(boolean enabled)
    {
        xNameLabel.setEnabled(enabled);
        yNameLabel.setEnabled(enabled);
        buttonsNameLabel.setEnabled(enabled);
        xLabel.setEnabled(enabled);
        yLabel.setEnabled(enabled);

        buttonsLabel.setEnabled(enabled);
        forwardButton.setEnabled(enabled);
        backwardButton.setEnabled(enabled);
        leftButton.setEnabled(enabled);
        rightButton.setEnabled(enabled);
        stopButton.setEnabled(enabled);

        pilot0Button.setEnabled(enabled);
        pilot1Button.setEnabled(enabled);
        pilot2Button.setEnabled(enabled);
        noPilotButton.setEnabled(enabled);
    }
}
