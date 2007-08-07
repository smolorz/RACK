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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
package rack.main.tims;

public abstract class Tims extends Thread
{
    // Tims return types / reply message types
    public static final byte MSG_OK            = 0;
    public static final byte MSG_ERROR         = -1;
    public static final byte MSG_TIMEOUT       = -2;
    public static final byte MSG_NOT_AVAILABLE = -3;

    protected boolean        terminate         = false;
    public TimsDataRate      dataRate          = new TimsDataRate();

    public Tims()
    {
    }

    public void terminate()
    {
        terminate = true;
    }

    /**
     * Internal send method.
     * 
     * To send Tims messages use methods from TimsMbx.
     * 
     * @see TimsMbx#send(TimsMsg)
     * @see TimsMbx#send0(byte, int, byte, byte)
     * @see TimsMbx#send(byte, int, byte, byte, TimsMsg)
     * @see TimsMbx#sendReply0(byte, TimsMsg)
     * @see TimsMbx#sendReply(byte, TimsMsg, TimsMsg)
     */
    protected abstract void send(TimsMsg m) throws TimsException;

    /**
     * Internal receive method.
     * 
     * To receive Tims messages use methods from TimsMbx.
     * 
     * @see TimsMbx#receive(int)
     */
    protected abstract TimsRawMsg receive(TimsMbx mbx, int timeout) throws TimsException;

    public abstract TimsMbx mbxInit(int mbxName) throws TimsException;

    public abstract void mbxDelete(TimsMbx mbx) throws TimsException;

    public abstract void mbxClean(TimsMbx mbx) throws TimsException;

    public TimsDataRate getDataRate()
    {
        TimsDataRate reply;

        synchronized (dataRate)
        {
            long currentTime = System.currentTimeMillis();
            dataRate.endTime = currentTime;
            
            reply = new TimsDataRate(dataRate);
            
            dataRate.sendBytes = 0;
            dataRate.sendMessages = 0;
            dataRate.receivedBytes = 0;
            dataRate.receivedMessages = 0;
            dataRate.startTime = currentTime;
            dataRate.endTime = currentTime;
        }
        return reply;
    }
}
