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
 *      Matthias Hentschel  <hentschel@rts.uni-hannover.de>
 *
 */
package rack.gui.drivers;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.*;
import rack.main.*;
import rack.drivers.IoDataMsg;
import rack.drivers.IoProxy;

public class IoGui extends RackModuleGui implements MapViewInterface
{
    protected IoProxy         ioProxy;

    protected JLabel          valueNumNameLabel   = new JLabel("Value Num", SwingConstants.RIGHT);
    protected JLabel          valueIndexNameLabel = new JLabel("Value Index", SwingConstants.RIGHT);
    protected JLabel          valueNameLabel      = new JLabel("Value", SwingConstants.RIGHT);

    protected JLabel          valueNumLabel      = new JLabel("0");
    protected JLabel          valueIndexLabel    = new JLabel("0");
    protected JLabel          valueLabel         = new JLabel("0");

    protected JButton         setDataButton      = new JButton("Set data");
    protected JSpinner	      ioIndexSpinner     = new JSpinner();

    protected JTextField      digOutText         = new JTextField("00");
    //protected

    protected boolean         mapViewIsShowing   = false;
    protected MapViewGui      mapViewGui;
    protected byte            byteOut            = 00;

    public IoGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        ioProxy = (IoProxy) proxy;

        JPanel northPanel = new JPanel(new BorderLayout(2,2));
        JPanel buttonPanel = new JPanel(new GridLayout(0,2,4,2));
        JPanel labelPanel = new JPanel(new GridLayout(0,2,8,0));
        JPanel southPanel = new JPanel(new GridLayout(0,1,4,0));

        ioIndexSpinner.setModel(new SpinnerNumberModel(0, 0, 100, 1));
        ioIndexSpinner.addChangeListener(new ChangeListener()
        {
			public void stateChanged(ChangeEvent e)
			{
			}
		});

        ActionListener setDataAction = new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                byteOut = (byte)Integer.parseInt(digOutText.getText(), 16);

                IoDataMsg data = null;
                data = ioProxy.getData();
                data.valueNum = 1;
                data.value[0] = byteOut;
                ioProxy.setData(data);
            }
        };

        digOutText.addActionListener(setDataAction);
        setDataButton.addActionListener(setDataAction);

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);

        northPanel.add(new JLabel(RackName.nameString(ioProxy.getCommandMbx())),BorderLayout.CENTER);
        northPanel.add(buttonPanel,BorderLayout.SOUTH);

        labelPanel.add(valueNumNameLabel);
        labelPanel.add(valueNumLabel);
        labelPanel.add(valueIndexNameLabel);
        labelPanel.add(valueIndexLabel);
        labelPanel.add(valueNameLabel);
        labelPanel.add(valueLabel);

        southPanel.add(ioIndexSpinner,BorderLayout.EAST);
        southPanel.add(setDataButton,BorderLayout.WEST);
        southPanel.add(digOutText,BorderLayout.SOUTH);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(labelPanel, BorderLayout.CENTER);
        rootPanel.add(southPanel, BorderLayout.SOUTH);
        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        valueNumNameLabel.setEnabled(enabled);
        valueIndexNameLabel.setEnabled(enabled);
        valueNameLabel.setEnabled(enabled);

        valueNumLabel.setEnabled(enabled);
        valueIndexLabel.setEnabled(enabled);
        valueLabel.setEnabled(enabled);
    }

    protected void runStart()
    {
    }

    protected void runStop()
    {
    }

    protected boolean needsRunData()
    {
        return (super.needsRunData() || mapViewIsShowing);
    }

    protected void runData()
    {
        IoDataMsg data;
        int valueIndex;

        data = ioProxy.getData();

        if(data != null)
        {
            valueIndex = ((SpinnerNumberModel) ioIndexSpinner.getModel()).getNumber().intValue();
            if (data.valueNum > valueIndex)
            {
                valueNumLabel.setText(data.valueNum + "");
                valueIndexLabel.setText(valueIndex + "");
                valueLabel.setText(data.value[valueIndex] + "");
            }
            else
            {
                valueNumLabel.setText("0");
                valueIndexLabel.setText(valueIndex+"");
                valueLabel.setText("0");
            }

            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
        mapViewIsShowing = false;
    }

    public void mapViewActionPerformed(MapViewActionEvent event)
    {
    }

    public synchronized void paintMapView(MapViewGraphics mvg)
    {
    }
}
