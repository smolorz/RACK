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

import java.awt.Color;
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

    // swing
    JInternalFrame         frame;
    Point                  location   = new Point();
    Dimension              size       = new Dimension();
    int                    frameState = Gui.FRAME_STATE_NORMAL;
    JPanel                 navPanel;
    JButton                navButton;
    JRadioButton           navStatusButton;
    Color                  navButtonBackground;

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

    public GuiElement getGui()
    {
        return gui;
    }

    public boolean hasParameter(String parameter)
    {
        for(int i = 0; i < cfgSplit.length; i++)
        {
            if(cfgSplit[i].startsWith("-" + parameter))
            {
                return true;
            }
        }
        return false;
    }

    public String getParameter(String parameter)
    {
        for(int i = 0; i < cfgSplit.length; i++)
        {
            if(cfgSplit[i].startsWith("-" + parameter + "="))
            {
                return cfgSplit[i].substring(parameter.length()+2);
            }
        }
        return "";
    }

    public Gui getMainGui()
    {
        return mainGui;
    }
    
    public String toString()
    {
        return cfg;
    }
    
    public void setNavButtonBackground(Color color)
    {
        if(navButtonBackground == null)
        {
            navButtonBackground = navButton.getBackground();
        }
        
        if(color != null)
        {
            navButton.setBackground(color);
        }
        else
        {
            navButton.setBackground(navButtonBackground);
        }
    }
}
