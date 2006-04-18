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
 *      Marko Reimer <reimer@l3s.de>
 *
 */

#ifndef __CAMERA_DCAM_H__
#define __CAMERA_DCAM_H__

#include <main/rack_datamodule.h>

#include <drivers/camera_proxy.h>

#include <libraw1394/raw1394.h>//firewire bus
#include <libdc1394/dc1394_control.h>//iidc 1.30 or dcam standard

#include "ww_b4_calibration_parameter_lr2.h"

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
} iidc_format7_image_t;

// define module class
#define MODULE_CLASS_ID     CAMERA

#define MAX_PORTS    6 //in the new board
#define MAX_RESETS  10
#define FW_ERROR   -10

//######################################################################
//# class ChassisPioneerModule
//######################################################################

class CameraDcam : public DataModule{
  private:

    // your values
    //variables for common control
    int      format;
    int      bytesPerPixel;
    int      vValue;
    int      uValue;
    char     *dataBuffer;
    int      fps;

    //variables for parameter
    int      cameraGuid;
    int      mode;
    unsigned int 	lossRate;
    int      vValueSet;
    int      uValueSet;
    int      minHue;
    int      maxHue;
    int      shutterMult;
    int      gainMult;

    //variables needed for initial handling of the firewire bus system.
    int                     firewireNumPorts;

    //variable needed to initialise and handle dcam strucures
    raw1394handle_t 		porthandle[MAX_PORTS]; //handle to raw 1394 bus for each port
    nodeid_t 				camera_node; //cameras[MODULE_NUM];
    dc1394_cameracapture 	dc1394Camera;//[id] //camera type already defined
    int 					dc1394CameraPortNo; //cameraId2portNo[MODULE_NUM];

    //variables for dcam parameter
    unsigned int 			channel;
    unsigned int            frameRate;
    unsigned int 			speed;
    char* 					device;
    iidc_format7_image_t    format7image;

    int autoBrightness(camera_data_msg *dataPackage);
    int getFirewirePortnum(void);
    int findCameraByGuid(void);
    int setupCaptureFormat2(void);
    int setupCaptureFormat7(void);

  protected:

    // -> non realtime context
    int  moduleOn(void);
    void moduleOff(void);
    int  moduleLoop(void);
    int  moduleCommand(MessageInfo *msgInfo);

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
