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

import java.io.*;

import rack.main.RackProxy;
import rack.main.tims.*;


public class GridMapDataMsg extends TimsMsg
{
	public int recordingTime   = 0;
	public int offsetX		   = 0;
	public int offsetY		   = 0;
	public int scale		   = 0;
	public int gridNumX  	   = 0;
	public int gridNumY   	   = 0;
	public byte[] occupancy    = new byte[0];

	public int[]  occupancyRGB = new int[0];


    public int getDataLen()
    {
        return (24 + gridNumX * gridNumY);
    }

    public GridMapDataMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public GridMapDataMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if (type == RackProxy.MSG_DATA)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    public void readTimsMsgBody(InputStream in) throws IOException
    {
        EndianDataInputStream dataIn;
    	int					  actualInt;
    	int					  gridNumMax;

        if (bodyByteorder == BIG_ENDIAN)
        {
            dataIn = new BigEndianDataInputStream(in);
        }
        else
        {
            dataIn = new LittleEndianDataInputStream(in);
        }

		recordingTime   = dataIn.readInt();
		offsetX		    = dataIn.readInt();
		offsetY		    = dataIn.readInt();
		scale		    = dataIn.readInt();
		gridNumY  	    = dataIn.readInt();		// exchange gridNumX and gridNumY
		gridNumX   	    = dataIn.readInt();		// for gridMap flip
		gridNumMax      = gridNumX * gridNumY;

		if(occupancy.length != gridNumMax)
			occupancy		= new byte[gridNumMax];
		if(occupancyRGB.length != gridNumMax)
			occupancyRGB	= new int[gridNumMax];

		// read complete byte array
		dataIn.readFully(occupancy);

		// every byte from the input data is put to all colors in one int
		// gridmap is flipped 90deg left for a better visualisation
		for (int j = 0; j < gridNumX; j++)
		{
			for (int i = 0; i < gridNumY; i++)
			{

				// invert occupancy value and write RGB - map
				actualInt = (int)(~occupancy[gridNumY*j + i] & 0xff);
				occupancyRGB[gridNumX*(gridNumY-i-1)+j] = (255 << 24) |
														  (actualInt << 16)|
														  (actualInt << 8) |
														   actualInt;
			}
		}

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
		int gridNumMax;

    	DataOutputStream dataOut = new DataOutputStream(out);
		dataOut.writeInt(recordingTime);
		dataOut.writeInt(offsetX);
		dataOut.writeInt(offsetY);
		dataOut.writeInt(scale);
		dataOut.writeInt(gridNumX);
		dataOut.writeInt(gridNumY);
		gridNumMax = gridNumX * gridNumY;

		for(int i = 0; i < gridNumMax; i++)
		{
			dataOut.writeByte(occupancy[i]);
		}
    }
}
