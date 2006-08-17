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

import java.awt.Color;
import java.awt.Component;

import javax.swing.BorderFactory;
import javax.swing.JLabel;
import javax.swing.JTable;
import javax.swing.border.Border;
import javax.swing.table.TableCellRenderer;

import rack.main.debug.GDOS;
import rack.main.debug.GDOSDataMsg;
import rack.main.naming.*;

/**
 * @author wulf
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class GDOSMessageRenderer extends JLabel implements TableCellRenderer
{

    protected Color colorPrint = Color.WHITE;
    protected Color colorError = new Color(1.0f, 0.8f, 0.8f); // light red
    protected Color colorWarning = new Color(1.0f, 1.0f, 0.8f); // light yellow
    protected Color colorInfo = new Color(0.8f, 1.0f, 0.8f); // light green
    protected Color colorDetail = new Color(0.9f, 0.9f, 0.9f); // light gray

    protected Border unselectedBorder = null;
    protected Border selectedBorder = null;
    protected boolean isBordered = true;

    private static final long serialVersionUID = 1L;

    public GDOSMessageRenderer(boolean isBordered)
    {
        this.isBordered = isBordered;
        setOpaque(true); // MUST do this for background to show up.
    }

    /*
     * (non-Javadoc)
     *
     * @see javax.swing.table.TableCellRenderer#getTableCellRendererComponent(javax.swing.JTable,
     *      java.lang.Object, boolean, boolean, int, int)
     */
    public Component getTableCellRendererComponent(JTable table, Object value,
            boolean isSelected, boolean hasFocus, int row, int column)
    {
        GDOSDataMsg gdosMsg = (GDOSDataMsg) value;

        if (column == 0)
        {
            String module = RackName.nameString(gdosMsg.src);
            setText(module);
            setToolTipText(module);
        }
        else
        {
            setText(gdosMsg.message);
            setToolTipText(gdosMsg.message);
        }

        switch (gdosMsg.type)
        {
            case GDOS.WARNING:
                setBackground(colorWarning);
                break;
            case GDOS.ERROR:
                setBackground(colorError);
                break;
            case GDOS.DBG_INFO:
                setBackground(colorInfo);
                break;
            case GDOS.DBG_DETAIL:
                setBackground(colorDetail);
                break;
            default:
            case GDOS.PRINT:
                setBackground(colorPrint);
                break;
        }

        setBorder(BorderFactory.createMatteBorder(2, 5, 2, 5, getBackground()));

        return this;
    }
}
