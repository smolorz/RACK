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
import rack.gui.GuiElementDescriptor;
import rack.gui.main.RackModuleGui;

public class CameraGui extends RackModuleGui
{
    protected JButton         storeButton;
    protected JButton         storeContOnButton;
    protected JButton         storeContOffButton;
    protected int             contStoring               = 0;
    protected int             singleStoring             = 0;
    protected ActionListener  storeAction;
    protected ActionListener  storeContOnAction;
    protected ActionListener  storeContOffAction;

    protected CameraComponent cameraComponent;

    public JLabel             mousePositionLabel;
    public JLabel             contStoringLabel;
    protected JPanel          labelPanel;
    protected JPanel          buttonPanel;
    protected JPanel          switchBoxPanel;
    protected JComboBox       switchRotateComboBox;
    protected String[]        switchRotateRates         = { "none", "90 deg right", "hor flip", "90 deg and hor flip" };
    protected int             switchRotate;
    protected ActionListener  switchRotateAction;
    protected JComboBox       zoomRateComboBox;
    protected String[]        possibleZoomRates         = { "25", "50", "100", "200", "300", "400" };
    protected int             zoomRate;
    protected ActionListener  zoomRateAction;
    protected JComboBox       resolutionComboBox;
    protected String[]        possibleResolutions       = { "160x120", "320x240", "352x288", "640x480", "1280x960" };
    protected int[]           possibleResolutionsWidth  = { 160, 320, 352, 640, 1280 };
    protected int[]           possibleResolutionsHeight = { 120, 240, 288, 480, 960 };
    protected ActionListener  resolutionAction;
    protected JComboBox       modeComboBox;
    protected String[]        possibleModes             = { "MONO8", "MONO12", "MONO16", "RGB24", "RGB565", "YUV422",
            "JPEG","RAW8", "RAW12", "RAW16", "RANGE", "INTENS", "TP_INT", "RANGE_TP", "SEGMENT", "ELEVA", "TYPE", "EDGE" };
    protected int[]           possibleModesNum          = { CameraDataMsg.CAMERA_MODE_MONO8,
            CameraDataMsg.CAMERA_MODE_MONO12, CameraDataMsg.CAMERA_MODE_MONO16, CameraDataMsg.CAMERA_MODE_RGB24,
            CameraDataMsg.CAMERA_MODE_RGB565, CameraDataMsg.CAMERA_MODE_YUV422, CameraDataMsg.CAMERA_MODE_JPEG, CameraDataMsg.CAMERA_MODE_RAW8,
            CameraDataMsg.CAMERA_MODE_RAW12, CameraDataMsg.CAMERA_MODE_RAW16, CameraDataMsg.CAMERA_MODE_RANGE,
            CameraDataMsg.CAMERA_MODE_INTENSITY, CameraDataMsg.CAMERA_MODE_TYPE_INTENSITY,
            CameraDataMsg.CAMERA_MODE_RANGE_TYPE, CameraDataMsg.CAMERA_MODE_SEGMENT,
            CameraDataMsg.CAMERA_MODE_ELEVATION, CameraDataMsg.CAMERA_MODE_TYPE, CameraDataMsg.CAMERA_MODE_EDGE };
    protected ActionListener  modeAction;

    protected JComboBox switchColorComboBox;
    protected String[] switchColorRates = { "standart", "Gray 0-63","Gray 0-127", "Gray 0-255", "Gray 0-1023", 
    		               "Gray 0-4095", "Miscolor range indoor", "Miscolor range outdoor", "Gray 4095 - 0", 
    		               "Thermal spectrum", "Thermal red"};
    protected int switchColor; 

    
    public CameraProxy        camera;

