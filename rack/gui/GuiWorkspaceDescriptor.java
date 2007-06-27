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
import javax.swing.JDesktopPane;

public class GuiWorkspaceDescriptor
{
    GuiWorkspaceDescriptor(String name)
    {
        this.name = name;
    }
    
    String              name;
    JDesktopPane        jdp;

    Vector<GuiElementDescriptor>  element = new Vector<GuiElementDescriptor>();

    public boolean equals(Object o)
    {
        try
        {
            return name == ((GuiWorkspaceDescriptor) o).name;
        }
        catch (Exception e)
        {
            return false;
        }
    }
}
