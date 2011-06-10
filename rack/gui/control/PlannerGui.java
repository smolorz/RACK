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
 *      Daniel Lecking  <lecking@rts.uni-hannover.de>
 *      Marko Reimer    <reimer@rts.uni-hannover.de>
 */

package rack.gui.control;

import java.awt.*;
import java.awt.event.*;

import javax.swing.*;
import javax.swing.table.TableColumn;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.RackModuleGui;
import rack.main.*;
import rack.control.*;

public class PlannerGui extends RackModuleGui
{
    protected PlannerProxy      planner;
    protected PlannerDataMsg    plannerData;
    protected byte              plannerDataType;
    protected String[]          commElement             = {};

    protected JButton			clearButton;
    protected JButton           sendButton;
    protected JComboBox         commandBox;
    protected JTextField        parameterField;

    protected JLabel            commandLabel;
    protected JLabel            optionLabel;

    protected JTable            plannerTable;
    protected PlannerTable      plannerTableModel;
    protected JScrollPane       plannerScrollPane;
    protected JScrollBar        plannerScrollBar;

    protected PlannerCommandMsg commList                = new PlannerCommandMsg();
    protected boolean           firstTimeout            = true;
    protected String            oldMessage              = "";
    protected int               oldState                = -1;

    public PlannerGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        planner = (PlannerProxy) proxy;

        JPanel buttonPanel = new JPanel(new GridLayout(2, 3, 4, 2));
        JPanel northPanel = new JPanel(new GridLayout(0, 1, 4, 2));

        // creat the button
        clearButton = new JButton("Clear");
        sendButton  = new JButton("Send");

        parameterField = new JTextField(10);

        commandBox = new JComboBox(commElement);
        commandBox.setEditable(false);

        plannerTableModel = new PlannerTable();

        plannerTable = new JTable(plannerTableModel);

        TableColumn column = plannerTable.getColumnModel().getColumn(0);
        column.setPreferredWidth(50);
        column = plannerTable.getColumnModel().getColumn(1);
        column.setPreferredWidth(150);

        plannerScrollPane = new JScrollPane(plannerTable, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        plannerTable.setPreferredScrollableViewportSize(new Dimension(300, 100));
        plannerScrollBar = plannerScrollPane.getVerticalScrollBar();

        clearButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                plannerTableModel.clearMessages();
            }
        });

        sendButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                PlannerCommandMsg data = new PlannerCommandMsg();
                try
                {
                    data.commandNum = 1;
                    data.command = new PlannerString[data.commandNum];
                    data.command[0]= new PlannerString();
                    String s = (String) commandBox.getSelectedItem() + parameterField.getText();
                    if (s.length() > PlannerString.MAX_STRING_LEN)
                    {
                        parameterField.setText("command string to long !");
                    }
                    else
                    {
                        data.command[0].string = s;
                        data.command[0].stringLen = s.length();
                        planner.sendCommand(PlannerProxy.MSG_PLANNER_COMMAND, data);
                        parameterField.setText("");
                    }
                }
                catch (NumberFormatException nfe)
                {}
            }
        });

        commandBox.addItemListener( new ItemListener()
        {
            public void itemStateChanged( ItemEvent e )
            {
                //parameterField.setText("");
            }
        });

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);
        buttonPanel.add(clearButton);
        buttonPanel.add(commandBox);
        buttonPanel.add(parameterField);
        buttonPanel.add(sendButton);

        northPanel.add(buttonPanel);

        rootPanel.add(new JLabel(RackName.nameString(planner.getCommandMbx())), BorderLayout.NORTH);
        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(plannerScrollPane, BorderLayout.CENTER);

        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        sendButton.setEnabled(enabled);
        commandBox.setEnabled(enabled);
    }

    protected void runData()
    {
        PlannerDataMsg data = new PlannerDataMsg();

        data = planner.getData();
        if (data != null)
        {
            if (data.messageNum > 0)
            {
                if ((data.message[0].string.equals(oldMessage) == false) ||
                    (data.state != oldState))
                {
                    plannerTableModel.addPlannerMsg(data);
    
                    try
                    {
    					Thread.sleep(100);
    				}
                    catch (InterruptedException e) {}
    
                    plannerScrollBar.setValue(plannerScrollBar.getMaximum());
                    firstTimeout = true;
                    oldMessage = data.message[0].string;
                    oldState = data.state;
                }
                setEnabled(true);
            }
        }
        else
        {
            setEnabled(false);

            if (firstTimeout)
            {
                plannerScrollBar.setValue(plannerScrollBar.getMaximum());
                oldMessage = "";
                oldState = -1;
                firstTimeout = false;
            }
        }
    }

    public void run()
    {
        PlannerCommandMsg commList = new PlannerCommandMsg();
        firstTimeout = true;
        oldMessage = "";
        commList = planner.getCommandList();
        if(commList != null)
        {
            for (int i = 0; i < commList.commandNum; i++)
            {
                String s = new String(commList.command[i].string);
                s = s.trim();
                if (s.length() != 0)
                {
                    commandBox.addItem(s);
                }
            }
        }
        super.run();
    }
}
