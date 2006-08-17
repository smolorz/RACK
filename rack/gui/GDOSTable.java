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
 *      Oliver Wulf      <wulf@rts.uni-hannover.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.gui;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Vector;
import javax.swing.table.AbstractTableModel;

import rack.main.debug.GDOS;
import rack.main.debug.GDOSDataMsg;
import rack.main.naming.*;

public class GDOSTable extends AbstractTableModel
{
  protected int debugLevel = GDOS.DBG_DETAIL;
  protected int maxMessages = 1000;
  protected Vector messageList = new Vector(maxMessages);
  protected Vector filteredMessageList = new Vector(maxMessages);

  private static final long serialVersionUID = 1L;

  /* (non-Javadoc)
   * @see javax.swing.table.TableModel#getRowCount()
   */
  public synchronized int getRowCount()
  {
    return filteredMessageList.size();
  }

  /* (non-Javadoc)
   * @see javax.swing.table.TableModel#getColumnCount()
   */
  public int getColumnCount()
  {
    return 2;
  }

  /* (non-Javadoc)
   * @see javax.swing.table.TableModel#getValueAt(int, int)
   */
  public synchronized Object getValueAt(int rowIndex, int columnIndex)
  {
    return filteredMessageList.elementAt(rowIndex);
  }

  public String getColumnName(int column)
  {
    if (column == 0) {
      return "Module";
    } else {
      return "Message";
    }
  }

  public Class getColumnClass(int c) {
    return GDOSDataMsg.class;
  }

  protected boolean isPrintableDebugLevel(GDOSDataMsg message)
  {
    return (message.type >= debugLevel);
  }

  public synchronized void addGDOSMsg(GDOSDataMsg message)
  {
    if (isPrintableDebugLevel(message)) {
      if (filteredMessageList.size() >= maxMessages) {
        filteredMessageList.removeElementAt(0);
        fireTableRowsDeleted(0,0);
      }

      filteredMessageList.addElement(message);
      fireTableRowsInserted(filteredMessageList.size()-1, filteredMessageList.size()-1);
      }

      if (messageList.size() >= maxMessages) {
        messageList.removeElementAt(0);
        fireTableRowsDeleted(0,0);
      }

      messageList.addElement(message);
  }

  public synchronized void clearMessages()
  {
    messageList.clear();

    int size = filteredMessageList.size();
    if (size > 0) {
      filteredMessageList.clear();
      fireTableRowsDeleted(0,size - 1);
    }
  }

  public synchronized void storeMessages()
  {
    messageList.clear();
    int size = filteredMessageList.size();

    if (size > 0) {
      try {
        System.out.println("Write GDOS messages to file \"gdos.txt\".");

        PrintWriter fileOut = new PrintWriter(new BufferedWriter(new FileWriter("gdos.txt")));

        for(int i = 0; i < size; i++) {

          GDOSDataMsg message = (GDOSDataMsg)filteredMessageList.elementAt(i);
          String output = RackName.nameString(message.src);

          switch(message.type) {

            case GDOS.PRINT:
              output = output + " print";
              break;
            case GDOS.ERROR:
              output = output + " error";
              break;
            case GDOS.WARNING:
              output = output + " warning";
              break;
            case GDOS.DBG_INFO:
              output = output + " dbg_info";
              break;
            case GDOS.DBG_DETAIL:
              output = output + " dbg_detail";
              break;

          }
          output = output + " " + message.message;
          fileOut.println(output);
        }

        fileOut.flush();
        fileOut.close();
      } catch (IOException e) {
        System.out.println("Can't write GDOS messages to file.");
        System.out.println(e.toString());
      }
    }
  }

  public synchronized void setDebugLevel(int debugLevel)
  {
    this.debugLevel = debugLevel;
    GDOSDataMsg message;

    int size = filteredMessageList.size();

    if (size > 0) {
      filteredMessageList.clear();
      fireTableRowsDeleted(0,size - 1);
    }

    for(int i = 0; i < messageList.size(); i++) {
      message = (GDOSDataMsg)messageList.elementAt(i);
      if (isPrintableDebugLevel(message)) {
        filteredMessageList.addElement(message);
      }
    }
    fireTableRowsInserted(0, filteredMessageList.size()-1);
  }
}
