/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Matthias Hentschel      <hentschel@rts.uni-hannover.de>
 *
 */
#ifndef __LADAR_IBEO_LUX_H__
#define __LADAR_IBEO_LUX_H__

#include <main/rack_data_module.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <drivers/ladar_proxy.h>
#include <perception/obj_recog_proxy.h>
#include <main/defines/point2d.h>

#define MODULE_CLASS_ID         LADAR

#define LADAR_IBEO_LUX_CMD_MAX              100
#define LADAR_IBEO_LUX_MESSAGE_SIZE_MAX     51000

#define LADAR_IBEO_LUX_SCAN_POINT_MAX       10000

#define LADAR_IBEO_LUX_MAGIC_WORD           0xAFFEC0C2
#define LADAR_IBEO_LUX_SCAN_DATA            0x2202
#define LADAR_IBEO_LUX_OBJ_DATA             0x2221

#define LADAR_IBEO_LUX_MAX_RANGE            200000      // 200m

// time data type
typedef struct
{
    uint32_t                seconds;            // seconds since 1.1.1900 - 0:00:00
    uint32_t                secondsFrac;        // fractional seconds in 2-32 s
} __attribute__((packed)) ladar_ibeo_lux_ntp64;

// point2d
typedef struct
{
    int16_t                 x;                  // x-part/coordinate of this value/point
    int16_t                 y;                  // y-part/coordinate of this value/point
} __attribute__((packed)) ladar_ibeo_lux_point2d;

// size2d
typedef struct
{
    uint16_t                x;                  // x-value/size/width
    uint16_t                y;                  // y-value/size/width
} __attribute__((packed)) ladar_ibeo_lux_size2d;


// scan data scan point
typedef struct
{
    uint8_t                 layerEcho;          // scan layer of this point (uint4_H)
                                                // echo number of this point (uint4_L)
    uint8_t                 flags;              // 0x01:    transparent point
                                                // 0x02:    rain/snow/spray/fog/...
                                                // 0x04:    ground
                                                // 0x08:    dirt
                                                // 0xF0:    reserved
    int16_t                 angle;              // angle of this point in angle ticks
    uint16_t                distance;           // distance of this point in cm
    uint16_t                pulseWidth;         // detected with of hte echo pulse in cm
    uint16_t                reserved;           // -
} __attribute__((packed)) ladar_ibeo_lux_scan_data_point;

// object
typedef struct
{
    uint16_t                id;                 // tracking id
    uint16_t                age;                // number of scans this object has been tracked for
    uint16_t                agePredict;         // number of scans this object has been predicted
                                                // without measurement update
    uint16_t                relTime;            // timestamp of the object relative to the scan
                                                // start time in ms
    ladar_ibeo_lux_point2d  refPoint;           // object reference point (center of gravity) in cm
    ladar_ibeo_lux_point2d  refPointSigma;      // standard deviation of the estimated reference
                                                // point
    ladar_ibeo_lux_point2d  closestPoint;       // unfiltered closest object point in cm
    ladar_ibeo_lux_point2d  boundingBoxCenter;  // center of the bounding box in the reference
                                                // coordinate system in cm
    ladar_ibeo_lux_size2d   boundingBoxSize;    // size of the bounding box in the reference
                                                // coordinate system in cm
    ladar_ibeo_lux_point2d  objBoxCenter;       // box center in the reference coordinate system
                                                // in cm
    ladar_ibeo_lux_size2d   objBoxSize;         // box size in the reference coordinate system in cm
    int16_t                 objBoxOrientation;  // box orientation in the reference coordinate
                                                // system in 1/32deg
    ladar_ibeo_lux_point2d  velAbs;             // absolute velocity in cm/s including ego motion,
                                                // set to 0x8000 if invalid
    ladar_ibeo_lux_size2d   velAbsSigma;        // standard deviation of the estimated absolut
                                                // velocity in cm/s
    ladar_ibeo_lux_point2d  velRel;             // relative velocity of the object without ego
                                                // motion in cm/s
    uint16_t                classification;     // classification of the object:
                                                //  0:  unclassified
                                                //  1:  unknown small
                                                //  2:  unknown big
                                                //  3:  pedestrian
                                                //  4:  bike
                                                //  5:  car
                                                //  6:  truck
    uint16_t                classAge;           // number of scans this object has bend classified
                                                // as current class
    uint16_t                classCertainty;     // the higher this value, the more reliable is the
                                                // assigned object class
    uint16_t                contourPointNum;    // number of contour points
} __attribute__((packed)) ladar_ibeo_lux_obj;


