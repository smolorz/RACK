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

#ifndef __CAMERA_V4L_H__
#define __CAMERA_V4L_H__

#include <main/rack_data_module.h>

#include <drivers/camera_proxy.h>

#include <unistd.h>
#include <sys/mman.h>
#include <linux/videodev.h>//Video4Linux
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
    
#include "creative_calibration_parameter_lr0.h"

// define module class
#define MODULE_CLASS_ID     CAMERA

typedef struct
{
    int dev;
    char devname[256];
    int mode;
    int currentFrame;
    struct video_capability vid_capability;
    struct video_window vid_window;
    struct video_picture vid_picture;
    struct video_mmap grab_buf;
    struct video_mbuf m_buf;
    unsigned char *grab_data;
    int grab_size;
} camera_v4l_config;


//######################################################################
//# class ChassisPioneerModule
//######################################################################

class CameraV4L : public RackDataModule {
  private:

    //variables for common control
    camera_v4l_config camera;

    //variables for parameter
    int      width;
    int      height;
    int      depth;
    int      mode;
    int      videoId;
    int      minHue;
    int      maxHue;
    int      gainMult;
    int      autoBrightnessSize;

    int      autoBrightness(camera_data_msg *dataPackage);

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
    CameraV4L();
    ~CameraV4L() {};

    // -> non realtime context
    int  moduleInit(void);
};

#endif // __CAMERA_V4L_H__
