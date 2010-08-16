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
import javax.swing.event.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.RackModuleGui;
import rack.drivers.VehicleDataMsg;
import rack.drivers.VehicleSetValueMsg;
import rack.drivers.VehicleProxy;

public class VehicleGui extends RackModuleGui
{
    protected JButton      setValueButton;

    protected JSlider      clutchSlider;
    protected JSlider      brakeSlider;
    protected JSlider      throttleSlider;
    protected JSlider      steeringSlider;

    protected JTextField   gearSetValField;
    protected JCheckBox    engineCheckBox;
    protected JCheckBox    parkBrakeCheckBox;
    protected JCheckBox    vehicleProtectCheckBox;

    protected JLabel       clutchNameLabel;
    protected JLabel       clutchActValLabel;
    protected JLabel       clutchSetValLabel;
    protected JLabel       brakeActValLabel;
    protected JLabel       brakeSetValLabel;
    protected JLabel       throttleActValLabel;
    protected JLabel       throttleSetValLabel;
    protected JLabel       steeringActValLabel;
    protected JLabel       steeringSetValLabel;

    protected JLabel       gearNameLabel;
    protected JLabel       gearActValLabel;
    protected JLabel       engineNameLabel;
    protected JLabel       engineActValLabel;
    protected JLabel       parkBrakeNameLabel;
    protected JLabel       parkBrakeActValLabel;
    protected JLabel       vehicleProtectNameLabel;
    protected JLabel       vehicleProtectActValLabel;
    protected JLabel       activeControllerNameLabel;
    protected JLabel       activeControllerActValLabel;
    protected JLabel       speedNameLabel;
    protected JLabel       speedActValLabel;
    protected JLabel       omegaNameLabel;
    protected JLabel       omegaActValLabel;

    protected VehicleProxy vehicle;

    public VehicleGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        vehicle = (VehicleProxy) proxy;

