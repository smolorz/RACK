/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2012 University of Hannover
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
 *      Sebastian Smolorz <smolorz@rts.uni-hannover.de>
 *		Ported module to libdc1394 v2
 *
 */

#ifndef __CAMERA_DCAM_H__
#define __CAMERA_DCAM_H__

#include <main/rack_data_module.h>

#include <drivers/camera_proxy.h>

#include <dc1394/dc1394.h>

//helper construct for format7 specification
typedef struct {
    int leftImagePosition;
    int topImagePosition;
    int width;
    int height;
    short bytesPerPacket;
    dc1394color_filter_t filter;
    dc1394video_mode_t mode;
    dc1394color_coding_t colorCodingId; // rggb=0; gbrg=1; grbg=2; bggr=3;
} camera_dcam_format7;

// define module class
#define MODULE_CLASS_ID     CAMERA

/**
 * Sensor driver for FireWire (IEEE1394) cameras with DCAM standard.
 *
 * @ingroup modules_camera
 */
class CameraDcam : public RackDataModule {
  private:

    //variables for common control
    int      bytesPerPixel;
    int      vValue;
    int      uValue;
    int      whitebalanceMode;
    int      whitebalanceCols;
    int      whitebalanceRows;
    char     *dataBuffer;
    int      fps;

    //variables for parameter
    uint64_t cameraGuid;
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

    dc1394_t                *dc1394_context;
    dc1394camera_t          *camera;
    dc1394camera_list_t     *camera_list;

    //variables for dcam parameter
    dc1394framerate_t       frameRate;
    dc1394speed_t           speed;
    unsigned int            width, height;
    camera_dcam_format7     format7image;
    camera_param_data       param;


    int autoBrightness(camera_data_msg *dataPackage);
    int autoWhitebalance(camera_data_msg *dataPackage);
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
    int  moduleCommand(RackMessage *msgInfo);

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
