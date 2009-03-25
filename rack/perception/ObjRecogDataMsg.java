/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2007 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Marko Reimer  <reimer@rts.uni-hannover.de>
 *
 */
package rack.perception;

import java.io.*;

import rack.main.RackProxy;
import rack.main.defines.Position3d;
import rack.main.tims.*;

public class ObjRecogDataMsg extends TimsMsg {

    public int                       recordingTime = 0; // 4 Bytes
    public Position3d 				 refPos 	   = new Position3d();  
    public int                       objectNum     = 0; // 4 Bytes
    public ObjRecogObject[]          object;

    public int getDataLen() {
        return (8 + Position3d.getDataLen() + objectNum * ObjRecogObject.getDataLen());
    }

    public ObjRecogDataMsg()
    {
        msglen = HEAD_LEN + getDataLen();
    }
    
    public ObjRecogDataMsg(TimsRawMsg p) throws TimsException 
    {
        readTimsRawMsg(p);
    }
    

    public boolean checkTimsMsgHead() 
    {
        if (type == RackProxy.MSG_DATA) 
        {
            return true;
        } 
        else 
        {
            return false;
        }
    }

    public void readTimsMsgBody(InputStream in) throws IOException {
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
        refPos.readData(dataIn);
        objectNum = dataIn.readInt();
        
        object = new ObjRecogObject[objectNum];

        for (int i = 0; i < objectNum; i++) {
            object[i] = new ObjRecogObject(dataIn);
        }

        bodyByteorder = BIG_ENDIAN;
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException 
    {
        DataOutputStream dataOut = new DataOutputStream(out);
        dataOut.writeInt(recordingTime);
        refPos.writeData(dataOut);
        dataOut.writeInt(objectNum);

        for (int i = 0; i < objectNum; i++) 
        {
            object[i].writeDataOut(dataOut);
        }
    }
}
