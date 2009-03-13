/*
 * RACK-RTS - Robotics Application Construction Kit (RTS internal)
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * All rights reserved.
 *
 * Authors
 *      Joerg Langenberg  <joerg.langenberg@gmx.net>
 *
 */
package rack_level2.drivers;

import rack.main.naming.*;
import rack.main.proxy.*;
import rack.main.tims.msg.*;
import rack.main.tims.exceptions.*;

public class DummyProxy extends RackDataProxy
{

    public DummyProxy(int id, int replyMbx)
    {
        super(RackName.create(RackName.GYRO, id), replyMbx, 5000, 1000, 1000);
        this.id = id;
    }

    public synchronized DummyDataMsg getData(int recordingtime)
    {
	try
        {
            TimsDataMsg raw = getRawData(recordingtime);

            if (raw != null)
            {
                DummyDataMsg data = new DummyDataMsg(raw);
                return data;
            }
            else
            {
                return null;
            }
        }
        catch(MsgException e)
        {
            System.out.println(e.toString());
            return null;
        }
    }

    public synchronized DummyDataMsg getData()
    {
        return(getData(0));
    }

    public int getCommandMbx()
    {
        return(RackName.create(RackName.GYRO, id));
    }
}
