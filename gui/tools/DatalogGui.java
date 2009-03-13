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
 *      Oliver Wulf        <oliver.wulf@gmx.de>
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 */
package rack.gui.tools;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.*;
import rack.main.*;
import rack.tools.DatalogProxy;
import rack.tools.DatalogLogInfo;
import rack.tools.DatalogDataMsg;

public class DatalogGui extends RackModuleGui
{
    protected JButton        logOnButton;
    protected JButton        logOffButton;

    protected JPanel         buttonPanel;
    protected JPanel         logPanel;
    protected JPanel         frequencyPanel;
    protected JPanel         namePanel;
    protected JPanel         statusPanel;
    protected JPanel         mainPanel;
    protected JPanel         pathPanel;

    protected JLabel         logLabel;
    protected JLabel         frequencyLabel;
    protected JLabel         fileNameLabel;
    protected JLabel         statusLabel;
    protected JLabel         pathNameLabel;

    protected JCheckBox[]    logCb;
    protected JTextField[]   logFrequency;
    protected JTextField[]   fileName;
    protected JRadioButton[] status;
    protected JTextField     pathName;
    
    protected int            num;
    protected int[]          moduleMbxLog;    
    protected DatalogDataMsg logStatus;
    protected DatalogDataMsg logData;
    public DatalogProxy      datalog;

    public DatalogGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        datalog = (DatalogProxy)proxy;

        datalog.initLog();
        
        logStatus = new DatalogDataMsg();
        logStatus = datalog.getLogStatus();

        if (logStatus == null)
        {
            logStatus = new DatalogDataMsg();
            logStatus.logNum = 0;
        }

        num = logStatus.logNum;

        rootPanel = new JPanel();
        rootPanel.setLayout(new BorderLayout(10, 10));

        buttonPanel = new JPanel();
        buttonPanel.setLayout(new FlowLayout(FlowLayout.CENTER, 20, 0));

        mainPanel = new JPanel();
        mainPanel.setLayout(new BoxLayout(mainPanel, BoxLayout.X_AXIS));

        pathNameLabel = new JLabel("common path", SwingConstants.LEFT);
        pathName = new JTextField("/tmp", 25);
        pathPanel = new JPanel();
        pathPanel.setLayout(new BoxLayout(pathPanel, BoxLayout.X_AXIS));
        pathPanel.add(Box.createRigidArea(new Dimension(15, 0)));
        pathPanel.add(pathNameLabel);
        pathPanel.add(Box.createRigidArea(new Dimension(15, 0)));
        pathPanel.add(pathName);
        pathPanel.add(Box.createRigidArea(new Dimension(50, 0)));

        logPanel = new JPanel();
        logPanel.setLayout(new GridLayout(logStatus.logNum + 1, 1));
        logCb = new JCheckBox[logStatus.logNum];
        logLabel = new JLabel("Log Module", SwingConstants.LEFT);
        logPanel.add(logLabel);

        frequencyPanel = new JPanel();
        frequencyPanel.setLayout(new GridLayout(logStatus.logNum + 1, 1));
        logFrequency = new JTextField[logStatus.logNum];
        frequencyLabel = new JLabel("Freq.", SwingConstants.LEFT);
        frequencyPanel.add(frequencyLabel);

        namePanel = new JPanel();
        namePanel.setLayout(new GridLayout(logStatus.logNum + 1, 1));
        fileName = new JTextField[logStatus.logNum];
        fileNameLabel = new JLabel("Filename", SwingConstants.LEFT);
        namePanel.add(fileNameLabel);

        statusPanel = new JPanel();
        statusPanel.setLayout(new GridLayout(logStatus.logNum + 1, 1));
        status = new JRadioButton[logStatus.logNum];
        statusLabel = new JLabel("State", SwingConstants.LEFT);
        statusPanel.add(statusLabel);

