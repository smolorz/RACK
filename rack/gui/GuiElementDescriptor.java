/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2007 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
package rack.gui;

import java.awt.Dimension;
import java.awt.Point;

import javax.swing.JButton;
import javax.swing.JInternalFrame;
import javax.swing.JPanel;
import javax.swing.JRadioButton;

import rack.main.RackProxy;
import rack.main.tims.Tims;
import rack.main.tims.TimsMbx;

public class GuiElementDescriptor
{
    // general
    String                 name       = "";
    GuiElement             gui;

    RackProxy              proxy;
    int                    instance   = -1;
    int                    status     = Tims.MSG_NOT_AVAILABLE;
    TimsMbx                replyMbx;

    // gui.cfg
    String                 cfg        = "";
    String[]               cfgSplit   = new String[] {""};
    String                 guiClass   = "";
    String                 proxyClass = "";
    boolean                start      = false;
    boolean                show       = false;

    // swing
    JInternalFrame         frame;
    Point                  location   = new Point();
    Dimension              size       = new Dimension();
    JPanel                 navPanel;
    JButton                navButton;
    JRadioButton           navStatusButton;

    // references
    Gui                    mainGui;
    GuiGroupDescriptor     group;
    GuiWorkspaceDescriptor workspace;

    public String getName()
    {
        return name;
    }
    
    public RackProxy getProxy()
    {
        return proxy;
    }
    
    public int getInstance()
    {
        return instance;
    }
    
    public Gui getMainGui()
    {
        return mainGui;
    }
    
    public boolean equals(Object o)
    {
        try
        {
            return cfg == ((GuiElementDescriptor) o).cfg;
        }
        catch (Exception e)
        {
            return false;
        }
    }
    
    public String toString()
    {
        return cfg;
    }
}
