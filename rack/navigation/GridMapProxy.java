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
package rack.navigation;

import java.io.IOException;
import java.io.File;
import java.awt.image.BufferedImage;
import javax.imageio.*;

import rack.main.*;
import rack.main.tims.*;
import rack.main.RackName;


public class GridMapProxy extends RackDataProxy
{

    public GridMapProxy(int system, int instance, TimsMbx replyMbx)
    {
    	super(RackName.create(system, RackName.GRID_MAP, instance, 0), replyMbx, 2500);
    }

    public void storeDataToFile(String filename)
	{
    	GridMapDataMsg data = getData();

		if (data != null)
		{
			try
			{
				System.out.println("Store gridmap data filename=" + filename);
			    BufferedImage image = new BufferedImage(data.gridNumX,
			    								data.gridNumY,
			    								BufferedImage.TYPE_INT_RGB);
			    image.setRGB(0, 0, data.gridNumX, data.gridNumY,
			    		     data.occupancyRGB, 0, data.gridNumX);

			    File file = new File(filename);
		        ImageIO.write(image, "png", file);
			}
			catch(IOException e)
			{
				System.out.println("Error storing gridnmap data filename=" +
								   filename + e.toString());
			}
		}
	}

    public synchronized void getData (int recordingTime, GridMapDataMsg data)
    {
    	try
        {
    		TimsRawMsg raw = getRawData(recordingTime);

        	if (raw != null)
        	{
				data.readTimsRawMsg(raw);
        	}
        	else
        	{
        		data.gridNumX = 0;
        		data.gridNumY = 0;
        	}
        }
        catch(TimsException e)
        {
            System.out.println(e.toString());
    		data.gridNumX = 0;
    		data.gridNumY = 0;
        }
    }

    public synchronized GridMapDataMsg getData(int recordingTime)
    {
        try
        {
    		TimsRawMsg raw = getRawData(recordingTime);

        	if(raw != null)
        	{
				GridMapDataMsg data = new GridMapDataMsg(raw);
				return data;
        	}
        	else
        	{
        		return null;
        	}
        }
        catch (TimsException e)
        {
            System.out.println(e.toString());
            return null;
        }
    }

    public synchronized GridMapDataMsg getData()
    {
        return(getData(0));
    }

    public synchronized void getData(GridMapDataMsg data)
    {
        getData(0, data);
    }
}
