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

public class ServoDriveProxy extends RackDataProxy
{
    public static final byte MSG_SERVO_DRIVE_HOME =
        RackProxy.MSG_POS_OFFSET + 1;

    public static final byte MSG_SERVO_DRIVE_MOVE_POS =
        RackProxy.MSG_POS_OFFSET + 2;

    public static final byte MSG_SERVO_DRIVE_MOVE_VEL =
        RackProxy.MSG_POS_OFFSET + 3;

    public static final byte MSG_SERVO_DRIVE_POSITION_REACHED =
        RackProxy.MSG_NEG_OFFSET - 1;

    public ServoDriveProxy(int system, int instance, TimsMbx replyMbx)
    {
        super(RackName.create(system, RackName.SERVO_DRIVE, instance, 0), replyMbx, 500);
    }

    public synchronized ServoDriveDataMsg getData(int recordingTime)
    {
        try
        {
            TimsRawMsg raw = getRawData(recordingTime);

            if (raw != null)
            {
                ServoDriveDataMsg data = new ServoDriveDataMsg(raw);
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

    public synchronized ServoDriveDataMsg getData()
    {
        return (getData(0));
    }

    public synchronized void home()
    {
        try
        {

            replyMbx.send0(MSG_SERVO_DRIVE_HOME, commandMbx,
                           (byte) 0, (byte) 0);
            replyMbx.receive(0);
            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx) + ".home");

        }
        catch (TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx) + ".home" + e);
        }
    }

    public synchronized void movePos(float position, float vel, float acc,
            int replyPositionReached)
    {
        try
        {

            ServoDriveMovePosMsg command = new ServoDriveMovePosMsg();
            command.position = position;
            command.vel = vel;
            command.acc = acc;
            command.replyPositionReached = replyPositionReached;

            replyMbx.send(MSG_SERVO_DRIVE_MOVE_POS, commandMbx,
                          (byte) 0, (byte) 0, command);
            replyMbx.receive(0);

            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx) + ".movePos position "
                    + position);
        }
        catch (TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx) + ".movePos " + e);
        }
    }

    public synchronized void moveVel(float vel)
    {
        try
        {

            ServoDriveMoveVelMsg command = new ServoDriveMoveVelMsg();
            command.vel = vel;

            replyMbx.send(MSG_SERVO_DRIVE_MOVE_VEL, commandMbx,
                          (byte) 0, (byte) 0, command);
            replyMbx.receive(0);

            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx) + ".moveVel vel " + vel);
        }
        catch (TimsException e)
        {
            System.out.println(RackName.nameString(replyMbx.getName()) + ": "
                    + RackName.nameString(commandMbx) + ".moveVel " + e);
        }
    }
}