// ladar header
typedef struct
{
    uint32_t                prevMessageSize;    // helps to navigate backwards through a file,
                                                // unused in live data
    uint32_t                messageSize;        // size of message content without this header
    uint8_t                 reserved;           // -
    uint8_t                 deviceId;           // id of the connected device, unused in data
                                                // received directly from ibeo LUX sensors
    uint16_t                dataType;           // data type within this message
    ladar_ibeo_lux_ntp64    ntpTime;            // time when this message was created
} __attribute__((packed)) ladar_ibeo_lux_header;

// scan data
typedef struct
{
    uint16_t                        scanNum;    //
    uint16_t                        scannerStatus;
    uint16_t                        syncPhaseOffset;
    ladar_ibeo_lux_ntp64            scanStartTime;
    ladar_ibeo_lux_ntp64            scanEndTime;
    uint16_t                        angleTicksPerRot;
    int16_t                         startAngle;
    int16_t                         endAngle;
    uint16_t                        scanPoints;
    int16_t                         posYaw;
    int16_t                         posPitch;
    int16_t                         posRoll;
    int16_t                         posX;
    int16_t                         posY;
    int16_t                         posZ;
    uint16_t                        reserved;
    ladar_ibeo_lux_scan_data_point  point[LADAR_IBEO_LUX_SCAN_POINT_MAX];
} __attribute__((packed)) ladar_ibeo_lux_scan_data;

// object data
typedef struct
{
    ladar_ibeo_lux_ntp64            timestamp;  // timestamp of the first measurement of the scan
                                                // these objects are updated with
    uint16_t                        objNum;     // number of objects
} __attribute__((packed)) ladar_ibeo_lux_obj_data;


typedef struct
{
    ladar_ibeo_lux_header   head;
    uint8_t                 data[LADAR_IBEO_LUX_CMD_MAX];
} __attribute__((packed)) ladar_ibeo_lux_command;


// data output
typedef struct
{
    ladar_data          data;
    ladar_point         point[LADAR_DATA_MAX_POINT_NUM];
} __attribute__((packed)) ladar_data_msg;

typedef struct
{
    obj_recog_data      data;
    obj_recog_object    object[OBJ_RECOG_OBJECT_MAX];
} __attribute__((packed)) obj_recog_data_msg;


// parsing functions
static inline void parseLadarIbeoLuxNtp64(ladar_ibeo_lux_ntp64 *data)
{
    data->seconds         = __be32_to_cpu(data->seconds);
    data->secondsFrac     = __be32_to_cpu(data->secondsFrac);
}

static inline void parseLadarIbeoLuxHeader(ladar_ibeo_lux_header *data)
{
    data->prevMessageSize   = __be32_to_cpu(data->prevMessageSize);
    data->messageSize       = __be32_to_cpu(data->messageSize);
    data->dataType          = __be16_to_cpu(data->dataType);
    parseLadarIbeoLuxNtp64(&data->ntpTime);
}

//######################################################################
//# class LadarIbeoLux
//######################################################################
class LadarIbeoLux : public RackDataModule {
    private:
        char                        *ladarIp;
        int                         ladarPort;
        int                         objRecogBoundInst;
        int                         objRecogContourInst;

        int                         tcpSocket;
        struct sockaddr_in          tcpAddr;

        double                      fracFactor;
        int                         dataRate;
        int                         dataRateCounter;
        rack_time_t                 dataRateStartTime;

        ladar_ibeo_lux_header       ladarHeader;
        uint8_t                     ladarData[LADAR_IBEO_LUX_MESSAGE_SIZE_MAX];

        ladar_ibeo_lux_command      ladarCommand;
        ladar_ibeo_lux_command      ladarCommandReply;

        obj_recog_data_msg          objRecogBoundData;
        obj_recog_data_msg          objRecogContourData;

        // mailboxes
        RackMailbox                 workMbx;

        // proxies
        ObjRecogProxy               *objRecogBound;
        ObjRecogProxy               *objRecogContour;

        uint32_t                    objRecogBoundMbxAdr;
        uint32_t                    objRecogContourMbxAdr;


    protected:

        // -> realtime context
        int  moduleOn(void);
        void moduleOff(void);
        int  moduleLoop(void);
        int  moduleCommand(message_info *msgInfo);

        // -> non realtime context
        void moduleCleanup(void);

        int  recvLadarHeader(ladar_ibeo_lux_header *data, rack_time_t *recordingTime,
                             unsigned int retryNumMax);
        int  recvLadarData(void *data, unsigned int messageSize);

    public:

        // constructor und destructor
        LadarIbeoLux();
        ~LadarIbeoLux() {};

        // -> non realtime context
        int moduleInit(void);

};

#endif // __LADAR_IBEO_LUX_H__
