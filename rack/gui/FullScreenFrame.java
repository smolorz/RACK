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
 *      Jan Kiszka <kiszka@rts.uni-hannover.de>
 *
 */

package rack.gui;

import java.awt.*;
import java.awt.image.*;

import javax.swing.*;

public class FullScreenFrame extends JFrame {

	private static final long serialVersionUID = 1L;

	public FullScreenFrame()
	{
    	setUndecorated(true);
    	setSize(Toolkit.getDefaultToolkit().getScreenSize());
    	setAlwaysOnTop(true);
	}

	public void hideCursor()
	{
    	Image image = Toolkit.getDefaultToolkit().createImage(
    	        new MemoryImageSource(0, 0, null, 0, 16));
    	Cursor transparentCursor = Toolkit.getDefaultToolkit().
    		createCustomCursor(image, new Point(0,0), "");
    	setCursor(transparentCursor);
	}
}
