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
#ifndef __CAMERA_JPEG_H__
#define __CAMERA_JPEG_H__

#include <main/rack_data_module.h>

#include <drivers/camera_proxy.h>

#include <main/jpeg_data_dst_mem.h> //destination manager for memory destination

// define module class
#define MODULE_CLASS_ID     CAMERA



//######################################################################
//# class ChassisPioneerModule
//######################################################################

class CameraJpeg : public RackDataModule {
  private:

    // your values
    //variables for common control
    RTIME timeCount1s;
    uint8_t*                    rgbByteArray; //converted to char later on
    char*                       jpegBuffer;//char enforced by jpegLib
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;

    //variables for parameter
    int cameraInst;
    int quality;

    // additional mailboxes
    RackMailbox cameraMbx;
    RackMailbox workMbx;

    // proxies
    CameraProxy  *camera;

    camera_data_msg cameraInputMsg;


    void compressByteStream2JpegStream(uint8_t* byteStream, j_compress_ptr cinfo);

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
    CameraJpeg();
    ~CameraJpeg() {};

    // -> non realtime context
    int  moduleInit(void);
};

#endif // __CAMERA_JPEG_H__
