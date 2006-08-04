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

import rack.drivers.ChassisDataMsg;
import rack.drivers.ChassisProxy;
import rack.gui.main.MapViewDrawContext;
import rack.gui.main.RackModuleGui;
import rack.main.proxy.*;
import rack.main.naming.*;

public class ChassisGui extends RackModuleGui
{
    protected boolean terminate = false;
    
    protected ChassisDataMsg chassisData;
    protected boolean mapViewIsShowing;
    
    protected JButton onButton = new JButton("On");
    protected JButton offButton = new JButton("Off");

    protected JRadioButton pilot0 = new JRadioButton("pilot(0)", false);
    protected JRadioButton pilot1 = new JRadioButton("pilot(1)", false);
    protected JRadioButton pilot2 = new JRadioButton("pilot(2)", false);
    protected JRadioButton pilot3 = new JRadioButton("pilot(3)", false);
    protected JRadioButton pilot4 = new JRadioButton("pilot(4)", false);
    protected JRadioButton pilot5 = new JRadioButton("pilot(5)", false);
    protected JRadioButton pilot6 = new JRadioButton("pilot(6)", false);

    protected JRadioButton otherPilot = new JRadioButton("other pilot", false);
    protected JRadioButton disablePilot = new JRadioButton("disabled", false);
    protected ButtonGroup pilotGroup = new ButtonGroup();

    protected JPanel panel;
    protected JPanel buttonPanel;
    protected JPanel labelPanel;

    protected JLabel vxLabel          = new JLabel("-0.00 m/s");
    protected JLabel vyLabel          = new JLabel("-0.00 m/s");
    protected JLabel omegaLabel       = new JLabel("-0.00 deg/s");
    protected JLabel batteryLabel     = new JLabel();


    protected JLabel vxNameLabel      = new JLabel("vx:",
                                                   SwingConstants.RIGHT);
    protected JLabel vyNameLabel      = new JLabel("vy:",
                                                   SwingConstants.RIGHT);
    protected JLabel omegaNameLabel   = new JLabel("omega:",
                                                   SwingConstants.RIGHT);
    protected JLabel batteryNameLabel = new JLabel("battery:",
                                                   SwingConstants.RIGHT);

    public ChassisProxy chassis;

