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
 *      Joerg Langenberg  <joerg.langenberg@gmx.net>
 *
 */
package rack.drivers;

import rack.main.*;
import rack.main.tims.*;
import rack.main.RackName;

public class GyroProxy extends RackDataProxy
{

    public GyroProxy(int system, int instance, TimsMbx replyMbx)
    {
        super(RackName.create(system, RackName.GYRO, instance, 0), replyMbx, 500);
    }

    public synchronized GyroDataMsg getData(int recordingTime)
    {
        try
        {
            TimsRawMsg raw = getRawData(recordingTime);

            if (raw != null)
            {
                GyroDataMsg data = new GyroDataMsg(raw);
                return data;
            }
            else
            {
                return null;
            }
        }
        catch(TimsException e)
        {
            System.out.println(e.toString());
            return null;
        }
    }

    public synchronized GyroDataMsg getData()
    {
        return(getData(0));
    }
}
