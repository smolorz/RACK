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
*      Matthias Hentschel  <hentschel@rts.uni-hannover.de>
 *
 */
package rack.drivers;

import rack.main.*;
import rack.main.tims.*;
import rack.main.RackName;
import rack.drivers.VehicleSetValueMsg;

public class VehicleProxy extends RackDataProxy
{
    public static final byte MSG_VEHICLE_SET_VALUE =
        RackProxy.MSG_POS_OFFSET + 1;

    public VehicleProxy(int system, int instance, TimsMbx replyMbx)
    {
        super(RackName.create(system, RackName.VEHICLE, instance, 0), replyMbx, 500);
    }

    public synchronized VehicleDataMsg getData(int recordingTime)
    {
        try
        {
            TimsRawMsg raw = getRawData(recordingTime);

            if (raw != null)
            {
                VehicleDataMsg data = new VehicleDataMsg(raw);
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

    public synchronized VehicleDataMsg getData()
    {
        return(getData(0));
    }

    public synchronized void setValue(VehicleSetValueMsg values)
    {
        currentSequenceNo++;
        try
        {
            replyMbx.send(MSG_VEHICLE_SET_VALUE, commandMbx,
                               (byte)0, currentSequenceNo, values);

            TimsRawMsg reply;
            do
            {
              reply = replyMbx.receive(replyTimeout);
            }
            while(reply.seqNr != currentSequenceNo);
        }
        catch(TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": " +
                               RackName.nameString(commandMbx) +
                               ".setValue " + e);
        }
    }
}