    public ChassisGui(ChassisProxy proxy)
    {
        chassis = proxy;

        panel = new JPanel(new BorderLayout(2, 2));
        panel.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));

        buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));

        labelPanel = new JPanel(new GridLayout(0, 3, 8, 0));

        onButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                chassis.on();
            }
        });

        offButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                chassis.off();
            }
        });

        disablePilot.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(ChassisProxy.INVAL_PILOT);
            }
        });

        pilot0.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 0));
            }
        });

        pilot1.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 1));
            }
        });

        pilot2.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 2));
            }
        });

        pilot3.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 3));
            }
        });

        pilot4.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 4));
            }
        });

        pilot5.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 5));
            }
        });

        pilot6.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 6));
            }
        });

        pilotGroup.add(pilot0);
        pilotGroup.add(pilot1);
        pilotGroup.add(pilot2);
        pilotGroup.add(pilot3);
        pilotGroup.add(pilot4);
        pilotGroup.add(pilot5);
        pilotGroup.add(pilot6);

        pilotGroup.add(otherPilot);
        pilotGroup.add(disablePilot);

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);

        northPanel.add(new JLabel("chassis"), BorderLayout.CENTER);
        northPanel.add(buttonPanel, BorderLayout.SOUTH);

        labelPanel.add(pilot0);
        labelPanel.add(vxNameLabel);
        labelPanel.add(vxLabel);
        labelPanel.add(pilot1);
        labelPanel.add(vyNameLabel);
        labelPanel.add(vyLabel);
        labelPanel.add(pilot2);
        labelPanel.add(omegaNameLabel);
        labelPanel.add(omegaLabel);
        labelPanel.add(pilot3);
        labelPanel.add(batteryNameLabel);
        labelPanel.add(batteryLabel);
        labelPanel.add(pilot4);
        labelPanel.add(new JLabel());
        labelPanel.add(new JLabel());
        labelPanel.add(pilot5);
        labelPanel.add(new JLabel());
        labelPanel.add(new JLabel());
        labelPanel.add(pilot6);
        labelPanel.add(new JLabel());
        labelPanel.add(new JLabel());
        labelPanel.add(otherPilot);
        labelPanel.add(new JLabel());
        labelPanel.add(new JLabel());
        labelPanel.add(disablePilot);
        labelPanel.add(new JLabel());
        labelPanel.add(new JLabel());

        panel.add(northPanel, BorderLayout.NORTH);
        panel.add(labelPanel, BorderLayout.CENTER);
    }

    public JComponent getComponent()
    {
        return (panel);
    }

    public String getModuleName()
    {
        return ("Chassis");
    }

    public RackProxy getProxy()
    {
        return (chassis);
    }

    public void run()
    {
        ChassisDataMsg data;
        float vx, vy;
        float omega;
        float battery;

        while(terminate==false)
        {
            if(panel.isShowing() | (mapViewIsShowing))
            {
                data = chassis.getData();
                synchronized(this)
                {
                    chassisData = data;
                }

                if(data != null)
                {
                    vx      = data.vx;
                    vy      = data.vy;
                    omega   = data.omega ;
                    battery = data.battery;

                    vxNameLabel.setEnabled(true);
                    vyNameLabel.setEnabled(true);
                    omegaNameLabel.setEnabled(true);
                    batteryNameLabel.setEnabled(true);
                    pilot0.setEnabled(true);
                    pilot1.setEnabled(true);
                    pilot2.setEnabled(true);
                    pilot3.setEnabled(true);
                    pilot4.setEnabled(true);
                    pilot5.setEnabled(true);
                    pilot6.setEnabled(true);
                    otherPilot.setEnabled(true);
                    disablePilot.setEnabled(true);

                    if(data.activePilot == RackName.create(
                                           RackName.PILOT, 0))
                    {
                        pilot0.setSelected(true);
                    }
                    else if(data.activePilot == RackName.create(
                                                RackName.PILOT, 1))
                    {
                        pilot1.setSelected(true);
                    }
                    else if(data.activePilot == RackName.create(
                                                RackName.PILOT, 2))
                    {
                        pilot2.setSelected(true);
                    }
                    else if(data.activePilot == RackName.create(
                                                RackName.PILOT, 3))
                    {
                        pilot3.setSelected(true);
                    }
                    else if(data.activePilot == RackName.create(
                                                RackName.PILOT, 4))
                    {
                        pilot4.setSelected(true);
                    }
                    else if(data.activePilot == RackName.create(
                                                RackName.PILOT, 5))
                    {
                        pilot5.setSelected(true);
                    }
                    else if(data.activePilot == RackName.create(
                                                RackName.PILOT, 6))
                    {
                        pilot6.setSelected(true);
                    }

                    else if(data.activePilot < 0)
                    {
                        disablePilot.setSelected(true);
                    }
                    else
                    {
                        otherPilot.setSelected(true);
                        otherPilot.setText("0x" + Integer.toHexString(data.activePilot));
                    }
                    vxLabel.setText(Math.rint(Math.round(vx/10.0))/
                                              100.0 + " m/s");
                    vyLabel.setText(Math.rint(Math.round(vy/10.0))/
                                              100.0 + " m/s");
                    omegaLabel.setText(Math.rint(Math.toDegrees(omega)*100.0)/
                                              100.0 + " deg/s");
                    batteryLabel.setText(battery+"");
                }
                else
                {
                    vxLabel.setText("");
                    vyLabel.setText("");
                    omegaLabel.setText("");
                    batteryLabel.setText("");

                    vxNameLabel.setEnabled(false);
                    vyNameLabel.setEnabled(false);
                    omegaNameLabel.setEnabled(false);
                    batteryNameLabel.setEnabled(false);
                    disablePilot.setSelected(true);
                    pilot0.setEnabled(false);
                    pilot1.setEnabled(false);
                    pilot2.setEnabled(false);
                    pilot3.setEnabled(false);
                    pilot4.setEnabled(false);
                    pilot5.setEnabled(false);
                    pilot6.setEnabled(false);
                    otherPilot.setEnabled(false);
                    disablePilot.setEnabled(false);
                }
                
                mapViewIsShowing = false;                
            }
            try
            {
                Thread.sleep(500);
            }
            catch(InterruptedException e)
            {
            }
        }
    }

    public void paintMapView(MapViewDrawContext drawContext) 
    {
        mapViewIsShowing = true;
        
        synchronized(this)
        {
            if (chassisData == null) return;
    
            Graphics2D graphics = drawContext.getFrameGraphics();
    
            graphics.setColor(Color.LIGHT_GRAY);
            graphics.fillRect(10,30,10,200);
            graphics.fillRect(30,10,200,10);
    
            graphics.setColor(Color.RED);
            int vx = (int)(chassisData.vx / 10.0f);

            if(vx > 100)
                vx = 100;
            if(vx < -100)
                vx = -100;
            
            if(vx > 0)
            {
                graphics.fillRect(10, 130 - vx, 10, vx);
            }
            if(vx < 0)
            {
                graphics.fillRect(10, 130, 10, -vx);
            }
            
            graphics.setColor(Color.RED);
            int omega = (int)(Math.toDegrees((double)chassisData.omega) * 5.0);

            if(omega > 100)
                omega = 100;
            if(omega < -100)
                omega = -100;
            
            if(omega > 0)
            {
                graphics.fillRect(130, 10, omega, 10);
            }
            if(omega < 0)
            {
                graphics.fillRect(130 + omega, 10, -omega, 10);
            }
    
            graphics.setColor(Color.BLACK);
            graphics.drawLine(10,130,19,130);
            graphics.drawLine(130,10,130,19);
        }
    }

    public void terminate()
    {
        terminate = true;
    }
}
