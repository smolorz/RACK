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
 *      Oliver Wulf  <oliver.wulf@gmx.de>
 *
 */
package rack.drivers;

import rack.main.*;
import rack.main.tims.*;

public class LadarProxy extends RackDataProxy
{
    public LadarProxy(int system, int instance, TimsMbx replyMbx)
    {
        super(RackName.create(system, RackName.LADAR, instance, 0), replyMbx, 500);
    }

    public synchronized LadarDataMsg getData(int recordingTime)
    {
        try
        {
            TimsRawMsg raw = getRawData(recordingTime);

            if (raw != null)
            {
                LadarDataMsg data = new LadarDataMsg(raw);
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

    public synchronized LadarDataMsg getData()
    {
        try
        {
            TimsRawMsg raw = getNextData();

            if (raw != null)
            {
                LadarDataMsg data = new LadarDataMsg(raw);
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
}
