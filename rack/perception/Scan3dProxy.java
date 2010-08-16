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
 *      Oliver Wulf  <oliver.wulf@gmx.de>
 *
 */
package rack.perception;

import rack.main.*;
import rack.main.tims.*;
import rack.main.RackName;

import java.io.*;

public class Scan3dProxy extends RackDataProxy
{
    public static final byte MSG_SCAN3D_GET_RANGE_IMAGE =
        RackProxy.MSG_POS_OFFSET + 1;

    public static final byte MSG_SCAN3D_RANGE_IMAGE =
        RackProxy.MSG_NEG_OFFSET - 1;

    // get range image timeout is 10s
    protected int getRangeImageTimeout = 10000;

    public Scan3dProxy(int system, int instance, TimsMbx replyMbx)
    {
        super(RackName.create(system, RackName.SCAN3D, instance, 0), replyMbx, 2500);
    }

    public synchronized Scan3dDataMsg getData(int recordingTime)
    {
        try
        {
            TimsRawMsg raw = getRawData(recordingTime);
            if (raw != null)
            {
                Scan3dDataMsg data = new Scan3dDataMsg(raw);
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

    public synchronized Scan3dDataMsg getData()
    {
        try
        {
            TimsRawMsg raw = getNextData();

            if (raw != null)
            {
                Scan3dDataMsg data = new Scan3dDataMsg(raw);
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

    public synchronized Scan3dRangeImageMsg getRangeImage()
    {
        currentSequenceNo++;
        try
        {
            replyMbx.send0(MSG_SCAN3D_GET_RANGE_IMAGE, commandMbx,
                    (byte) 0, currentSequenceNo);

            TimsRawMsg reply;
            do
            {
                reply = replyMbx.receive(getRangeImageTimeout);
            }
            while ((reply.seqNr != currentSequenceNo)
                    & (reply.type == MSG_SCAN3D_RANGE_IMAGE));

            Scan3dRangeImageMsg data = new Scan3dRangeImageMsg(reply);

            // System.out.println(RackName.nameString(replyMbx) +
            // ": Scan3D.getRangeImage");

            return (data);
        }
        catch (TimsException e)
        {
            // System.out.println(MbxName.nameString(replyMbx) +
            // ": Scan3D.getRangeImage " + e);
            return (null);
        }
    }

    public void storeDataToFile(String filename)
    {
        Scan3dDataMsg data = getData();

        if (data != null)
        {
            System.out.println("Store 3D data pointNum=" + data.pointNum
                    + " filename=" + filename);

            try
            {
                PrintWriter fileOut = new PrintWriter(new BufferedWriter(
                        new FileWriter(filename)));

                for (int i = 0; i < data.pointNum; i++)
                {
                    fileOut.println(data.point[i].toString());
                }
                fileOut.flush();
                fileOut.close();
            }
            catch (IOException e)
            {
                System.out.println("Can't write 3D data to file.");
                System.out.println(e.toString());
            }
        }
    }
}
