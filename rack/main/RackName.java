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
 *      Oliver Wulf <oliver.wulf@web.de>
 *
 */
package rack.main;

import java.util.Hashtable;

public class RackName
{
    //  RACK class names

    public static final int TIMS                = 0x00;
    public static final int GDOS                = 0x01;
    public static final int GUI                 = 0x02;
    public static final int TEST                = 0x10;

    public static final int CHASSIS             = 0x11;
    public static final int ODOMETRY            = 0x12;
    public static final int POSITION            = 0x13;
    public static final int LADAR               = 0x14;
    public static final int CAMERA              = 0x15;
    public static final int GPS                 = 0x16;
    public static final int JOYSTICK            = 0x17;
    public static final int PILOT               = 0x18;
    public static final int SCAN2D              = 0x19;
    public static final int DATALOG             = 0x20;    

    public static final int OFFSET              = 0x80;

    protected static final int LOCAL_ID_RANGE      = 8;
    protected static final int INSTANCE_ID_RANGE   = 8;
    protected static final int CLASS_ID_RANGE      = 8;
    protected static final int SYSTEM_ID_RANGE     = 8;

    protected static Hashtable<Integer,String> classStringTable;

    public static void initClassStringTable()
    {
        classStringTable = new Hashtable<Integer,String>();

        classStringTable.put(new Integer(TIMS), "Tims");
        classStringTable.put(new Integer(GDOS), "GDOS");
        classStringTable.put(new Integer(GUI), "GUI");
        classStringTable.put(new Integer(TEST), "Test");

        classStringTable.put(new Integer(CHASSIS), "Chassis");
        classStringTable.put(new Integer(ODOMETRY), "Odometry");
        classStringTable.put(new Integer(POSITION), "Position");
        classStringTable.put(new Integer(LADAR), "Ladar");
        classStringTable.put(new Integer(CAMERA), "Camera");
        classStringTable.put(new Integer(GPS), "Gps");
        classStringTable.put(new Integer(JOYSTICK), "Joystick");
        classStringTable.put(new Integer(PILOT), "Pilot");
        classStringTable.put(new Integer(SCAN2D), "Scan2d");
        classStringTable.put(new Integer(DATALOG), "Datalog");
    }

    public static String classString(int rackName)
    {
        if(classStringTable == null)
        {
            initClassStringTable();
        }

        int classId = RackName.classId(rackName);
        
        String classString = (String)classStringTable.get(new Integer(classId));
        
        if(classString == null)
        {
            classString = "unknown[" + Integer.toHexString(classId) + "]";
        }
        
        return classString;
    }

    public static String nameString(int rackName)
    {
        int instanceId = RackName.instanceId(rackName);
        
        return classString(rackName) + "(" + instanceId + ")";
    }

    public static String string(int rackName)
    {
        return nameString(rackName) + " [" + Integer.toHexString(rackName) + "]";
    }

    public static int create(int sysID, int classID, int instID, int locID) {
      return (int)
        (((sysID)   << (LOCAL_ID_RANGE + INSTANCE_ID_RANGE + CLASS_ID_RANGE)) |
         ((classID) << (LOCAL_ID_RANGE + INSTANCE_ID_RANGE)) |
         ((instID)  << (LOCAL_ID_RANGE)) |
         (locID));
    }

    public static int create(int classID, int instID, int locID) {
      return (int)
        (((classID) << (LOCAL_ID_RANGE + INSTANCE_ID_RANGE)) |
         ((instID)  << (LOCAL_ID_RANGE)) |
         (locID));
    }

    public static int create(int classID, int instID) {
      return (int)
        (((classID) << (LOCAL_ID_RANGE + INSTANCE_ID_RANGE)) |
         ((instID)  <<  LOCAL_ID_RANGE));
    }

    public static int systemId(int rackName) {
        return (int)
          ( (rackName >> (LOCAL_ID_RANGE + INSTANCE_ID_RANGE + CLASS_ID_RANGE)) &
            ( (1 << SYSTEM_ID_RANGE) -1) );
    }

    public static int classId(int rackName) {
        return (int)
          ( (rackName >> (LOCAL_ID_RANGE + INSTANCE_ID_RANGE)) &
            ( (1 << CLASS_ID_RANGE) -1) );
    }

    public static int instanceId(int rackName) {
        return (int)
          ( (rackName >> (LOCAL_ID_RANGE)) &
            ( (1 << INSTANCE_ID_RANGE) -1) );
    }

    public static int localId(int rackName) {
        return(rackName & ( (1 << LOCAL_ID_RANGE) -1) );
    }
}
