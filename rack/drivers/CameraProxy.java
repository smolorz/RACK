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
 *      Marko Reimer     <reimer@l3s.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.drivers;

import rack.main.*;
import rack.main.tims.*;

public class CameraProxy extends RackDataProxy {

    public static final byte MSG_CAMERA_GET_PARAMETER =
        RackProxy.MSG_POS_OFFSET + 1;

    public static final byte MSG_CAMERA_SET_FORMAT =
        RackProxy.MSG_POS_OFFSET + 2;

    public static final byte MSG_CAMERA_PARAMETER =
        RackProxy.MSG_NEG_OFFSET - 1;

    public static final byte MSG_CAMERA_FORMAT =
        RackProxy.MSG_NEG_OFFSET - 2;


    public CameraProxy(int id, TimsMbx replyMbx)
    {
        super(RackName.create(RackName.CAMERA, id), replyMbx, 5000, 1000, 5000);
        this.id = id;
    }

    public synchronized CameraDataMsg getData(int recordingTime)
    {
        try
        {
            TimsRawMsg raw = getRawData(recordingTime);

            if(raw != null)
            {
                CameraDataMsg data = new CameraDataMsg(raw);
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

    public synchronized CameraDataMsg getData()
    {
        try
        {
            TimsRawMsg raw = getNextData();

            if(raw != null)
            {
                CameraDataMsg data = new CameraDataMsg(raw);
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

    public synchronized void setFormat(CameraFormatMsg format)
    {
        currentSequenceNo++;
        System.out.println(format);

        try {
            replyMbx.send(
                MSG_CAMERA_SET_FORMAT,
                commandMbx,
                (byte)0,
                (byte)currentSequenceNo,
                format);

            TimsRawMsg reply;

            do {
                reply = replyMbx.receive(dataTimeout);
            } while (reply.seqNr != currentSequenceNo);

            if (reply.type == RackProxy.MSG_OK) {
                System.out.println(RackName.nameString(replyMbx.getName()) +
                        ": cameraProxy setFormat");
            } else {
                System.out.println(
                        RackName.nameString(replyMbx.getName())
                        + ": "
                        + RackName.nameString(commandMbx)
                        + ".setFormat replied error");
            }
        } catch (TimsException e) {
            System.out.println(RackName.nameString(replyMbx.getName()) +
                    ": cameraProxy setFormat " + e);
        }
    }

    public int getCommandMbx()
    {
        return(RackName.create(RackName.CAMERA, id));
    }
}