    public CameraGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);
        
        camera = (CameraProxy) proxy;

        updateTime = 200;

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));

        buttonPanel = new JPanel(new GridLayout(0, 3, 4, 2));
        switchBoxPanel = new JPanel(new GridLayout(0, 4, 4, 2));
        labelPanel = new JPanel(new GridLayout(0, 2, 4, 2));

        storeButton = new JButton("Store");
        storeContOnButton = new JButton("StoreOn");
        storeContOffButton = new JButton("StoreOff");

        storeContOnAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                contStoring = 1;
                contStoringLabel.setText("Cont.storing on.");

            }
        };
        storeContOnButton.addActionListener(storeContOnAction);

        storeContOffAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                contStoring = 0;
                contStoringLabel.setText("Cont.storing off.");

            }
        };
        storeContOffButton.addActionListener(storeContOffAction);

        storeAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                singleStoring = 1;
            }
        };
        storeButton.addActionListener(storeAction);

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);
        buttonPanel.add(storeButton);
        buttonPanel.add(storeContOnButton);
        buttonPanel.add(storeContOffButton);

        northPanel.add(buttonPanel, BorderLayout.NORTH);

        switchColorComboBox = new JComboBox(switchColorRates);
        switchColorComboBox.setSelectedIndex(0);

        switchColorComboBox.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
            	switchColor = switchColorComboBox.getSelectedIndex();
            	CameraMisColorConvert a = null;
            	switch (switchColor) {
            	case 1:
            		a = new CameraMisColorConvert();
            		a.initTable();
            		a.addColor(0, Color.BLACK);
            		a.addColor(63, Color.WHITE);
            		a.addColor(0xffff, Color.RED);
            		a.calcTable();
            		cameraComponent.setColorConverter(a);
            		break;
            	case 2:
            		a = new CameraMisColorConvert();
            		a.initTable();
            		a.addColor(0, Color.BLACK);
            		a.addColor(127, Color.WHITE);
            		a.addColor(0xffff, Color.RED);
            		a.calcTable();
            		cameraComponent.setColorConverter(a);
            		break;
            	case 3:
            		a = new CameraMisColorConvert();
            		a.initTable();
            		a.addColor(0, Color.BLACK);
            		a.addColor(255, Color.WHITE);
            		a.addColor(0xffff, Color.RED);
            		a.calcTable();
            		cameraComponent.setColorConverter(a);
            		break;
            	case 4:
            		a = new CameraMisColorConvert();
            		a.initTable();
            		a.addColor(0, Color.BLACK);
            		a.addColor(1023, Color.WHITE);
            		a.addColor(0xffff, Color.RED);
            		a.calcTable();
            		cameraComponent.setColorConverter(a);
            		break;          		
            	case 5:
            		a = new CameraMisColorConvert();
            		a.initTable();
            		a.addColor(0, Color.BLACK);
            		a.addColor(4095, Color.WHITE);
            		a.addColor(0xffff, Color.RED);
            		a.calcTable();
            		cameraComponent.setColorConverter(a);
            		break;          		
            	case 6:
            		a = new CameraMisColorConvert();
            		a.initTable();
            		a.addColor(0, Color.BLACK);
            		a.addColor(1000, Color.BLUE);
            		a.addColor(2000, Color.GREEN);
            		a.addColor(4000, Color.CYAN);
            		a.addColor(6000, Color.YELLOW);
            		a.addColor(0xfffe, Color.WHITE);
            		a.addColor(0xffff, Color.RED);
            		a.calcTable();
            		cameraComponent.setColorConverter(a);
            		break;
            	case 7:
            		a = new CameraMisColorConvert();
            		a.initTable();
            		a.addColor(0, Color.BLACK);
            		a.addColor(3000, Color.BLUE);
            		a.addColor(6000, Color.GREEN);
            		a.addColor(9000, Color.CYAN);
            		a.addColor(15000, Color.YELLOW);
            		a.addColor(0xfffe, Color.WHITE);
            		a.addColor(0xffff, Color.RED);
            		a.calcTable();
            		cameraComponent.setColorConverter(a);
            		break;
            	case 8:
            		a = new CameraMisColorConvert();
            		a.initTable();
            		a.addColor(4095, Color.BLACK);
            		a.addColor(0, Color.WHITE);
            		a.addColor(0xffff, Color.RED);
            		a.calcTable();
            		cameraComponent.setColorConverter(a);
            		break;          		
            	case 9:
            		a = new CameraMisColorConvert();
            		a.initTable();
            		a.addColor(0, Color.BLACK);
            		a.addColor(48, Color.BLUE);
            		a.addColor(96, Color.CYAN);
            		a.addColor(154, Color.GREEN);
            		a.addColor(202, Color.YELLOW);
            		a.addColor(250, Color.RED);
            		a.addColor(0xffff, Color.WHITE);
            		a.calcTable();
            		cameraComponent.setColorConverter(a);
            		break;
            	case 10:
            		a = new CameraMisColorConvert();
            		a.initTable();
            		a.addColor(0, Color.BLACK);
            		a.addColor(48, Color.BLUE);
            		a.addColor(154, Color.RED);
            		a.addColor(252, Color.ORANGE);
            		a.addColor(0xffff, Color.WHITE);
            		a.calcTable();
            		cameraComponent.setColorConverter(a);
            		break;
            	default:
            		cameraComponent.setColorConverter(null);
            		break;
            	}
            }
        });        

        
        switchRotateComboBox = new JComboBox(switchRotateRates);
        switchRotateComboBox.setSelectedIndex(0);

        switchRotateAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                switchRotate = switchRotateComboBox.getSelectedIndex();
            }
        };
        switchRotateComboBox.addActionListener(switchRotateAction);

        zoomRateComboBox = new JComboBox(possibleZoomRates);
        zoomRateComboBox.setSelectedIndex(2);

        zoomRateAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                int boxValue = new Integer(possibleZoomRates[zoomRateComboBox.getSelectedIndex()]).intValue();
                if (boxValue >= 100)
                {
                    zoomRate = boxValue / 100;
                }
                else
                {
                    zoomRate = (-1) * 100 / boxValue;
                }
            }
        };
        zoomRateComboBox.addActionListener(zoomRateAction);

        resolutionComboBox = new JComboBox(possibleResolutions);
        resolutionComboBox.setSelectedIndex(2);

        resolutionAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                int width = possibleResolutionsWidth[resolutionComboBox.getSelectedIndex()];
                int height = possibleResolutionsHeight[resolutionComboBox.getSelectedIndex()];
                camera.setFormat(new CameraFormatMsg(width, height));
            }
        };
        resolutionComboBox.addActionListener(resolutionAction);

        modeComboBox = new JComboBox(possibleModes);
        modeComboBox.setSelectedIndex(1);

        modeAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                int mode = possibleModesNum[modeComboBox.getSelectedIndex()];
                camera.setFormat(new CameraFormatMsg(mode));
            }
        };
        modeComboBox.addActionListener(modeAction);

        switchBoxPanel.add(switchColorComboBox);
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

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(scrollPane, BorderLayout.CENTER);
    }

    protected void runData()
    {
        CameraDataMsg data;

        data = camera.getData();

        if (data != null)
        {
            cameraComponent.transformImage(zoomRate, switchRotate, data);
            if ((contStoring == 1) || (singleStoring == 1))
            {
                data.storeDataToFile("camera" + System.currentTimeMillis() + ".png");
                singleStoring = 0;
            }
        }
        else
        {
            // cameraComponent.img = null;
            // cameraComponent.repaint();
            try
            {
                Thread.sleep(1000);
            }
            catch (InterruptedException e)
            {
            }
        }
    }
    
    protected void runStop()
    {
        cameraComponent.removeListener();
        cameraComponent = null;
        
        storeButton.removeActionListener(storeAction);
        storeContOnButton.removeActionListener(storeContOnAction);
        storeContOffButton.removeActionListener(storeContOffAction);

        storeAction = null;
        storeContOnAction = null;
        storeContOffAction = null;
        
        switchRotateComboBox.removeActionListener(switchRotateAction);
        zoomRateComboBox.removeActionListener(zoomRateAction);
        resolutionComboBox.removeActionListener(resolutionAction);
        modeComboBox.removeActionListener(modeAction);

        switchRotateAction = null;
        zoomRateAction = null;
        resolutionAction = null;
        modeAction = null;
    }
}
