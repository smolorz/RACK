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
#ifndef __RACK_NAME_H__
#define __RACK_NAME_H__

// RACK class names

#define TIMS                    0x00
#define GDOS                    0x01
#define GUI                     0x02

#define TEST                    0x10

#define CHASSIS                 0x11
#define ODOMETRY                0x12
#define POSITION                0x13
#define LADAR                   0x14
#define CAMERA                  0x15
#define GPS                     0x16
#define JOYSTICK                0x17
#define PILOT                   0x18
#define SCAN2D                  0x19

#define RACK_NAME_OFFSET        0x80

#define RACK_NAME_LOCAL_ID_RANGE          8
#define RACK_NAME_INSTANCE_ID_RANGE       8
#define RACK_NAME_CLASS_ID_RANGE          8
#define RACK_NAME_SYSTEM_ID_RANGE         8

class RackName {
    public:

        static unsigned int getSysMask(void)
        {
            unsigned int mask = (1 << RACK_NAME_SYSTEM_ID_RANGE) -1;
            return
              (mask << (RACK_NAME_LOCAL_ID_RANGE + RACK_NAME_INSTANCE_ID_RANGE + RACK_NAME_CLASS_ID_RANGE));
        }

        static unsigned int getClassMask(void)
        {
            unsigned int mask = (1 << RACK_NAME_CLASS_ID_RANGE) -1;
            return (mask << (RACK_NAME_LOCAL_ID_RANGE + RACK_NAME_INSTANCE_ID_RANGE));
        }

        static unsigned int getInstMask(void)
        {
            unsigned int mask = (1 << RACK_NAME_INSTANCE_ID_RANGE) -1;
            return (mask << (RACK_NAME_LOCAL_ID_RANGE));
        }

        static unsigned int getLocalMask(void)
        {
            return ((1 << RACK_NAME_LOCAL_ID_RANGE) -1);
        }

        static uint32_t create(uint32_t classID, uint32_t instID)
        {
            return (uint32_t)
                (((classID) << (RACK_NAME_LOCAL_ID_RANGE + RACK_NAME_INSTANCE_ID_RANGE)) |
                 ((instID)  <<  RACK_NAME_LOCAL_ID_RANGE));
        }

        static uint32_t create(uint32_t sysID, uint32_t classID,
                               uint32_t instID)
        {
            return (uint32_t)
                (((sysID)   << (RACK_NAME_LOCAL_ID_RANGE + RACK_NAME_INSTANCE_ID_RANGE +
                                RACK_NAME_CLASS_ID_RANGE)) |
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
                ((rackName >> (RACK_NAME_LOCAL_ID_RANGE + RACK_NAME_INSTANCE_ID_RANGE +
                               RACK_NAME_CLASS_ID_RANGE)) &
                ((1 << RACK_NAME_SYSTEM_ID_RANGE) -1) );
        }

        static uint32_t classId(uint32_t rackName)
        {
            return (uint32_t)
                ((rackName >> (RACK_NAME_LOCAL_ID_RANGE + RACK_NAME_INSTANCE_ID_RANGE)) &
                ((1 << RACK_NAME_CLASS_ID_RANGE) -1) );
        }

        static uint32_t instanceId(int rackName)
        {
            return (uint32_t)
                ((rackName >> (RACK_NAME_LOCAL_ID_RANGE)) &
                ((1 << RACK_NAME_INSTANCE_ID_RANGE) -1) );
        }

        static uint32_t localId(uint32_t rackName)
        {
            return (rackName & ( (1 << RACK_NAME_LOCAL_ID_RANGE) -1) );
        }

};

#endif // __RACK_NAME_H__
