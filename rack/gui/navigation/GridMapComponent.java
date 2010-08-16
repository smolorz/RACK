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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
package rack.gui.navigation;

import java.awt.*;
import java.awt.image.MemoryImageSource;
import javax.swing.*;

import rack.navigation.GridMapDataMsg;


public class GridMapComponent extends JComponent
{

	public GridMapComponent()
	{
		this.setPreferredSize(new Dimension(320,240));
        this.setBackground(Color.LIGHT_GRAY );
	}

	public void transformImage(int zoomRate, GridMapDataMsg dataPackage)
	{
		img = createImage(new MemoryImageSource (dataPackage.gridNumX,
						  dataPackage.gridNumY, dataPackage.occupancyRGB, 0,
						  dataPackage.gridNumX));

		if (zoomRate > 0)
		{
			dataPackage.gridNumX  = dataPackage.gridNumX * zoomRate;
			dataPackage.gridNumY  = dataPackage.gridNumY * zoomRate;
			img = img.getScaledInstance(dataPackage.gridNumX, dataPackage.gridNumY,
										Image.SCALE_FAST);
		}
		if (zoomRate < 0)
		{
			dataPackage.gridNumX  = dataPackage.gridNumX / -zoomRate;
			dataPackage.gridNumY  = dataPackage.gridNumY / -zoomRate;
			img = img.getScaledInstance(dataPackage.gridNumX, dataPackage.gridNumY,
										Image.SCALE_FAST);
		}

		this.setPreferredSize(new Dimension(dataPackage.gridNumX,
											dataPackage.gridNumY));
		this.setSize(new Dimension(dataPackage.gridNumX,
								   dataPackage.gridNumY));

		this.repaint();
	}


    public void paintComponent(Graphics g)
    {
    	if( img != null)
    	{
    		g.drawImage(img, 0, 0, this);
    	}
    }

	protected Image img;

    private static final long serialVersionUID = 1L;
}
