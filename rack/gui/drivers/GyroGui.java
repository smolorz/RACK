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
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.RackModuleGui;
import rack.drivers.GyroDataMsg;
import rack.drivers.GyroProxy;

public class GyroGui extends RackModuleGui
{
    protected JLabel        rollLabel       = new JLabel("-000 deg");
    protected JLabel        pitchLabel      = new JLabel("-000 deg");
    protected JLabel        yawLabel        = new JLabel("-000 deg");
    protected JLabel        wRollLabel      = new JLabel("-000.0 deg/s");
    protected JLabel        wPitchLabel     = new JLabel("-000.0 deg/s");
    protected JLabel        wYawLabel       = new JLabel("-000.0 deg/s");
    protected JLabel        aXLabel         = new JLabel("-0.00 m/ss");
    protected JLabel        aYLabel         = new JLabel("-0.00 m/ss");
    protected JLabel        aZLabel         = new JLabel("-0.00 m/ss");
    protected JLabel        rollNameLabel   = new JLabel("Roll", SwingConstants.RIGHT);
    protected JLabel        pitchNameLabel  = new JLabel("Pitch", SwingConstants.RIGHT);
    protected JLabel        yawNameLabel    = new JLabel("Yaw", SwingConstants.RIGHT);
    protected JLabel        wRollNameLabel  = new JLabel("w Roll", SwingConstants.RIGHT);
    protected JLabel        wPitchNameLabel = new JLabel("w Pitch", SwingConstants.RIGHT);
    protected JLabel        wYawNameLabel   = new JLabel("w Yaw", SwingConstants.RIGHT);
    protected JLabel        aXNameLabel     = new JLabel("a X", SwingConstants.RIGHT);
    protected JLabel        aYNameLabel     = new JLabel("a Y", SwingConstants.RIGHT);
    protected JLabel        aZNameLabel     = new JLabel("a Z", SwingConstants.RIGHT);
    protected GyroComponent gyroComponent;
    protected GyroProxy     gyro;

    public GyroGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        gyro = (GyroProxy) proxy;

        updateTime = 200;

        JPanel buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));
        JPanel labelPanel = new JPanel(new GridLayout(0, 2, 8, 0));
        JPanel southPanel = new JPanel(new BorderLayout(2, 2));

        gyroComponent = new GyroComponent();

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);

        labelPanel.add(rollNameLabel);
        labelPanel.add(rollLabel);
        labelPanel.add(pitchNameLabel);
        labelPanel.add(pitchLabel);
        labelPanel.add(yawNameLabel);
        labelPanel.add(yawLabel);
        labelPanel.add(new JLabel());
        labelPanel.add(new JLabel());
        labelPanel.add(wRollNameLabel);
        labelPanel.add(wRollLabel);
        labelPanel.add(wPitchNameLabel);
        labelPanel.add(wPitchLabel);
        labelPanel.add(wYawNameLabel);
        labelPanel.add(wYawLabel);
        labelPanel.add(new JLabel());
        labelPanel.add(new JLabel());
        labelPanel.add(aXNameLabel);
        labelPanel.add(aXLabel);
        labelPanel.add(aYNameLabel);
        labelPanel.add(aYLabel);
        labelPanel.add(aZNameLabel);
        labelPanel.add(aZLabel);
        southPanel.add(labelPanel, BorderLayout.CENTER);

        rootPanel.add(buttonPanel, BorderLayout.NORTH);
        rootPanel.add(gyroComponent, BorderLayout.CENTER);
        rootPanel.add(southPanel, BorderLayout.SOUTH);

        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        rollNameLabel.setEnabled(enabled);
        pitchNameLabel.setEnabled(enabled);
        yawNameLabel.setEnabled(enabled);
        wRollNameLabel.setEnabled(enabled);
        wPitchNameLabel.setEnabled(enabled);
        wYawNameLabel.setEnabled(enabled);
        aXNameLabel.setEnabled(enabled);
        aYNameLabel.setEnabled(enabled);
        aZNameLabel.setEnabled(enabled);

        rollLabel.setEnabled(enabled);
        pitchLabel.setEnabled(enabled);
        yawLabel.setEnabled(enabled);
        wRollLabel.setEnabled(enabled);
        wPitchLabel.setEnabled(enabled);
        wYawLabel.setEnabled(enabled);
        aXLabel.setEnabled(enabled);
        aYLabel.setEnabled(enabled);
        aZLabel.setEnabled(enabled);
    }

    protected void runData()
    {
        GyroDataMsg data;
        double roll;
        double pitch;
        double yaw;
        double wRoll;
        double wPitch;
        double wYaw;
        double aX;
        double aY;
        double aZ;

        data = gyro.getData();

        if (data != null)
        {
            gyroComponent.updateData(data);

            roll = Math.rint(Math.toDegrees(data.roll));
            pitch = Math.rint(Math.toDegrees(data.pitch));
            yaw = Math.rint(Math.toDegrees(data.yaw));
            wRoll = Math.rint(Math.toDegrees(data.wRoll) * 10.0) / 10.0;
            wPitch = Math.rint(Math.toDegrees(data.wPitch) * 10.0) / 10.0;
            wYaw = Math.rint(Math.toDegrees(data.wYaw) * 10.0) / 10.0;
            aX = Math.rint(data.aX / 10.0) / 100.0;
            aY = Math.rint(data.aY / 10.0) / 100.0;
            aZ = Math.rint(data.aZ / 10.0) / 100.0;

            // Symetrierung auf +/- 180
            if (roll > 180.0)
            {
                roll -= 360.0;
            }

            if (pitch > 180.0)
            {
                pitch -= 360.0;
            }

            rollLabel.setText(roll + " deg");
            pitchLabel.setText(pitch + " deg");
            yawLabel.setText(yaw + " deg");
            wRollLabel.setText(wRoll + " deg/s");
            wPitchLabel.setText(wPitch + " deg/s");
            wYawLabel.setText(wYaw + " deg/s");
            aXLabel.setText(aX + " m/ss");
            aYLabel.setText(aY + " m/ss");
            aZLabel.setText(aZ + " m/ss");

            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
    }
}
