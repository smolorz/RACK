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
 *      Oliver Wulf      <oliver.wulf@web.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.gui.drivers;

import java.awt.*;
import java.awt.event.*;

import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.RackDataModuleGui;
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
    protected JButton pilotButton[];

    protected ChassisProxy chassisProxy;
    protected PilotProxy[] pilotProxy;
    protected String[]     pilotName;

    public JoystickSoftware(GuiElementDescriptor guiElement)
    {
        super(RackName.JOYSTICK, guiElement.getInstance(), 100, guiElement);

        joystickProxy = (JoystickProxy)proxy;

        chassisProxy = (ChassisProxy) mainGui.getProxy(RackName.CHASSIS, 0);

        int i;
        for(i = 0; i < 8; i++)
        {
            if(mainGui.getGuiElement(RackName.PILOT, i) == null)
            {
                break;
            }
        }
        pilotProxy = new PilotProxy[i];
        pilotName = new String[i];
        for(i = 0; i < pilotProxy.length; i++)
        {
            GuiElementDescriptor ge = mainGui.getGuiElement(RackName.PILOT, i);
            pilotProxy[i] = (PilotProxy)ge.getProxy();
            pilotName[i] = ge.getName();
        }

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
        labelPanel.add(buttonsNameLabel);
        labelPanel.add(buttonsLabel);

        outputData.position.x = 0;
        outputData.position.y = 0;
        outputData.buttons    = 0;

        forwardButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                forward();
                stopButton.grabFocus();
            }
        });
        backwardButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                backward();
                stopButton.grabFocus();
            }
        });
        leftButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                left();
                stopButton.grabFocus();
            }
        });
        rightButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                right();
                stopButton.grabFocus();
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

        ActionListener pilotButtonActionListener = new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                int pilot = Integer.parseInt(e.getActionCommand());
                activatePilot(pilot);
                stopButton.grabFocus();
            }
        };

        pilotPanel = new JPanel(new GridLayout(0, 1, 2, 2));
        pilotButton = new JButton[pilotProxy.length + 1];
        
        for(i = 0; i < pilotProxy.length; i++)
        {
            pilotButton[i] = new JButton(pilotName[i]);
            pilotButton[i].setActionCommand(Integer.toString(i));
            pilotButton[i].addActionListener(pilotButtonActionListener);
            pilotPanel.add(pilotButton[i]);
        }

        i = pilotProxy.length;
        pilotButton[i] = new JButton("No Pilot");
        pilotButton[i].setActionCommand(Integer.toString(ChassisProxy.INVAL_PILOT));
        pilotButton[i].addActionListener(pilotButtonActionListener);
        pilotPanel.add(pilotButton[i]);

        JPanel centerPanel = new JPanel(new BorderLayout(2, 2));
        centerPanel.add(labelPanel, BorderLayout.NORTH);
        centerPanel.add(steeringPanel, BorderLayout.CENTER);

        // panel.add(Box.createRigidArea(new Dimension(0,20)));
        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(centerPanel, BorderLayout.CENTER);
        rootPanel.add(pilotPanel, BorderLayout.SOUTH);

        KeyListener keyListener = new KeyListener()
        {
            public void keyPressed(KeyEvent e)
            {
                if((e.getKeyCode() >= KeyEvent.VK_1) && (e.getKeyCode() <= KeyEvent.VK_9))
                {
                    zero();
                    activatePilot(e.getKeyCode() - KeyEvent.VK_1);
                }
                else
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
                        case KeyEvent.VK_SPACE:
                            zero();
                            break;
                        default:
                            zero();
                            activatePilot(ChassisProxy.INVAL_PILOT);
                    }
                }
                stopButton.grabFocus();
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

        rootPanel.addKeyListener(keyListener);

        onButton.addKeyListener(keyListener);
        offButton.addKeyListener(keyListener);

        forwardButton.addKeyListener(keyListener);
        backwardButton.addKeyListener(keyListener);
        leftButton.addKeyListener(keyListener);
        rightButton.addKeyListener(keyListener);
        stopButton.addKeyListener(keyListener);

        for(i = 0; i < pilotButton.length; i++)
        {
            pilotButton[i].addKeyListener(keyListener);
        }

        setEnabled(false);
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
        zero();
    }

    public synchronized void f2Down()
    {
        outputData.buttons |= 0x02;
    }

    public synchronized void f2Up()
    {
        outputData.buttons &= ~0x02;
        zero();
    }

    public synchronized void f3Down()
    {
        outputData.buttons |= 0x04;
    }

    public synchronized void f3Up()
    {
        outputData.buttons &= ~0x04;
        zero();
    }

    public synchronized void f4Down()
    {
        outputData.buttons |= 0x08;
    }

    public synchronized void f4Up()
    {
        outputData.buttons &= ~0x08;
        zero();
    }

    public synchronized void activatePilot(int pilot)
    {
        zero();

        // turn off pilots
        for(int i = 0; i < pilotProxy.length; i++)
        {
            if((i != pilot) && (pilotProxy[i] != null))
                pilotProxy[i].off();
        }

        // set active pilot
        if((pilot >= 0) && (chassisProxy != null))
        {
            chassisProxy.on();
            chassisProxy.setActivePilot(RackName.create(RackName.PILOT, pilot));
        }
        else
        {
            chassisProxy.setActivePilot(ChassisProxy.INVAL_PILOT);
        }

        // turn on pilot
        if((pilot >= 0) && (pilot < pilotProxy.length))
        {
            pilotProxy[pilot].on();
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
            boolean pilotButtonHasFocus = false;
            for(int i = 0; i < pilotButton.length; i++)
            {
                if(pilotButton[i].hasFocus())
                {
                    pilotButtonHasFocus = true;
                    break;
                }
            }

            if( !rootPanel.hasFocus() &
                !onButton.hasFocus() &
                !offButton.hasFocus() &
                !forwardButton.hasFocus() &
                !backwardButton.hasFocus() &
                !leftButton.hasFocus() &
                !rightButton.hasFocus() &
                !stopButton.hasFocus() &
                !pilotButtonHasFocus)
            {
                zero();
            }

            gdos.dbgInfo("Writing data ... ");
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

    public void moduleCommand(TimsRawMsg raw)
    {
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

        for(int i = 0; i < pilotButton.length; i++)
        {
            pilotButton[i].setEnabled(enabled);
        }
    }
}
