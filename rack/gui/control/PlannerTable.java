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
 *
 */
package rack.gui.control;

import java.util.Vector;
import javax.swing.table.AbstractTableModel;

import rack.control.PlannerDataMsg;

public class PlannerTable extends AbstractTableModel
{
    private static final long serialVersionUID = 1L;
    protected int             maxMessages      = 100;
    protected Vector<String>  messageList      = new Vector<String>(maxMessages);
    protected Vector<String>  stateList        = new Vector<String>(maxMessages);

    public synchronized int getRowCount()
    {
        return messageList.size();
    }

    public int getColumnCount()
    {
        return 2;
    }

    public synchronized Object getValueAt(int rowIndex, int columnIndex)
    {
        if (columnIndex == 0)
        {
            return stateList.elementAt(rowIndex);
        }
        else
        {
            return messageList.elementAt(rowIndex);
        }
    }

    public String getColumnName(int column)
    {
        if (column == 0)
        {
            return "State";
        }
        else
        {
            return "Message";
        }
    }

    public synchronized void addPlannerMsg(PlannerDataMsg data)
    {
        if (messageList.size() >= maxMessages)
        {
            messageList.removeElementAt(0);
            stateList.removeElementAt(0);
            fireTableRowsDeleted(0, 0);
        }
        String s = new String(data.message[0].string);
        s = s.trim();
        stateList.addElement(String.valueOf(data.state));
        messageList.addElement(s);
        fireTableRowsInserted(messageList.size() - 1, messageList.size() - 1);
    }

    public synchronized void clearMessages()
    {
        stateList.clear();
        messageList.clear();
        fireTableRowsDeleted(0, 0);
    }
}
