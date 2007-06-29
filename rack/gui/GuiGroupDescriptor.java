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

import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JPanel;

public class GuiGroupDescriptor
{
    GuiGroupDescriptor(String name)
    {
        this.name = name;
    }
    
    String  name;

    JPanel  panel;
    JPanel  interPanel;
    JButton button;

    int     error;
    int     on;
    int     sum;

    Vector<GuiElementDescriptor> elements = new Vector<GuiElementDescriptor>();

    public boolean equals(Object o)
    {
        try
        {
            return name.matches(((GuiGroupDescriptor) o).name);
        }
        catch (Exception e)
        {
            return false;
        }
    }

    public String toString()
    {
        return "GROUP " + name;
    }
}
