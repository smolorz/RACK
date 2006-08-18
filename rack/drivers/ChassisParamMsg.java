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

import java.io.*;
import rack.main.tims.msg.*;
import rack.main.tims.exceptions.*;
import rack.main.tims.streams.*;

public class ChassisParamMsg extends TimsMsg
{

    // message
    public int vxMax = 0;
    public int vyMax = 0;
    public int vxMin = 0;
    public int vyMin = 0;
    public int axMax = 0;
    public int ayMax = 0;

    public float omegaMax = 0.0f;
    public int minTurningRadius = 0;
    
    public float breakConstant = 0.0f;
    public int safetyMargin = 0;
    public int safetyMarginMove = 0;
    public int comfortMargin = 0;

    public int boundaryFront = 0;
    public int boundaryBack = 0;
    public int boundaryLeft = 0;
    public int boundaryRight = 0;

    public int wheelBase = 0;
    public int wheelRadius = 0;
    public int trackWidth = 0;

    public float pilotParameterA = 0;
    public float pilotParameterB = 0;
    public int   pilotVTransMax  = 0;

    
    public int getDataLen()
    {
        return (88);
    }

    public ChassisParamMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }

    public ChassisParamMsg(TimsDataMsg p) throws MsgException
    {
        readTimsDataMsg(p);
    }

    protected boolean checkTimsMsgHead()
    {
        /*if ((type == RackMsgType.MSG_DATA)
                && (msglen == headLen + getDataLen()))*/
            if (msglen == HEAD_LEN + getDataLen())            
        {
            return (true);
        }
        else
        {
            return (false);
        }
    }

    protected void readTimsMsgBody(InputStream in) throws IOException
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

        vxMax = dataIn.readInt();
        vyMax = dataIn.readInt();
        vxMin = dataIn.readInt();
        vyMin = dataIn.readInt();
        axMax = dataIn.readInt();
        ayMax = dataIn.readInt();
        omegaMax = dataIn.readFloat();
        minTurningRadius = dataIn.readInt();
        breakConstant = dataIn.readFloat();
        safetyMargin = dataIn.readInt();
        safetyMarginMove = dataIn.readInt();
        comfortMargin = dataIn.readInt();
        boundaryFront = dataIn.readInt();
        boundaryBack = dataIn.readInt();
        boundaryLeft = dataIn.readInt();
        boundaryRight = dataIn.readInt();
        wheelBase = dataIn.readInt();
        wheelRadius = dataIn.readInt();
        trackWidth = dataIn.readInt();
        pilotParameterA = dataIn.readFloat();
        pilotParameterB = dataIn.readFloat();
        pilotVTransMax  = dataIn.readInt();        

        bodyByteorder = BIG_ENDIAN;
    }

    protected void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(vxMax);
        dataOut.writeInt(vyMax);
        dataOut.writeInt(vxMin);
        dataOut.writeInt(vyMin);
        dataOut.writeInt(axMax);
        dataOut.writeInt(ayMax);
        dataOut.writeFloat(omegaMax);
        dataOut.writeInt(minTurningRadius);
        dataOut.writeFloat(breakConstant);
        dataOut.writeInt(safetyMargin);
        dataOut.writeInt(safetyMarginMove);
        dataOut.writeInt(comfortMargin);
        dataOut.writeInt(boundaryFront);
        dataOut.writeInt(boundaryBack);
        dataOut.writeInt(boundaryLeft);
        dataOut.writeInt(boundaryRight);
        dataOut.writeInt(wheelBase);
        dataOut.writeInt(wheelRadius);
        dataOut.writeInt(trackWidth);
        dataOut.writeFloat(pilotParameterA);
        dataOut.writeFloat(pilotParameterB);
        dataOut.writeInt(pilotVTransMax);        
    }

}
