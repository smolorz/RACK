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
package rack.main.naming;

public class RackName
{
    private static final int LOCAL_ID_RANGE      = 8;
    private static final int INSTANCE_ID_RANGE   = 8;
    private static final int CLASS_ID_RANGE      = 8;
    private static final int SYSTEM_ID_RANGE     = 8;

    public static final int CAMERA              = 0x29;
    public static final int CHASSIS             = 0x13;
    public static final int GDOS                = 0xf0;
    public static final int GPS                 = 0x11;
    public static final int GUI                 = 0x50;
    public static final int GYRO                = 0x12;
    public static final int JOYSTICK            = 0x20;
    public static final int LADAR               = 0x14;
    public static final int MAP_VIEW            = 0x44;    
    public static final int OBJECT_RECOGNITION  = 0x4B;
    public static final int ODOMETRY            = 0x18;
    public static final int PATH                = 0x34;
    public static final int PILOT               = 0x43;
    public static final int POSITION            = 0x32;
    public static final int SCAN2D              = 0x30;
    public static final int SCAN3D              = 0x45;    
    public static final int SERVO_DRIVE         = 0x16;
    public static final int MCL                 = 0x33;

/* OLD IDS
    public static final int SYSTEM              = 0x00;
    public static final int CAN_SERVER          = 0x01;
    public static final int SERIAL_SERVER       = 0x02;
    public static final int ROBOT_TIME_SERVER   = 0x04;
    public static final int WATCHDOG            = 0x05;
    public static final int SERIAL_TRIGGER      = 0x06;
    public static final int GPS_RAW             = 0x09;
    public static final int COMPASS             = 0x10;
    public static final int LORI                = 0x15;
    public static final int WEBCAM              = 0x17;
    public static final int SONAR               = 0x19;
    public static final int STAIR_CLIMBER_IO    = 0x21;
    public static final int STAIR_CLIMBER_CONTROL = 0x22;
    public static final int POWERPACK           = 0x27;
    public static final int TRIGGER             = 0x28;
    public static final int FORCE_SENSOR        = 0xA0;
    public static final int FMP_MEASURING_UNIT  = 0x26;
    public static final int FMP_IO              = 0x25;
    public static final int FMP_MEASUREMENT     = 0x24;
    public static final int FMP_LASER_SENSOR    = 0x23;

    public static final int MAP_MODULE          = 0x33;
    public static final int CONTROL             = 0x35;
    public static final int CHASSIS_PROTECTION  = 0x36;
    public static final int EXPLORATION         = 0x37;
    public static final int SEGMENTATION        = 0x38;

    public static final int MODEL_3D            = 0x46;

    public static final int ELECTROHAND         = 0x48;
    public static final int ROBOTARM            = 0x49;
    public static final int ROBOTARM_INTERFACE  = 0x4A;

    // Motorsteuerung der Hebeantriebe des Treppensteigers
    public static final int STAIR_CLIMBER       = 0x40;
    public static final int DATABASE            = 0x42;


    public static final int MODULE_MONITOR      = 0x51;
    public static final int DATA_LOG            = 0x52;
    public static final int SCOPE               = 0x53;

    public static final int TEST                = 0x70;

    //* MZH Module
    public static final int FCB_CAM             = 0x80;
    public static final int IO_REIBMO           = 0x81;
    public static final int MEASURE_BOX         = 0x82;
    public static final int ROBOTARM_DISPLAY    = 0x83;
    public static final int ROBOTARM_KALMAN     = 0x84;
    public static final int ROBOTARM_MAIN       = 0x85;
    public static final int ROBOTARM_CONTROLLER = 0x86;
    public static final int LBR_FORCECONTROL    = 0x87;

    // Das GDOS-System auf RTAI-RTOS Rechnern
    public static final int GDOS_EXTERN        = 0xf1;
*/

