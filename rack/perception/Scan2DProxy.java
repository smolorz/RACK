/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.perception;

import java.awt.image.BufferedImage;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

import javax.imageio.ImageIO;

import rack.drivers.CameraDataMsg;
import rack.main.naming.*;
import rack.main.proxy.*;
import rack.main.tims.msg.*;
import rack.main.tims.exceptions.*;

public class Scan2DProxy extends RackDataProxy
{
  public static final int MAX = 8;

  public Scan2DProxy(int id, int replyMbx)
  {
    super(RackName.create(RackName.SCAN2D, id), replyMbx, 10000, 5000, 1000);
    this.id = id;
  }

  public synchronized Scan2DDataMsg getData(int recordingtime)
  {
    try {
      TimsDataMsg raw = getRawData(recordingtime);

      if (raw != null) {
        Scan2DDataMsg data = new Scan2DDataMsg(raw);
        return(data);
      } else {
        return(null);
      }
    } catch(MsgException e) {
      System.out.println(e.toString());
      return(null);
    }
  }

  public synchronized Scan2DDataMsg getData()
  {
    return(getData(0));
  }

  public int getCommandMbx()
  {
    return(RackName.create(RackName.SCAN2D, id));
  }
  
  public int storeDataToFile(String filename, Scan2DDataMsg scan2DData)
  {
      if(scan2DData != null)
      {
         try
          {
	          PrintWriter fileOut = new PrintWriter(new BufferedWriter(
	                  new FileWriter(filename)));
	
	          for (int i = 0; i < scan2DData.pointNum; i++)
	          {
	              fileOut.println(scan2DData.point[i].toString());
	          }
	          fileOut.flush();
	          fileOut.close();
          }
         catch (IOException e)
	      {
	          System.out.println("Can't write 2D data to file.");
	          System.out.println(e.toString());
	      }
      }
	  return 0;
  }
  
  public int storeDataToFile(String filename)
  {
  	Scan2DDataMsg scan2DData = getData(0);
  	return storeDataToFile(filename, scan2DData);
  }
}
