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

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

import rack.main.*;
import rack.main.tims.*;

public class Scan2dProxy extends RackDataProxy
{
  public Scan2dProxy(int id, TimsMbx replyMbx)
  {
    super(RackName.create(RackName.SCAN2D, id), replyMbx, 2500);
  }

  public synchronized Scan2dDataMsg getData(int recordingTime)
  {
    try {
      TimsRawMsg raw = getRawData(recordingTime);

      if (raw != null) {
        Scan2dDataMsg data = new Scan2dDataMsg(raw);
        return(data);
      } else {
        return(null);
      }
    } catch(TimsException e) {
      System.out.println(e.toString());
      return(null);
    }
  }

  public synchronized Scan2dDataMsg getData()
  {
      long timeA, timeB;
      
    try {
        timeA = System.currentTimeMillis();
//      TimsRawMsg raw = getNextData();
      TimsRawMsg raw = getRawData(0);
      timeB = System.currentTimeMillis();
      System.out.println("get data " + (timeB - timeA) + "ms");

      if (raw != null) {
        Scan2dDataMsg data = new Scan2dDataMsg(raw);
        return(data);
      } else {
        return(null);
      }
    } catch(TimsException e) {
      System.out.println(e.toString());
      return(null);
    }
  }

  public int storeDataToFile(String filename, Scan2dDataMsg scan2dData)
  {
      if(scan2dData != null)
      {
         try
          {
	          PrintWriter fileOut = new PrintWriter(new BufferedWriter(
	                  new FileWriter(filename)));
	
	          for (int i = 0; i < scan2dData.pointNum; i++)
	          {
	              fileOut.println(scan2dData.point[i].toString());
	          }
	          fileOut.flush();
	          fileOut.close();
          }
         catch (IOException e)
	      {
	          System.out.println("Can't write 2d data to file.");
	          System.out.println(e.toString());
	      }
      }
	  return 0;
  }
  
  public int storeDataToFile(String filename)
  {
  	Scan2dDataMsg scan2dData = getData(0);
  	return storeDataToFile(filename, scan2dData);
  }
}
