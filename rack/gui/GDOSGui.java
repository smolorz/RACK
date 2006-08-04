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
package rack.gui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.table.TableColumn;

import rack.main.debug.GDOSDataMsg;
import rack.main.tims.Tims;
import rack.main.tims.exceptions.*;


public class GDOSGui extends Thread {

  protected JPanel panel;

  /** Mailboxnummer des GDOS-Systems */
  protected int GDOSMbx;
  protected GDOSTable gdosTableModel = new GDOSTable();
  protected JTable    gdosTable;

  /** Filtereinstellungen fr Nachrichten */
  protected JRadioButton printRadio   = new JRadioButton("Print");
  protected JRadioButton errorRadio   = new JRadioButton("Error");
  protected JRadioButton warningRadio   = new JRadioButton("Warning");
  protected JRadioButton debugRadio   = new JRadioButton("Info");
  protected JRadioButton debugDetailRadio = new JRadioButton("Detail");
  protected JButton      clearButton  = new JButton("Clear");
  protected JButton      storeButton  = new JButton("Store");
  protected JScrollPane  jsp;
  protected JScrollBar   jsb;
  //Abbruchbedingung
  protected boolean terminate = false;

  /** PRINTOUT (HIGHEST LEVEL) */
  public static final byte GDOS_MSG_PRINT      = -124;
  /** ERROR */
  public static final byte GDOS_MSG_ERROR      = -125;
  /** WARNING */
  public static final byte GDOS_MSG_WARNING    = -126;
  /** DEBUG INFORMATION */
  public static final byte GDOS_MSG_DBG_INFO   = -127;
  /** DETAILED DEBUG INFORMATION */
  public static final byte GDOS_MSG_DBG_DETAIL = -128;

  public GDOSGui(int GDOSMbxNum) {
    GDOSMbx = GDOSMbxNum;
    panel = new JPanel(new BorderLayout());

    printRadio.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        gdosTableModel.setDebugLevel(GDOS_MSG_PRINT);
      }
    });

    errorRadio.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        gdosTableModel.setDebugLevel(GDOS_MSG_ERROR);
      }
    });
    warningRadio.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        gdosTableModel.setDebugLevel(GDOS_MSG_WARNING);
      }
    });
    debugRadio.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        gdosTableModel.setDebugLevel(GDOS_MSG_DBG_INFO);
      }
    });
    debugDetailRadio.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        gdosTableModel.setDebugLevel(GDOS_MSG_DBG_DETAIL);
      }
    });

    clearButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        gdosTableModel.clearMessages();
      }
    });

    storeButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
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

    JPanel panelNorth = new JPanel(new BorderLayout());
    panelNorth.add(panelTopLeft, BorderLayout.WEST);

    gdosTableModel = new GDOSTable();
    gdosTable = new JTable(gdosTableModel);
    gdosTable.setDefaultRenderer(GDOSDataMsg.class, new GDOSMessageRenderer(true));

    TableColumn column = gdosTable.getColumnModel().getColumn(0);
    column.setPreferredWidth(200);
    column = gdosTable.getColumnModel().getColumn(1);
    column.setPreferredWidth(400);

    jsp = new JScrollPane(gdosTable,JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
    gdosTable.setPreferredScrollableViewportSize(new Dimension(600, 200));
    jsb=jsp.getVerticalScrollBar();

    debugDetailRadio.setSelected(true);
    gdosTableModel.setDebugLevel(GDOS_MSG_DBG_DETAIL);

    panel.add(panelNorth,BorderLayout.NORTH);
    panel.add(jsp,BorderLayout.CENTER);
  }

  public JComponent getComponent() {
    return(panel);
  }

  public String getModuleName() {
    return("GDOS");
  }

  public void terminate() {
    terminate = true;
  }

  public void run() {
    GDOSDataMsg data = null;
    boolean firstTimeout = true;

    try {
      while(!terminate) {
        try {
          data = new GDOSDataMsg(Tims.receive(GDOSMbx,500));
        } catch (MsgTimeoutException e) {
          data = null;
        }

        if(data != null) {
          // print GDOS message
          gdosTableModel.addGDOSMsg(data);
          jsb.setValue(jsb.getMaximum());
          firstTimeout = true;
        }else{
          if(firstTimeout)
          {
            // make newest message visible
            jsb.setValue(jsb.getMaximum());
            firstTimeout = false;
          }
        }
      }
    } catch (MsgException e) {
      e.printStackTrace();
    }
    System.out.println("GDOSGui terminated");
  }
}
