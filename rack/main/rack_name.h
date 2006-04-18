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
#ifndef __RACK_NAME_H__
#define __RACK_NAME_H__

#define LOCAL_ID_RANGE          8
#define INSTANCE_ID_RANGE       8
#define CLASS_ID_RANGE          8
#define SYSTEM_ID_RANGE         8

// RACK COMPONENT CLASSES

// drivers
#define CAMERA                  0x29
#define CHASSIS                 0x13
#define GPS                     0x11
#define LADAR                   0x14
#define JOYSTICK                0x20
#define ODOMETRY                0x18

// navigation
#define PILOT                   0x43
#define POSITION                0x32

// perception
#define SCAN2D 			        0x30

// main
#define GDOS                    0xf0
#define GUI                     0x50

#define TEST                    0x70    // skel

// RACK_RTS
#define GYRO                    0x12
#define OBJECT_RECOGNITION      0x4B
#define SCAN3D           		0x45
#define SERVO_DRIVE             0x16
#define PATH                    0x34


// old ids ...
/*
#define CONTROL                 0x35
#define SYSTEM                  0x00
#define CAN_SERVER              0x01
#define SERIAL_SERVER           0x02
#define ROBOT_TIME_SERVER       0x04
#define WATCHDOG                0x05
#define SERIAL_TRIGGER          0x06
#define GPS_RAW                 0x09
#define COMPASS                 0x10
#define LORI                    0x15

#define WEBCAM                  0x17

#define SONAR                   0x19
#define STAIR_CLIMBER_IO        0x21
#define STAIR_CLIMBER_CONTROL   0x22
#define POWERPACK               0x27
#define TRIGGER                 0x28
#define FORCE_SENSOR            0xA0
#define FMP_MEASURING_UNIT      0x26
#define FMP_IO                  0x25
#define FMP_MEASUREMENT         0x24
#define FMP_LASER_SENSOR        0x23
#define MAP_MODULE              0x33
#define PATH                    0x34
#define CONTROL                 0x35
#define CHASSIS_PROTECTION      0x36
#define EXPLORATION             0x37
#define SEGMENTATION            0x38
#define MODEL_3D                0x46
#define ELECTROHAND             0x48
#define ROBOTARM                0x49
#define ROBOTARM_INTERFACE      0x4A
#define STAIR_CLIMBER           0x40
#define DATABASE                0x42
#define MAP_VIEW                0x44
#define MODULE_MONITOR          0x51
#define DATA_LOG                0x52
#define SCOPE                   0x53

#define FCB_CAM                 0x80
#define IO_REIBMO               0x81
#define MEASURE_BOX             0x82
#define ROBOTARM_DISPLAY        0x83
#define ROBOTARM_KALMAN         0x84
#define ROBOTARM_MAIN           0x85
#define ROBOTARM_CONTROLLER     0x86
#define LBR_FORCECONTROL        0x87
#define GDOS_EXTERN             0xf1
*/

class RackName {
    public:

