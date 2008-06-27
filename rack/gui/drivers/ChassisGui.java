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
 *      Joerg Langenberg  <joerg.langenberg@gmx.net>
 *      Marco Langerwisch <marco.langerwisch@web.de>
 *
 */
package rack.gui.drivers;

import java.awt.*;
import java.awt.event.*;

import javax.sound.midi.MidiChannel;
import javax.sound.midi.MidiSystem;
import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.Synthesizer;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.*;
import rack.main.*;
import rack.drivers.ChassisDataMsg;
import rack.drivers.ChassisProxy;

public class ChassisGui extends RackModuleGui
{
    protected JRadioButton   pilot0           = new JRadioButton("pilot(0)", false);
    protected JRadioButton   pilot1           = new JRadioButton("pilot(1)", false);
    protected JRadioButton   pilot2           = new JRadioButton("pilot(2)", false);
    protected JRadioButton   pilot3           = new JRadioButton("pilot(3)", false);
    protected JRadioButton   pilot4           = new JRadioButton("pilot(4)", false);
    protected JRadioButton   pilot5           = new JRadioButton("pilot(5)", false);
    protected JRadioButton   pilot6           = new JRadioButton("pilot(6)", false);

    protected JRadioButton   otherPilot       = new JRadioButton("other pilot", false);
    protected JRadioButton   disablePilot     = new JRadioButton("disabled", false);
    protected ButtonGroup    pilotGroup       = new ButtonGroup();

    protected ActionListener pilot0Action;
    protected ActionListener pilot1Action;
    protected ActionListener pilot2Action;
    protected ActionListener pilot3Action;
    protected ActionListener pilot4Action;
    protected ActionListener pilot5Action;
    protected ActionListener pilot6Action;
    protected ActionListener disablePilotAction;

    protected JLabel         vxLabel          = new JLabel("-0.00 m/s");
    protected JLabel         vyLabel          = new JLabel("-0.00 m/s");
    protected JLabel         omegaLabel       = new JLabel("-0.00 deg/s");
    protected JLabel         batteryLabel     = new JLabel();

    protected JLabel         vxNameLabel      = new JLabel("vx:", SwingConstants.RIGHT);
    protected JLabel         vyNameLabel      = new JLabel("vy:", SwingConstants.RIGHT);
    protected JLabel         omegaNameLabel   = new JLabel("omega:", SwingConstants.RIGHT);
    protected JLabel         batteryNameLabel = new JLabel("battery:", SwingConstants.RIGHT);

    protected ChassisProxy   chassis;
    
    // for sound output
	private Synthesizer	    synth;
	private MidiChannel     midiChannels[];
	private int             currentNote;
	private int             vxMax;

