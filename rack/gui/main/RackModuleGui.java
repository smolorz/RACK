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
package rack.gui.main;

import rack.gui.GuiElement;
import rack.gui.GuiElementDescriptor;
import rack.main.RackProxy;

public abstract class RackModuleGui extends GuiElement
{
    protected RackProxy proxy;

    public RackModuleGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);
        
        proxy = ge.getProxy();
    }
}
