/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.gui.main;

import java.awt.*;
import java.awt.event.*;
import java.util.Vector;

import javax.swing.*;
import javax.swing.table.TableColumn;

import rack.gui.GuiElement;
import rack.gui.GuiElementDescriptor;
import rack.main.GDOS;
import rack.main.GDOSDataMsg;
import rack.main.RackName;
import rack.main.tims.*;

public class GDOSGui extends GuiElement
{
    protected JPanel       rootPanel;

    /** Mailboxnummer des GDOS-Systems */
    protected TimsMbx      gdosMbx;
    protected GDOSTable    gdosTableModel   = new GDOSTable();
    protected JTable       gdosTable;

    /** Filtereinstellungen fr Nachrichten */
    protected JRadioButton printRadio       = new JRadioButton("Print");
    protected JRadioButton errorRadio       = new JRadioButton("Error");
    protected JRadioButton warningRadio     = new JRadioButton("Warning");
    protected JRadioButton debugRadio       = new JRadioButton("Info");
    protected JRadioButton debugDetailRadio = new JRadioButton("Detail");
    protected JButton      clearButton      = new JButton("Clear");
    protected JButton      storeButton      = new JButton("Store");
    protected JCheckBox    multiErrorCheck  = new JCheckBox("MultiErrorCheck", true);
    
    protected JScrollPane  jsp;
    protected JScrollBar   jsb;

    protected Color             multiErrorBackground;
    protected Vector<String>    multiErrorList  = new Vector<String>();

    public GDOSGui(GuiElementDescriptor guiElement) throws TimsException
    {
        super(guiElement);

        gdosMbx = mainGui.getTims().mbxInit(RackName.create(RackName.GDOS, 0));
        
        rootPanel = new JPanel(new BorderLayout());

        printRadio.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                gdosTableModel.setDebugLevel(GDOS.PRINT);
            }
        });

        errorRadio.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                gdosTableModel.setDebugLevel(GDOS.ERROR);
            }
        });

        warningRadio.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                gdosTableModel.setDebugLevel(GDOS.WARNING);
            }
        });

        debugRadio.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                gdosTableModel.setDebugLevel(GDOS.DBG_INFO);
            }
        });

        debugDetailRadio.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                gdosTableModel.setDebugLevel(GDOS.DBG_DETAIL);
            }
        });

        clearButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                gdosTableModel.clearMessages();

                multiErrorList.clear();
                ge.setNavButtonBackground(null);
                multiErrorCheck.setBackground(multiErrorBackground);
            }
        });

        storeButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                gdosTableModel.storeMessages();
            }
        });

        ButtonGroup radio = new ButtonGroup();
        radio.add(printRadio);
        radio.add(errorRadio);
        radio.add(warningRadio);
        radio.add(debugRadio);
        radio.add(debugDetailRadio);

        JPanel panelTopLeft = new JPanel();
        panelTopLeft.add(clearButton);
        panelTopLeft.add(storeButton);
        panelTopLeft.add(printRadio);
        panelTopLeft.add(errorRadio);
        panelTopLeft.add(warningRadio);
        panelTopLeft.add(debugRadio);
        panelTopLeft.add(debugDetailRadio);
        panelTopLeft.add(multiErrorCheck);

        JPanel panelNorth = new JPanel(new BorderLayout());
        panelNorth.add(panelTopLeft, BorderLayout.WEST);

        gdosTableModel = new GDOSTable();
        gdosTable = new JTable(gdosTableModel);
        gdosTable.setDefaultRenderer(GDOSDataMsg.class, new GDOSMessageRenderer(true));

        TableColumn column = gdosTable.getColumnModel().getColumn(0);
        column.setPreferredWidth(200);
        column = gdosTable.getColumnModel().getColumn(1);
        column.setPreferredWidth(400);

        jsp = new JScrollPane(gdosTable, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        gdosTable.setPreferredScrollableViewportSize(new Dimension(600, 200));
        jsb = jsp.getVerticalScrollBar();

        debugDetailRadio.setSelected(true);
        gdosTableModel.setDebugLevel(GDOS.DBG_DETAIL);

        rootPanel.add(panelNorth, BorderLayout.NORTH);
        rootPanel.add(jsp, BorderLayout.CENTER);
    }

    public boolean ckeckMultiError(GDOSDataMsg data)
    {
        if(multiErrorBackground == null)
        {
            multiErrorBackground = multiErrorCheck.getBackground();
        }
        
        if(multiErrorCheck.isSelected())
        {
            String src = RackName.classString(data.src);
            
            if(multiErrorList.contains(src))
            {
                if((data.type == GDOS.PRINT) &
                   ((data.message.startsWith("Module on")) ||
                    (data.message.startsWith("Module off")) ||
                    (data.message.startsWith("Init")) ||
                    (data.message.startsWith("Terminated"))))
                {
                    multiErrorList.remove(src);

                    if(multiErrorList.size() == 0)
                    {
                        ge.setNavButtonBackground(null);
                        multiErrorCheck.setBackground(multiErrorBackground);
                    }

                    return true;
                }
                
                return false;
            }
            else
            {
                if((data.type == GDOS.PRINT) &
                   (data.message.startsWith("Error")))
                {
                    ge.setNavButtonBackground(Color.RED);
                    multiErrorCheck.setBackground(Color.RED);
                    multiErrorList.add(src);
                }
                return true;
            }
        }
        else
        {
            multiErrorList.clear();
            ge.setNavButtonBackground(null);
            multiErrorCheck.setBackground(multiErrorBackground);
            return true;
        }
    }

    public JComponent getComponent()
    {
        return rootPanel;
    }

    public void run()
    {
        GDOSDataMsg data = null;
        boolean firstTimeout = true;

        while (terminate == false)
        {
            try
            {
                try
                {
                    data = new GDOSDataMsg(gdosMbx.receive(500));
                }
                catch (TimsTimeoutException e)
                {
                    data = null;
                }

                if (data != null)
                {
                    if(ckeckMultiError(data))
                    {
                        // print GDOS message
                        gdosTableModel.addGDOSMsg(data);
                        jsb.setValue(jsb.getMaximum());
                        firstTimeout = true;
                    }
                }
                else
                {
                    if (firstTimeout)
                    {
                        // make newest message visible
                        jsb.setValue(jsb.getMaximum());
                        firstTimeout = false;
                    }
                }
            }
            catch (TimsException e)
            {
                e.printStackTrace();
                terminate = true;
            }
        }
        
        if(gdosMbx != null)
            gdosMbx.delete();
        
        rootPanel.removeAll();
        
        System.out.println("GDOSGui terminated");
    }
}
