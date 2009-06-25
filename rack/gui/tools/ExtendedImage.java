/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2009	   University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Frauke Wübbold	<wuebbold@rts.uni-hannover.de>
 *
 */

package rack.gui.tools;

import java.awt.image.BufferedImage;


public class ExtendedImage
{
	BufferedImage bImage;
	long width, height, centerX, centerY;
	
	ExtendedImage()
	{
		super();
	}
	
	public void setImage(BufferedImage bI)
	{
		bImage = bI;
	}
	
	public void setWidth(long w)
	{
		width = w;
	}
	
	public void setHeight(long h)
	{
		height = h;
	}
	
	public void setCenterX(long cX, long offset)
	{
		centerX = cX - offset; 
		//System.out.println("ExtendedImage: centerX, offset, subst. " + cX + " " + offset + " " + centerX);
	}
	
	public void setCenterY(long cY, long offset)
	{
		centerY = cY - offset;
		//System.out.println("ExtendedImage: centerY, offset, subst. " + cY + " " + offset + " " + centerY);
	}	
	
	public BufferedImage getImage()
	{
		return bImage;
	}
	
	public long getWidth()
	{
		return width;
	}
	
	public long getHeight()
	{
		return height;
	}
	
	public long getCenterX()
	{
		return centerX;
	}
	
	public long getCenterY()
	{
		return centerY;
	}
	
	public void finalize()
	{
	}
}