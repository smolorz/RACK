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


public class ImageDescription
{
	String imageFilename;
	double centerFirstPixelX, centerFirstPixelY;
	int numberOfPixelsX, numberOfPixelsY;
	float sizeOfPixelX, sizeOfPixelY;
	double xMin, xMax, yMin, yMax;
	
	ImageDescription()
	{
		super();
	}
	
	public void setIFilename(String s)
	{
		imageFilename = s;
	}
	
	public void setCenterFirstPixelX(double cx)
	{
		centerFirstPixelX = cx;
	}
	
	public void setCenterFirstPixelY(double cy)
	{
		centerFirstPixelY = cy;
	}
	
	public void setNumberOfPixelsX(int nx)
	{
		numberOfPixelsX = nx;
	}
	
	public void setNumberOfPixelsY(int ny)
	{
		numberOfPixelsY = ny;
	}
	
	public void setSizeOfPixelX(float sx)
	{
		sizeOfPixelX = sx; 
	}
	
	public void setSizeOfPixelY(float sy)
	{
		sizeOfPixelY = sy; 
	}
	
	public void setXMin(double xmin)
	{
		xMin = xmin;
	}
	
	public void setXMax(double xmax)
	{
		xMax = xmax;
	}
	
	public void setYMin(double ymin)
	{
		yMin = ymin;
	}
	
	public void setYMax(double ymax)
	{
		yMax = ymax;
	}
	
	public String getIFilename()
	{
		return imageFilename;
	}
	
	public double[] getCentersFirstPixel()
	{
		double[] d = {centerFirstPixelX, centerFirstPixelY};
		return d;
	}
	
	public int[] getNumbersOfPixel()
	{
		int[] i = {numberOfPixelsX, numberOfPixelsY};
		return i;
	}
	
	public float[] getSizesOfPixel()
	{
		float[] f = {sizeOfPixelX, sizeOfPixelY};
		return f;
	}
	
	public double[] getX()
	{
		double[] d = {xMin, xMax};
		return d;
	}
	
	public double[] getY()
	{
		double[] d = {yMin, yMax};
		return d;
	}
	
	public void finalize()
	{
	}
}