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
import java.awt.event.*;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.RackModuleGui;
import rack.main.*;
import rack.drivers.PtzDriveDataMsg;
import rack.drivers.PtzDriveMoveVelMsg;
import rack.drivers.PtzDriveMovePosMsg;
import rack.drivers.PtzDriveProxy;

public class PtzDriveGui extends RackModuleGui
{
    protected JButton         homeButton;
    protected JButton         gotoVelButton;
    protected JLabel          gotoVelPanLabel		= new JLabel("vel Pan", SwingConstants.LEFT);
    protected JLabel          gotoVelTiltLabel		= new JLabel("vel Tilt", SwingConstants.LEFT);
    protected JLabel          gotoVelZoomLabel		= new JLabel("vel Zoom", SwingConstants.LEFT);
    protected JTextField      gotoVelPanField;
    protected JTextField      gotoVelTiltField;
    protected JTextField      gotoVelZoomField;

    protected JButton         gotoPosButton;
    protected JLabel          gotoPosPanLabel		= new JLabel("pos Pan", SwingConstants.LEFT);
    protected JLabel          gotoPosTiltLabel		= new JLabel("pos Tilt", SwingConstants.LEFT);
    protected JLabel          gotoPosZoomLabel		= new JLabel("pos Zoom", SwingConstants.LEFT);
    protected JTextField      gotoPosPanField;
    protected JTextField      gotoPosTiltField;
    protected JTextField      gotoPosZoomField;

    protected JLabel          posPanLabel      = new JLabel("-000.00 deg");
    protected JLabel          posTiltLabel     = new JLabel("-000.00 deg");
    protected JLabel          posZoomLabel     = new JLabel("-000.00");
    protected JLabel          posPanNameLabel  = new JLabel("pos Pan", SwingConstants.LEFT);
    protected JLabel          posTiltNameLabel = new JLabel("pos Tilt", SwingConstants.LEFT);
    protected JLabel          posZoomNameLabel = new JLabel("pos Zoom", SwingConstants.LEFT);
    protected PtzDriveProxy   ptzDrive;

    public PtzDriveGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        ptzDrive = (PtzDriveProxy) proxy;

        JPanel buttonPanel = new JPanel(new GridLayout(0, 7, 4, 2));

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);
        homeButton       = new JButton("home");
        gotoVelPanField  = new JTextField("");
        gotoVelTiltField = new JTextField("");
        gotoVelZoomField = new JTextField("");
        gotoPosPanField  = new JTextField("");
        gotoPosTiltField = new JTextField("");
        gotoPosZoomField = new JTextField("");
        gotoVelButton    = new JButton("goto");
        gotoPosButton    = new JButton("goto");

        homeButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                int modifiers = e.getModifiers() & (ActionEvent.CTRL_MASK | ActionEvent.SHIFT_MASK);
                if (modifiers == (ActionEvent.CTRL_MASK | ActionEvent.SHIFT_MASK))
                {
                    ptzDrive.home();
                }
            }
        });

        gotoPosButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                ptzDrive.getData();
                int modifiers = e.getModifiers() & (ActionEvent.CTRL_MASK | ActionEvent.SHIFT_MASK);
                if (modifiers == ActionEvent.CTRL_MASK)
                {
                    try
                    {
                        PtzDriveMovePosMsg data = new PtzDriveMovePosMsg();
                        data.posPan  = Float.parseFloat(gotoPosPanField.getText());
                        data.posTilt = Float.parseFloat(gotoPosTiltField.getText());
                        data.posZoom = Float.parseFloat(gotoPosZoomField.getText());
                        ptzDrive.movePos(data);
                    }
                    catch (NumberFormatException nfe)
                    {
                    }
                }
            }
        });

        gotoVelButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                int modifiers = e.getModifiers() & (ActionEvent.CTRL_MASK | ActionEvent.SHIFT_MASK);
                if (modifiers == ActionEvent.CTRL_MASK)
                {
                    try
                    {
                    	PtzDriveMoveVelMsg data = new PtzDriveMoveVelMsg();
                        data.velPan  = Float.parseFloat(gotoVelPanField.getText());
                        data.velTilt = Float.parseFloat(gotoVelTiltField.getText());
                        data.velZoom = Float.parseFloat(gotoVelZoomField.getText());
                        ptzDrive.moveVel(data);
                    }
                    catch (NumberFormatException nfe)
                    {
                    }
                }
            }
        });


        buttonPanel.add(homeButton);
        buttonPanel.add(new JLabel(""));
        buttonPanel.add(new JLabel(""));
        buttonPanel.add(new JLabel(""));
        buttonPanel.add(new JLabel(""));
        buttonPanel.add(posPanLabel);
        buttonPanel.add(posPanNameLabel);
        buttonPanel.add(posTiltLabel);
        buttonPanel.add(posTiltNameLabel);
        buttonPanel.add(posZoomLabel);
        buttonPanel.add(posZoomNameLabel);

        buttonPanel.add(new JLabel(""));
        buttonPanel.add(gotoVelPanField);
        buttonPanel.add(gotoVelPanLabel);
        buttonPanel.add(gotoVelTiltField);
        buttonPanel.add(gotoVelTiltLabel);
        buttonPanel.add(gotoVelZoomField);
        buttonPanel.add(gotoVelZoomLabel);
        buttonPanel.add(gotoVelButton);

        buttonPanel.add(gotoPosPanField);
        buttonPanel.add(gotoPosPanLabel);
        buttonPanel.add(gotoPosTiltField);
        buttonPanel.add(gotoPosTiltLabel);
        buttonPanel.add(gotoPosZoomField);
        buttonPanel.add(gotoPosZoomLabel);
        buttonPanel.add(gotoPosButton);

        rootPanel.add(new JLabel(RackName.nameString(ptzDrive.getCommandMbx())), BorderLayout.NORTH);
        rootPanel.add(buttonPanel, BorderLayout.CENTER);

        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        homeButton.setEnabled(enabled);
        gotoVelButton.setEnabled(enabled);
        gotoVelPanLabel.setEnabled(enabled);
        gotoVelTiltLabel.setEnabled(enabled);
        gotoVelZoomLabel.setEnabled(enabled);
        gotoVelPanField.setEnabled(enabled);
        gotoVelTiltField.setEnabled(enabled);
        gotoVelZoomField.setEnabled(enabled);

        gotoPosButton.setEnabled(enabled);
        gotoPosPanLabel.setEnabled(enabled);
        gotoPosTiltLabel.setEnabled(enabled);
        gotoPosZoomLabel.setEnabled(enabled);
        gotoPosPanField.setEnabled(enabled);
        gotoPosTiltField.setEnabled(enabled);
        gotoPosZoomField.setEnabled(enabled);

        posPanLabel.setEnabled(enabled);
        posTiltLabel.setEnabled(enabled);
        posZoomLabel.setEnabled(enabled);
        posPanNameLabel.setEnabled(enabled);
        posTiltNameLabel.setEnabled(enabled);
        posZoomNameLabel.setEnabled(enabled);
    }

    protected void runData()
    {
        PtzDriveDataMsg data;

        data = ptzDrive.getData();

        if (data != null)
        {
            posPanLabel.setText(Math.rint(Math.toDegrees(data.posPan)*100.0)/100.0   + " deg");
            posTiltLabel.setText(Math.rint(Math.toDegrees(data.posTilt)*100.0)/100.0 + " deg");
            posZoomLabel.setText(Math.rint(Math.toDegrees(data.posZoom)*100.0)/100.0 + " ");
            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
    }
}
