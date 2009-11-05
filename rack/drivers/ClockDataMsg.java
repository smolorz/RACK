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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
package rack.drivers;

import java.io.*;

import rack.main.RackProxy;
import rack.main.tims.*;

public class ClockDataMsg extends TimsMsg
{
    public int          recordingTime = 0;
    public int          hour          = 0;   // hour   [00...23]
    public int          minute        = 0;   // minute [00...59]
    public int          second        = 0;   // second [00...59]
    public int          day           = 0;   // day    [01...31]
    public int          month         = 0;   // month  [01...12]
    public int          year          = 0;   // year   [00...99] without century
    public long         utcTime       = 0;   // POSIX time in sec since 1.1.1970
    public int          dayOfWeek     = 0;   // day of week, 1=monday...7=sunday
    public int          syncMode      = 0;   // mode for clock synchronisation
    public int          varT          = 0;   // variance of the clock sensor in ms

    public static final byte CLOCK_SYNC_MODE_NONE   = 0;
    public static final byte CLOCK_SYNC_MODE_REMOTE = 1;


    public int getDataLen()
    {
        return 48;
    }

    public ClockDataMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public ClockDataMsg(TimsRawMsg p) throws TimsException
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

        recordingTime = dataIn.readInt();
        hour          = dataIn.readInt();
        minute        = dataIn.readInt();
        second        = dataIn.readInt();
        day           = dataIn.readInt();
        month         = dataIn.readInt();
        year          = dataIn.readInt();
        utcTime       = dataIn.readLong();
        dayOfWeek     = dataIn.readInt();
        syncMode      = dataIn.readInt();
        varT          = dataIn.readInt();

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(recordingTime);
        dataOut.writeInt(hour);
        dataOut.writeInt(minute);
        dataOut.writeInt(second);
        dataOut.writeInt(day);
        dataOut.writeInt(month);
        dataOut.writeInt(year);
        dataOut.writeLong(utcTime);
        dataOut.writeInt(dayOfWeek);
        dataOut.writeInt(syncMode);
        dataOut.writeInt(varT);
    }
}