        rootPanel = new JPanel(new BorderLayout(4, 25));
        rootPanel.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));
        JPanel buttonPanel = new JPanel(new GridLayout(0, 3, 4, 2));
        JPanel clutchPanel = new JPanel(new BorderLayout(2, 2));
        JPanel brakePanel = new JPanel(new BorderLayout(2, 2));
        JPanel throttlePanel = new JPanel(new BorderLayout(2, 2));
        JPanel miscPanel = new JPanel(new GridLayout(8, 3, 0, 0));
        JPanel steeringPanel = new JPanel(new BorderLayout(2, 2));
        JPanel sliderPanel = new JPanel(new GridLayout(0, 4, 0, 4));

        onButton = new JButton("On");
        onButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                vehicle.on();

                // init values
                clutchSlider.setValue(0);
                clutchSetValLabel.setText("" + clutchSlider.getValue());

                brakeSlider.setValue(0);
                brakeSetValLabel.setText("" + brakeSlider.getValue());

                throttleSlider.setValue(0);
                throttleSetValLabel.setText("" + throttleSlider.getValue());

                steeringSlider.setValue(0);
                steeringSetValLabel.setText("" + steeringSlider.getValue());

                gearSetValField.setText("0");
                engineCheckBox.setEnabled(false);
                parkBrakeCheckBox.setEnabled(false);
                vehicleProtectCheckBox.setEnabled(false);
            }
        });

        offButton = new JButton("Off");
        offButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                vehicle.off();
            }
        });

        setValueButton = new JButton("Set value");
        setValueButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                int modifiers = e.getModifiers() & (ActionEvent.CTRL_MASK | ActionEvent.SHIFT_MASK);

                if (modifiers == ActionEvent.CTRL_MASK)
                {
                    VehicleSetValueMsg data;
                    data = new VehicleSetValueMsg();

                    data.clutch = (float) clutchSlider.getValue();
                    data.brake = (float) brakeSlider.getValue();
                    data.throttle = (float) throttleSlider.getValue();
                    data.steering = (float) steeringSlider.getValue();
                    data.gear = Integer.parseInt(gearSetValField.getText());

                    if (engineCheckBox.isSelected() == true)
                    {
                        data.engine = 1;
                    }
                    else
                    {
                        data.engine = 0;
                    }

                    if (parkBrakeCheckBox.isSelected() == true)
                    {
                        data.parkBrake = 1;
                    }
                    else
                    {
                        data.parkBrake = 0;
                    }

                    if (vehicleProtectCheckBox.isSelected() == true)
                    {
                        data.vehicleProtect = 1;
                    }
                    else
                    {
                        data.vehicleProtect = 0;
                    }

                    vehicle.setValue(data);
                }
            }
        });

        clutchSlider = new JSlider(JSlider.VERTICAL, 0, 100, 0);
        clutchSlider.setMajorTickSpacing(50);
        clutchSlider.setMinorTickSpacing(5);
        clutchSlider.setPaintTicks(true);
        clutchSlider.setPaintLabels(true);

        clutchActValLabel = new JLabel("clutch: 0 %", SwingConstants.CENTER);
        clutchSetValLabel = new JLabel("0 %", SwingConstants.CENTER);

        clutchSlider.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e)
            {
                JSlider source = (JSlider) e.getSource();
                if (source.getValueIsAdjusting())
                {
                    clutchSetValLabel.setText((int) source.getValue() + " %");
                }
            }
        });

        brakeSlider = new JSlider(JSlider.VERTICAL, 0, 100, 0);
        brakeSlider.setMajorTickSpacing(50);
        brakeSlider.setMinorTickSpacing(5);
        brakeSlider.setPaintTicks(true);
        brakeSlider.setPaintLabels(true);

        brakeActValLabel = new JLabel("brake: 0 %", SwingConstants.CENTER);
        brakeSetValLabel = new JLabel("0 %", SwingConstants.CENTER);

        brakeSlider.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e)
            {
                JSlider source = (JSlider) e.getSource();
                if (source.getValueIsAdjusting())
                {
                    brakeSetValLabel.setText((int) source.getValue() + " %");
                }
            }
        });

        throttleSlider = new JSlider(JSlider.VERTICAL, 0, 100, 0);
        throttleSlider.setMajorTickSpacing(50);
        throttleSlider.setMinorTickSpacing(5);
        throttleSlider.setPaintTicks(true);
        throttleSlider.setPaintLabels(true);

        throttleActValLabel = new JLabel("throttle: 0 %", SwingConstants.CENTER);
        throttleSetValLabel = new JLabel("0 %", SwingConstants.CENTER);

        throttleSlider.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e)
            {
                JSlider source = (JSlider) e.getSource();
                if (source.getValueIsAdjusting())
                {
                    throttleSetValLabel.setText((int) source.getValue() + " %");
                }
            }
        });

        steeringSlider = new JSlider(JSlider.HORIZONTAL, -100, 100, 0);
        steeringSlider.setMajorTickSpacing(50);
        steeringSlider.setMinorTickSpacing(5);
        steeringSlider.setPaintTicks(true);
        steeringSlider.setPaintLabels(true);

        steeringActValLabel = new JLabel("steering: 0 %", SwingConstants.CENTER);
        steeringSetValLabel = new JLabel("0 %", SwingConstants.CENTER);

        steeringSlider.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e)
            {
                JSlider source = (JSlider) e.getSource();
                if (source.getValueIsAdjusting())
                {
                    steeringSetValLabel.setText((int) source.getValue() + " %");
                }
            }
        });

        gearNameLabel = new JLabel("gear: ", SwingConstants.LEFT);
        gearActValLabel = new JLabel("0 ", SwingConstants.LEFT);
        gearSetValField = new JTextField("0");
        engineNameLabel = new JLabel("engine: ", SwingConstants.LEFT);
        engineActValLabel = new JLabel("0 ", SwingConstants.LEFT);
        engineCheckBox = new JCheckBox("", false);
        parkBrakeNameLabel = new JLabel("park brake: ", SwingConstants.LEFT);
        parkBrakeActValLabel = new JLabel("0 ", SwingConstants.LEFT);
        parkBrakeCheckBox = new JCheckBox("", false);
        vehicleProtectNameLabel = new JLabel("protection: ", SwingConstants.LEFT);
        vehicleProtectActValLabel = new JLabel("0 ", SwingConstants.LEFT);
        vehicleProtectCheckBox = new JCheckBox("", false);
        activeControllerNameLabel = new JLabel("act. control: ", SwingConstants.LEFT);
        activeControllerActValLabel = new JLabel("0 ", SwingConstants.LEFT);
        speedNameLabel = new JLabel("speed: ", SwingConstants.LEFT);
        speedActValLabel = new JLabel("0 m/s", SwingConstants.LEFT);
        omegaNameLabel = new JLabel("omega: ", SwingConstants.LEFT);
        omegaActValLabel = new JLabel("0.0 deg/s", SwingConstants.LEFT);

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);
        buttonPanel.add(setValueButton);
        northPanel.add(new JLabel("vehicle"), BorderLayout.CENTER);
        northPanel.add(buttonPanel, BorderLayout.SOUTH);

        clutchPanel.add(clutchActValLabel, BorderLayout.NORTH);
        clutchPanel.add(clutchSlider, BorderLayout.CENTER);
        clutchPanel.add(clutchSetValLabel, BorderLayout.SOUTH);

        brakePanel.add(brakeActValLabel, BorderLayout.NORTH);
        brakePanel.add(brakeSlider, BorderLayout.CENTER);
        brakePanel.add(brakeSetValLabel, BorderLayout.SOUTH);

        throttlePanel.add(throttleActValLabel, BorderLayout.NORTH);
        throttlePanel.add(throttleSlider, BorderLayout.CENTER);
        throttlePanel.add(throttleSetValLabel, BorderLayout.SOUTH);

        steeringPanel.add(steeringActValLabel, BorderLayout.NORTH);
        steeringPanel.add(steeringSlider, BorderLayout.CENTER);
        steeringPanel.add(steeringSetValLabel, BorderLayout.SOUTH);

        miscPanel.add(gearNameLabel);
        miscPanel.add(gearActValLabel);
        miscPanel.add(gearSetValField);
        miscPanel.add(engineNameLabel);
        miscPanel.add(engineActValLabel);
        miscPanel.add(engineCheckBox);
        miscPanel.add(parkBrakeNameLabel);
        miscPanel.add(parkBrakeActValLabel);
        miscPanel.add(parkBrakeCheckBox);
        miscPanel.add(vehicleProtectNameLabel);
        miscPanel.add(vehicleProtectActValLabel);
        miscPanel.add(vehicleProtectCheckBox);
        miscPanel.add(activeControllerNameLabel);
        miscPanel.add(activeControllerActValLabel);
        miscPanel.add(new JLabel(""));
        miscPanel.add(new JLabel(""));
        miscPanel.add(new JLabel(""));
        miscPanel.add(new JLabel(""));
        miscPanel.add(speedNameLabel);
        miscPanel.add(speedActValLabel);
        miscPanel.add(new JLabel(""));
        miscPanel.add(omegaNameLabel);
        miscPanel.add(omegaActValLabel);
        miscPanel.add(new JLabel(""));

        sliderPanel.add(clutchPanel);
        sliderPanel.add(brakePanel);
        sliderPanel.add(throttlePanel);
        sliderPanel.add(miscPanel);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(sliderPanel, BorderLayout.CENTER);
        rootPanel.add(steeringPanel, BorderLayout.SOUTH);

        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        setValueButton.setEnabled(enabled);

        clutchActValLabel.setEnabled(enabled);
        clutchSetValLabel.setEnabled(enabled);
        clutchSlider.setEnabled(enabled);

        brakeActValLabel.setEnabled(enabled);
        brakeSetValLabel.setEnabled(enabled);
        brakeSlider.setEnabled(enabled);

        throttleActValLabel.setEnabled(enabled);
        throttleSetValLabel.setEnabled(enabled);
        throttleSlider.setEnabled(enabled);

        steeringActValLabel.setEnabled(enabled);
        steeringSetValLabel.setEnabled(enabled);
        steeringSlider.setEnabled(enabled);

        gearNameLabel.setEnabled(enabled);
        gearActValLabel.setEnabled(enabled);
        gearSetValField.setEnabled(enabled);

        engineNameLabel.setEnabled(enabled);
        engineActValLabel.setEnabled(enabled);
        engineCheckBox.setEnabled(enabled);

        parkBrakeNameLabel.setEnabled(enabled);
        parkBrakeActValLabel.setEnabled(enabled);
        parkBrakeCheckBox.setEnabled(enabled);

        vehicleProtectNameLabel.setEnabled(enabled);
        vehicleProtectActValLabel.setEnabled(enabled);
        vehicleProtectCheckBox.setEnabled(enabled);

        activeControllerNameLabel.setEnabled(enabled);
        activeControllerActValLabel.setEnabled(enabled);

        speedNameLabel.setEnabled(enabled);
        speedActValLabel.setEnabled(enabled);

        omegaNameLabel.setEnabled(enabled);
        omegaActValLabel.setEnabled(enabled);
    }

    protected void runData()
    {
        VehicleDataMsg data;

        data = vehicle.getData();

        if (data != null)
        {
            setValueButton.setEnabled(true);

            clutchActValLabel.setText("clutch: " + data.clutch + " %");
            brakeActValLabel.setText("brake: " + data.brake + " %");
            throttleActValLabel.setText("throttle: " + data.throttle + " %");
            steeringActValLabel.setText("steering: " + data.steering + " %");
            gearActValLabel.setText("" + data.gear);
            engineActValLabel.setText("" + data.engine);
            parkBrakeActValLabel.setText("" + data.parkBrake);
            vehicleProtectActValLabel.setText("" + data.vehicleProtect);
            activeControllerActValLabel.setText("" + data.activeController);
            speedActValLabel.setText(Math.round(data.speed / 10.0) / 100.0 + " m/s");
            omegaActValLabel.setText(Math.rint(Math.toDegrees(data.omega) * 100) / 100.0 + " deg/s");

            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
    }
}