        moduleMbxLog = new int[logStatus.logNum];
        for (int i = 0; i < logStatus.logNum; i++)
        {
            logCb[i] = new JCheckBox(RackName.nameString(logStatus.logInfo[i].moduleMbx));
            if (logStatus.logInfo[i].logEnable == 1)
            {
                logCb[i].setSelected(true);
            }
            else
            {
                logCb[i].setSelected(false);
            }
         
            moduleMbxLog[i] = logStatus.logInfo[i].moduleMbx;
            logFrequency[i] = new JTextField("max", 4);
            logFrequency[i].setHorizontalAlignment(JTextField.RIGHT);

            while ((logStatus.logInfo[i].filename).endsWith("\0"))
            {
                logStatus.logInfo[i].filename = (logStatus.logInfo[i].filename).substring(0,
                        (logStatus.logInfo[i].filename).length() - 1);
            }

            fileName[i] = new JTextField(logStatus.logInfo[i].filename, 15);
            status[i] = new JRadioButton();
            status[i].setSelected(false);
            status[i].setForeground(Color.BLACK);

            logPanel.add(logCb[i]);
            frequencyPanel.add(logFrequency[i]);
            namePanel.add(fileName[i]);
            statusPanel.add(status[i]);
        }

        logOnButton = new JButton("On");
        logOffButton = new JButton("Off");

        logOnButton.addActionListener(new ActionListener() 
        {
            public void actionPerformed(ActionEvent e)
            {
                setLog();
                datalog.on();
                getLogStatus();
            }
        });

        logOffButton.addActionListener(new ActionListener() 
        {
            public void actionPerformed(ActionEvent e)
            {
                datalog.off();
                getLogStatus();
            }
        });

        buttonPanel.add(logOnButton);
        buttonPanel.add(logOffButton);

        mainPanel.add(Box.createRigidArea(new Dimension(15, 0)));
        mainPanel.add(logPanel);
        mainPanel.add(Box.createRigidArea(new Dimension(15, 0)));
        mainPanel.add(frequencyPanel);
        mainPanel.add(Box.createRigidArea(new Dimension(15, 0)));
        mainPanel.add(namePanel);
        mainPanel.add(Box.createRigidArea(new Dimension(15, 0)));
        mainPanel.add(statusPanel);
        mainPanel.add(Box.createRigidArea(new Dimension(15, 0)));

        rootPanel.add(buttonPanel, BorderLayout.NORTH);
        rootPanel.add(mainPanel, BorderLayout.CENTER);
        rootPanel.add(pathPanel, BorderLayout.SOUTH);
        rootPanel.setBorder(BorderFactory.createEmptyBorder(15, 15, 15, 15));

        getLogStatus();
    }

    public void getLogStatus()
    {
        DatalogDataMsg state;
        state = datalog.getLogStatus();

        if (state == null)
        {
            state = new DatalogDataMsg();
            state.logNum = 0;
        }
        
        for (int j = 0; j < state.logNum; j++)
        {
            if (state.logInfo[j].logEnable == 1)
            {
                status[j].setForeground(Color.GREEN);
                status[j].setSelected(true);
            }
            else
            {
                status[j].setSelected(false);
                status[j].setForeground(Color.BLACK);
            }
        }
        
        System.out.println("logNum="+state.logNum);
    }

    public void setLog() 
    {
        datalog.initLog();
        getLogStatus();
                                   
        logData = new DatalogDataMsg();
        logData.logPathName = pathName.getText();
        if (!logData.logPathName.endsWith("/"))
        {
            logData.logPathName += "/";
        }
            
        logData.logInfo = new DatalogLogInfo[num];
        logData.logNum = num;
                    
        for (int i = 0; i < num; i++)
        {
            logData.logInfo[i] = new DatalogLogInfo();

            if (logCb[i].isSelected())
            {
                logData.logInfo[i].logEnable = 1;
            }
            else
            {
                logData.logInfo[i].logEnable = 0;
            }
            logData.logInfo[i].moduleMbx = moduleMbxLog[i];
            if (logFrequency[i].getText().equals("max"))
            {
                logData.logInfo[i].periodTime = 0;
            }
            else
            {
                logData.logInfo[i].periodTime = (int) (1000 / Double.valueOf(logFrequency[i].getText())
                        .doubleValue());
            }
            logData.logInfo[i].filename   = fileName[i].getText();
            logData.logInfo[i].maxDataLen = logStatus.logInfo[i].maxDataLen;
        }

        datalog.setLog(logData);

        getLogStatus();
    }
    
    protected void runData()
    {
        DatalogDataMsg data;

        data = datalog.getData();

        if (data != null)
        {
        }
    }
}