    public static String classString(int rackName) {
        switch(RackName.classId(rackName)) {

        case CAMERA:
            return("Camera");
        case CHASSIS:
            return("Chassis");
        case GDOS:
            return("GDOS");
        case GPS:
            return("Gps");
        case GUI:
            return("GUI");
        case GYRO:
            return("Gyro");
        case JOYSTICK:
            return("Joystick");
        case LADAR:
            return("Ladar");
        case MAP_VIEW:
            return("MapView");            
        case OBJECT_RECOGNITION:
            return("ObjectRecognition");
        case ODOMETRY:
            return("Odometry");
        case PATH:
            return("Path");
        case PILOT:
            return("Pilot");
        case POSITION:
            return("Position");
        case SCAN2D:
            return("Scan2D");
        case SCAN3D:
            return("Scan3D");
        case SERVO_DRIVE:
            return("ServoDrive");
        case MCL:
            return("MCL");


 /*
        case SYSTEM:
            return("System");

        case CAN_SERVER:
            return("CanServer");
        case SERIAL_SERVER:
            return("SerialServer");
        case ROBOT_TIME_SERVER:
            return("RobotTimeServer");
        case WATCHDOG :
            return("Watchdog");
        case SERIAL_TRIGGER:
            return("SerialTrigger");
        case GPS_RAW:
            return("GPSRaw");
        case COMPASS:
            return("Compass");
        case LORI:
            return("Lori");
        case WEBCAM:
            return("Webcam");
        case SONAR:
            return("Sonar");
        case STAIR_CLIMBER_IO:
            return("StairClimber IO");
        case STAIR_CLIMBER_CONTROL:
            return("StairClimber Control");
        case POWERPACK:
            return("Powerpack");
        case TRIGGER:
            return("Trigger");
        case FORCE_SENSOR:
            return("ForceSensor");
        case FMP_MEASURING_UNIT:
            return("MessKapsel");
        case FMP_IO:
            return("InOut");
        case FMP_MEASUREMENT:
            return("AblaufSt");
        case FMP_LASER_SENSOR :
            return ("TexturSensor");
        case MAP_MODULE:
            return("MapModule");
        case CONTROL:
            return("Control");
        case CHASSIS_PROTECTION:
            return("Chassis Protection");
        case EXPLORATION:
            return("Exploration");
        case SEGMENTATION:
            return("Segmentation");
        case MODEL_3D:
            return("Model3D");
        case ELECTROHAND:
            return("Electrohand");
        case ROBOTARM:
            return("Robotarm");
        case ROBOTARM_INTERFACE:
            return("Robotarm_Interface");

        case STAIR_CLIMBER:
            return("StairClimber");
        case DATABASE:
            return("Database");
        case MODULE_MONITOR:
            return("ModuleMonitor");
        case DATA_LOG :
            return ("DataLog");
        case SCOPE :
            return ("Scope");
        case TEST:
            return("Test");

        case GDOS_EXTERN:
            return("GDOS EXTERN");
*/

        default:
            return("unknown[" + rackName + "]");
        }
    }

    public static String nameString(int rackName) {
        return(RackName.classString(rackName) + "(" + RackName.instanceId(rackName) + ")");
    }

    public static String string(int rackName) {
        return(RackName.nameString(rackName) + " [" + Integer.toHexString(rackName) + "]");
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

/*
    public static int create(int classId, int instanceId) {
        return(classId * 0x10000 + instanceId * 0x100);
    }

    public static int create(int classId, int instanceId, int localId) {
        return(classId * 0x10000 + instanceId * 0x100 + localId);
    }

    public static int create(int systemId, int classId, int instanceId, int localId) {
        return(systemId * 0x1000000 + classId * 0x10000 + instanceId * 0x100 + localId);
    }

    public static int systemId(int rackName) {
        return((rackName >> 24) & 0xff);
    }

    public static int classId(int rackName) {
        return((rackName >> 16) & 0xff);
    }

    public static int instanceId(int rackName) {
        return((rackName >> 8) & 0xff);
    }

    public static int localId(int rackName) {
        return(rackName & 0xff);
    }
*/

}
