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
package rack.drivers;

import rack.main.naming.*;
import rack.main.proxy.*;
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;
import rack.main.tims.router.*;

public class JoystickProxy extends RackDataProxy
{
    public static final byte MSG_SET_POSITION =
        RackMsgType.RACK_PROXY_MSG_POS_OFFSET + 1;

    public static final int MAX = 8;
    public static final int MAX_BUTTON = 8;
    public static final int MAX_LED = 8;

    public JoystickProxy(int id, int replyMbx)
    {
        super(RackName.create(RackName.JOYSTICK, id), replyMbx, 2000, 1000,
                1000);
        this.id = id;
    }

    public synchronized JoystickDataMsg getData(int recordingtime)
    {
        try
        {
            TimsDataMsg raw = getRawData(recordingtime);
            if (raw != null)
            {
                JoystickDataMsg data = new JoystickDataMsg(raw);
                return data;
            }
            else
            {
                return null;
            }
        }
        catch (MsgException e)
        {
            System.out.println(e.toString());
            return null;
        }
    }

    public synchronized JoystickDataMsg getData()
    {
        return (getData(0));
    }

    public synchronized void setLed(boolean[] leds)
    {
    }

    public synchronized void setData(JoystickDataMsg data)
    {
        currentSequenceNo++;
        try
        {
            TimsMsgRouter.send(MSG_SET_POSITION, commandMbx, replyMbx,
                    (byte) 0, (byte) currentSequenceNo, data);
            TimsDataMsg reply;
            do
            {
                reply = TimsMsgRouter.receive(replyMbx, dataTimeout);
            }
            while (reply.seq_nr != currentSequenceNo);

            System.out.println(RackName.nameString(replyMbx)
                    + ": joystick.setData");
        }
        catch (MsgException e)
        {
            System.out.println(RackName.nameString(replyMbx)
                    + ": joystick.setData " + e);
        }
    }

    public int getCommandMbx()
    {
        return (RackName.create(RackName.JOYSTICK, id));
    }
}
