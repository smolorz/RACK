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
 *      Marko Reimer     <reimer@l3s.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.gui.drivers;

import java.awt.*;
import java.awt.event.*;

import javax.swing.*;

import rack.drivers.CameraDataMsg;
import rack.drivers.CameraFormatMsg;
import rack.drivers.CameraProxy;
import rack.gui.main.RackModuleGui;
import rack.main.proxy.*;

public class CameraGui extends RackModuleGui
{

    public CameraGui(Integer moduleIndex, RackProxy[] proxyList, RackModuleGui[] guiList)
    {
        camera = (CameraProxy)proxyList[moduleIndex.intValue()];
        rackProxyList = proxyList;

        panel = new JPanel(new BorderLayout(2, 2));
        panel.setBorder(BorderFactory.createEmptyBorder(2, 2, 2, 2));

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));

        buttonPanel = new JPanel(new GridLayout(0, 3, 4, 2));
        switchBoxPanel = new JPanel(new GridLayout(0, 4, 4, 2));
        labelPanel = new JPanel(new GridLayout(0, 2, 4, 2));

        onButton = new JButton("On");
        offButton = new JButton("Off");
        storeButton = new JButton("Store");
        storeContOnButton = new JButton("StoreOn");
        storeContOffButton = new JButton("StoreOff");

        onButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                camera.on();
            }
        });

        offButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                camera.off();
            }
        });

        storeContOnButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                contStoring = 1;
                contStoringLabel.setText("Cont.storing on.");

            }
        });

        storeContOffButton.addActionListener(new ActionListener()
                {
                    public void actionPerformed(ActionEvent e)
                    {
                        contStoring = 0;
                        contStoringLabel.setText("Cont.storing off.");

                    }
                });
        storeButton.addActionListener(new ActionListener()
                {
                    public void actionPerformed(ActionEvent e)
                    {
                        camera.storeDataToFile("camera.png");
                    }
                });

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);
        buttonPanel.add(storeButton);
        buttonPanel.add(storeContOnButton);
        buttonPanel.add(storeContOffButton);

        northPanel.add(buttonPanel, BorderLayout.NORTH);

        switchRotateComboBox = new JComboBox(switchRotateRates);
        switchRotateComboBox.setSelectedIndex(0);

        switchRotateComboBox.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                switchRotate = switchRotateComboBox.getSelectedIndex();
            }
        });

        zoomRateComboBox = new JComboBox(possibleZoomRates);
        zoomRateComboBox.setSelectedIndex(2);

        zoomRateComboBox.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                int boxValue = new Integer(possibleZoomRates[zoomRateComboBox
                        .getSelectedIndex()]).intValue();
                if (boxValue >= 100)
                {
                    zoomRate = boxValue / 100;
                }
                else
                {
                    zoomRate = (-1) * 100 / boxValue;
                }
            }
        });

        resolutionComboBox = new JComboBox(possibleResolutions);
        resolutionComboBox.setSelectedIndex(2);

        resolutionComboBox.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                int width = possibleResolutionsWidth[resolutionComboBox
                        .getSelectedIndex()];
                int height = possibleResolutionsHeight[resolutionComboBox
                        .getSelectedIndex()];
                camera.setFormat(new CameraFormatMsg(width, height));
            }
        });

        modeComboBox = new JComboBox(possibleModes);
        modeComboBox.setSelectedIndex(1);

        modeComboBox.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                int mode = possibleModesNum[modeComboBox.getSelectedIndex()];
                camera.setFormat(new CameraFormatMsg(mode));
            }
        });

        switchBoxPanel.add(switchRotateComboBox);
        switchBoxPanel.add(zoomRateComboBox);
        switchBoxPanel.add(resolutionComboBox);
        switchBoxPanel.add(modeComboBox);
        northPanel.add(switchBoxPanel, BorderLayout.CENTER);

        mousePositionLabel = new JLabel();
        mousePositionLabel.setText("Mouse not in yet.");
        contStoringLabel = new JLabel();
        contStoringLabel.setText("Cont.storing off.");

        labelPanel.add(mousePositionLabel, BorderLayout.WEST);
        labelPanel.add(contStoringLabel, BorderLayout.EAST);

        northPanel.add(labelPanel, BorderLayout.SOUTH);

        cameraComponent = new CameraComponent();
        cameraComponent.setMousePositionLabel(mousePositionLabel);

        JScrollPane scrollPane = new JScrollPane(cameraComponent);

        panel.add(northPanel, BorderLayout.NORTH);
        panel.add(scrollPane, BorderLayout.CENTER);
    }

    public JComponent getComponent()
    {
        return (panel);
    }

    public String getModuleName()
    {
        return ("camera");
    }

    public RackProxy getProxy()
    {
        return (camera);
    }

    public void run()
    {
        CameraDataMsg data;

        while (terminate == false)
        {
            if (panel.isShowing())
            {
                data = camera.getData();

                if (data != null)
                {
                    cameraComponent
                            .transformImage(zoomRate, switchRotate, data);
                    if (contStoring == 1)
                    {
                        camera.storeDataToFile("camera"+System.currentTimeMillis()+".png", data);
                    }
                }
                else
                {
                    // cameraComponent.img = null;
                    // cameraComponent.repaint();
                    try
                    {
                        Thread.sleep(200);
                    }
                    catch (InterruptedException e)
                    {
                    }
                }
            }
            try
            {
                Thread.sleep(200);
            }
            catch (InterruptedException e)
            {
            }
        }
    }

    protected JButton onButton;
    protected JButton offButton;
    protected JButton storeButton;
    protected JButton storeContOnButton;
    protected JButton storeContOffButton;
    protected int contStoring = 0;

    protected CameraComponent cameraComponent;

    public JLabel mousePositionLabel;
    public JLabel contStoringLabel;
    protected JPanel panel;
    protected JPanel labelPanel;
    protected JPanel buttonPanel;
    protected JPanel switchBoxPanel;
    protected JComboBox switchRotateComboBox;
    protected String[] switchRotateRates =
    { "none", "90�right", "hor flip", "90� and hor flip" };
    protected int switchRotate;
    protected JComboBox zoomRateComboBox;
    protected String[] possibleZoomRates =
    { "25", "50", "100", "200", "300", "400" };
    protected int zoomRate;
    protected JComboBox resolutionComboBox;
    protected String[] possibleResolutions =
    { "160x120", "320x240", "352x288", "640x480", "1280x960" };
    protected int[] possibleResolutionsWidth =
    { 160, 320, 352, 640, 1280 };
    protected int[] possibleResolutionsHeight =
    { 120, 240, 288, 480, 960 };
    protected JComboBox modeComboBox;
    protected String[] possibleModes =
    { "MONO8", "MONO12", "MONO16", "RGB24", "RGB565", "YUV422", "RAW", "RAW12",
            "RAW16" };
    protected int[] possibleModesNum =
    { CameraDataMsg.CAMERA_MODE_MONO8,
            CameraDataMsg.CAMERA_MODE_MONO12,
            CameraDataMsg.CAMERA_MODE_MONO16,
            CameraDataMsg.CAMERA_MODE_RGB24,
            CameraDataMsg.CAMERA_MODE_RGB565,
            CameraDataMsg.CAMERA_MODE_YUV422,
            CameraDataMsg.CAMERA_MODE_RAW8,
            CameraDataMsg.CAMERA_MODE_RAW12,
            CameraDataMsg.CAMERA_MODE_RAW16 };

    public CameraProxy camera;
    protected RackProxy[] rackProxyList;
}
