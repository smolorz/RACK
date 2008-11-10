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
 *      Marko Reimer <reimer@rts.uni-hannover.de>
 *
 */

#ifndef __CAMERA_DCAM_H__
#define __CAMERA_DCAM_H__

#include <main/rack_data_module.h>

#include <drivers/camera_proxy.h>

#include <libraw1394/raw1394.h>//firewire bus
#include <libdc1394/dc1394_control.h>//iidc 1.30 or dcam standard

//helper construct for format7 specification
typedef struct {
    int leftImagePosition;
    int topImagePosition;
    int width;
    int height;
    short bytesPerPacket;
    unsigned int colorFilterId;
    short mode;
    short colorCodingId; // rggb=0; gbrg=1; grbg=2; bggr=3;
} camera_dcam_format7;

// define module class
#define MODULE_CLASS_ID     CAMERA

#define MAX_PORTS    6 //in the new board
#define MAX_RESETS  10
#define FW_ERROR   -10

//######################################################################
//# class ChassisPioneerModule
//######################################################################

class CameraDcam : public RackDataModule {
  private:

    // your values
    //variables for common control
    int      format;
    int      bytesPerPixel;
    int      vValue;
    int      uValue;
    int      whitebalanceMode;
    int      whitebalanceCols;
    int      whitebalanceRows;
    char     *dataBuffer;
    int      fps;

    //variables for parameter
    int      cameraGuid;
    int      mode;
    unsigned int lossRate;
    int      vValueSet;
    int      uValueSet;
    int      minHue;
    int      maxHue;
    int      shutterMult;
    int      gainMult;
    int      autoBrightnessSize;
    char     *intrParFile;
    char     *extrParFile;

    //variables needed for initial handling of the firewire bus system.
    int                     firewireNumPorts;

    //variable needed to initialise and handle dcam strucures
    raw1394handle_t         porthandle[MAX_PORTS]; //handle to raw 1394 bus for each port
    nodeid_t                camera_node; //cameras[MODULE_NUM];
    dc1394_cameracapture    dc1394Camera;//[id] //camera type already defined
    int                     dc1394CameraPortNo; //cameraId2portNo[MODULE_NUM];

    //variables for dcam parameter
    unsigned int            channel;
    unsigned int            frameRate;
    unsigned int            speed;
    const char*             device;
    camera_dcam_format7     format7image;
    camera_param_data       param;


    int autoBrightness(camera_data_msg *dataPackage);
    int autoWhitebalance(camera_data_msg *dataPackage);
    int getFirewirePortnum(void);
    int findCameraByGuid(void);
    int setupCaptureFormat2(void);
    int setupCaptureFormat7(void);
    int loadCameraParameter(camera_param_data *parData, char *intrParFilename, const int lossrate, char *extrParFilename);
    int parseParameterfile(char *filenameBuffer, camera_param_data *parData);

  protected:

    // -> non realtime context
    int  moduleOn(void);
    void moduleOff(void);
    int  moduleLoop(void);
    int  moduleCommand(message_info *msgInfo);

    // -> non realtime context
    void moduleCleanup(void);

  public:

    // constructor und destructor
    CameraDcam();
    ~CameraDcam() {};

    // -> non realtime context
    int  moduleInit(void);
};

#endif // __CAMERA_DCAM_H__
