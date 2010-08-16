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

import java.io.*;

import rack.main.RackProxy;
import rack.main.tims.*;

public class VehicleDataMsg extends TimsMsg
{
    public int       recordingTime  = 0;
    public int		 speed		    = 0;
    public float	 omega			= 0;
    public float     throttle       = 0;
    public float     brake          = 0;
    public float     clutch         = 0;
    public float     steering       = 0;
    public int       gear           = 0;
    public int       engine         = 0;
    public int       parkBrake      = 0;
    public int       vehicleProtect = 0;
    public int		 activeController = 0;

    public int getDataLen()
    {
        return 48;

    }

    public VehicleDataMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public VehicleDataMsg(TimsRawMsg p) throws TimsException
    {
        readTimsRawMsg(p);
    }

    public boolean checkTimsMsgHead()
    {
        if (type == RackProxy.MSG_DATA)
        {
            return (true);
        }
        else
        {
            return (false);
        }
    }

    public void readTimsMsgBody(InputStream in) throws IOException
    {
        EndianDataInputStream dataIn;

        if (bodyByteorder == BIG_ENDIAN)
        {
            dataIn = new BigEndianDataInputStream(in);
        }
        else
        {
            dataIn = new LittleEndianDataInputStream(in);
        }

        recordingTime    = dataIn.readInt();
        speed			 = dataIn.readInt();
        omega			 = dataIn.readFloat();
        throttle         = dataIn.readFloat();
        brake            = dataIn.readFloat();
        clutch           = dataIn.readFloat();
        steering         = dataIn.readFloat();
        gear             = dataIn.readInt();
        engine           = dataIn.readInt();
        parkBrake        = dataIn.readInt();
        vehicleProtect   = dataIn.readInt();
        activeController = dataIn.readInt();

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(recordingTime);
        dataOut.writeInt(speed);
        dataOut.writeFloat(omega);
        dataOut.writeFloat(throttle);
        dataOut.writeFloat(brake);
        dataOut.writeFloat(clutch);
        dataOut.writeFloat(steering);
        dataOut.writeInt(gear);
        dataOut.writeInt(engine);
        dataOut.writeInt(parkBrake);
        dataOut.writeInt(vehicleProtect);
        dataOut.writeInt(activeController);
    }
}