        static char* classString(int rackName)
        {
            switch (RackName::classId(rackName))
            {
                // drivers
                case CAMERA:                return("Camera");
                case CHASSIS:               return("Chassis");
                case GPS:                   return("GPS");
                case JOYSTICK:              return("Joystick");
                case LADAR:                 return("Ladar");
                case ODOMETRY:              return("Odometry");

                // navigation
                case PILOT:                 return("Pilot");
                case POSITION:              return("Position");

                // perception
                case SCAN2D:                return("Scan2D");

                // main
                case GDOS:                  return("GDOS");
                case GUI:                   return("GUI");

                case TEST:                  return("Test");

                case GYRO:                  return("Gyro");
                case OBJECT_RECOGNITION:    return("ObjectRecognition");
                case PATH:                  return("Path");
                case SCAN3D:         		return("Scan3D");
                case SERVO_DRIVE:           return("ServoDrive");

// old ids
/*
        case CONTROL:               return("Control");
        case SYSTEM:                return("System");
        case CAN_SERVER:            return("CanServer");
        case SERIAL_SERVER:         return("SerialServer");
        case ROBOT_TIME_SERVER:     return("RobotTimeServer");
        case WATCHDOG :             return("Watchdog");
        case SERIAL_TRIGGER:        return("SerialTrigger");
        case GPS_RAW:               return("GPSRaw");
        case COMPASS:               return("Compass");
        case LORI:                  return("Lori");
        case WEBCAM:                return("Webcam");
        case SONAR:                 return("Sonar");
        case STAIR_CLIMBER_IO:      return("StairClimber IO");
        case STAIR_CLIMBER_CONTROL: return("StairClimber Control");
        case POWERPACK:             return("Powerpack");
        case TRIGGER:               return("Trigger");
        case FORCE_SENSOR:          return("ForceSensor");
        case FMP_MEASURING_UNIT:    return("MessKapsel");
        case FMP_IO:                return("InOut");
        case FMP_MEASUREMENT:       return("AblaufSt");
        case FMP_LASER_SENSOR:      return("TexturSensor");

        case MAP_MODULE:            return("MapModule");
        case CHASSIS_PROTECTION:    return("Chassis Protection");
        case EXPLORATION:           return("Exploration");
        case SEGMENTATION:          return("Segmentation");
        case MODEL_3D:              return("Model3D");
        case ELECTROHAND:           return("Electrohand");
        case ROBOTARM:              return("Robotarm");
        case ROBOTARM_INTERFACE:    return("Robotarm_Interface");
        case STAIR_CLIMBER:         return("StairClimber");
        case DATABASE:              return("Database");
        case MAP_VIEW:              return("Map_View");
        case MODULE_MONITOR:        return("ModuleMonitor");
        case DATA_LOG:              return("DataLog");
        case SCOPE:                 return("Scope");

        case GDOS_EXTERN:           return("GDOS EXTERN");
*/

                default:
                    return "unknown RackName";
            }
        }

        static unsigned int getSysMask(void)
        {
            unsigned int mask = (1 << SYSTEM_ID_RANGE) -1;
            return
              (mask << (LOCAL_ID_RANGE + INSTANCE_ID_RANGE + CLASS_ID_RANGE));
        }

        static unsigned int getClassMask(void)
        {
            unsigned int mask = (1 << CLASS_ID_RANGE) -1;
            return (mask << (LOCAL_ID_RANGE + INSTANCE_ID_RANGE));
        }

        static unsigned int getInstMask(void)
        {
            unsigned int mask = (1 << INSTANCE_ID_RANGE) -1;
            return (mask << (LOCAL_ID_RANGE));
        }

        static unsigned int getLocalMask(void)
        {
            return ((1 << LOCAL_ID_RANGE) -1);
        }

        static uint32_t create(uint32_t classID, uint32_t instID)
        {
            return (uint32_t)
                (((classID) << (LOCAL_ID_RANGE + INSTANCE_ID_RANGE)) |
                 ((instID)  <<  LOCAL_ID_RANGE));
        }

        static uint32_t create(uint32_t sysID, uint32_t classID,
                               uint32_t instID)
        {
            return (uint32_t)
                (((sysID)   << (LOCAL_ID_RANGE + INSTANCE_ID_RANGE +
                                CLASS_ID_RANGE)) |
                 create(classID, instID));
        }

        static uint32_t create(uint32_t sysID, uint32_t classID,
                               uint32_t instID, uint32_t locID)
        {
            return create(sysID, classID, instID) | locID;
        }

        static uint32_t systemId(uint32_t rackName)
        {
            return (uint32_t)
                ((rackName >> (LOCAL_ID_RANGE + INSTANCE_ID_RANGE +
                               CLASS_ID_RANGE)) &
                ((1 << SYSTEM_ID_RANGE) -1) );
        }

        static uint32_t classId(uint32_t rackName)
        {
            return (uint32_t)
                ((rackName >> (LOCAL_ID_RANGE + INSTANCE_ID_RANGE)) &
                ((1 << CLASS_ID_RANGE) -1) );
        }

        static uint32_t instanceId(int rackName)
        {
            return (uint32_t)
                ((rackName >> (LOCAL_ID_RANGE)) &
                ((1 << INSTANCE_ID_RANGE) -1) );
        }

        static uint32_t localId(uint32_t rackName)
        {
            return (rackName & ( (1 << LOCAL_ID_RANGE) -1) );
        }

};

#endif // __RACK_NAME_H__