    public ChassisGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        chassis = (ChassisProxy) proxy;

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));
        JPanel buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));
        JPanel labelPanel = new JPanel(new GridLayout(0, 3, 8, 0));

        disablePilotAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(ChassisProxy.INVAL_PILOT);
            }
        };
        disablePilot.addActionListener(disablePilotAction);

        pilot0Action = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 0));
            }
        };
        pilot0.addActionListener(pilot0Action);

        pilot1Action = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 1));
            }
        };
        pilot1.addActionListener(pilot1Action);

        pilot2Action = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 2));
            }
        };
        pilot2.addActionListener(pilot2Action);

        pilot3Action = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 3));
            }
        };
        pilot3.addActionListener(pilot3Action);

        pilot4Action = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 4));
            }
        };
        pilot4.addActionListener(pilot4Action);

        pilot5Action = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 5));
            }
        };
        pilot5.addActionListener(pilot5Action);

        pilot6Action = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                chassis.setActivePilot(RackName.create(RackName.PILOT, 6));
            }
        };
        pilot6.addActionListener(pilot6Action);

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

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(labelPanel, BorderLayout.CENTER);
        
        setEnabled(false);
        
        if (guiElement.hasParameter("sound"))
        {
		    try
		    {
			    synth = MidiSystem.getSynthesizer();
			    synth.open();
			    midiChannels = synth.getChannels();
			    midiChannels[0].programChange(43);
		    }
		    catch (MidiUnavailableException e)
		    {
			    e.printStackTrace();
		    }
		
		    currentNote = 0;
		    vxMax = chassis.getParam().vxMax;
        }
        else
        {
        	synth = null;
        }
    }

    protected void setEnabled(boolean enabled)
    {
        vxNameLabel.setEnabled(enabled);
        vxLabel.setEnabled(enabled);
        vyNameLabel.setEnabled(enabled);
        vyLabel.setEnabled(enabled);
        omegaNameLabel.setEnabled(enabled);
        omegaLabel.setEnabled(enabled);
        batteryNameLabel.setEnabled(enabled);
        batteryLabel.setEnabled(enabled);
        pilot0.setEnabled(enabled);
        pilot1.setEnabled(enabled);
        pilot2.setEnabled(enabled);
        pilot3.setEnabled(enabled);
        pilot4.setEnabled(enabled);
        pilot5.setEnabled(enabled);
        pilot6.setEnabled(enabled);
        otherPilot.setEnabled(enabled);
        disablePilot.setEnabled(enabled);
    }
    
    protected void runData()
    {
        ChassisDataMsg data;
        float vx, vy;
        float omega;
        float battery;

        data = chassis.getData();

        if (data != null)
        {
            vx = data.vx;
            vy = data.vy;
            omega = data.omega;
            battery = data.battery;

            if (data.activePilot == RackName.create(RackName.PILOT, 0))
            {
                pilot0.setSelected(true);
            }
            else if (data.activePilot == RackName.create(RackName.PILOT, 1))
            {
                pilot1.setSelected(true);
            }
            else if (data.activePilot == RackName.create(RackName.PILOT, 2))
            {
                pilot2.setSelected(true);
            }
            else if (data.activePilot == RackName.create(RackName.PILOT, 3))
            {
                pilot3.setSelected(true);
            }
            else if (data.activePilot == RackName.create(RackName.PILOT, 4))
            {
                pilot4.setSelected(true);
            }
            else if (data.activePilot == RackName.create(RackName.PILOT, 5))
            {
                pilot5.setSelected(true);
            }
            else if (data.activePilot == RackName.create(RackName.PILOT, 6))
            {
                pilot6.setSelected(true);
            }

            else if (data.activePilot < 0)
            {
                disablePilot.setSelected(true);
            }
            else
            {
                otherPilot.setSelected(true);
                otherPilot.setText("0x" + Integer.toHexString(data.activePilot));
            }
            vxLabel.setText(Math.rint(Math.round(vx / 10.0)) / 100.0 + " m/s");
            vyLabel.setText(Math.rint(Math.round(vy / 10.0)) / 100.0 + " m/s");
            omegaLabel.setText(Math.rint(Math.toDegrees(omega) * 100.0) / 100.0 + " deg/s");
            batteryLabel.setText(battery + "");
            
            setEnabled(true);
            
            if (synth != null)
            {
                // calculate note in octave
                int newNote = (int)(24.0 + vx * 12 / vxMax);
                if (vx < 0) newNote *= -1;
            
                if (currentNote != newNote)
                {
            	    midiChannels[0].noteOn(newNote, 127);
            	    midiChannels[0].noteOff(currentNote);
            	    currentNote = newNote;
                }
            }
        }
        else
        {
            setEnabled(false);
            
            if (synth != null)
            {
                midiChannels[0].noteOff(currentNote, 127);
                currentNote = 0;
            }
        }
    }
    
    protected void runStop()
    {
        disablePilot.removeActionListener(disablePilotAction);
        pilot0.removeActionListener(pilot0Action);
        pilot1.removeActionListener(pilot1Action);
        pilot2.removeActionListener(pilot2Action);
        pilot3.removeActionListener(pilot3Action);
        pilot4.removeActionListener(pilot4Action);
        pilot5.removeActionListener(pilot5Action);
        pilot6.removeActionListener(pilot6Action);
        
		if (synth != null) synth.close();
    }
}
