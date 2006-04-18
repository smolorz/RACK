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
#ifndef __CAMERA_FILE_H__
#define __CAMERA_FILE_H__

#include <main/rack_datamodule.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <drivers/camera_proxy.h>

using namespace std;

// define module class
#define MODULE_CLASS_ID     CAMERA


//const unsigned char turnTemplate[]     = {0xFA, 0xFB, 6, 21, 0, 0, 0, 0, 0};

//######################################################################
//# class CameraFileModule
//######################################################################

class CameraFile : public DataModule{
  private:

    // your values
    ifstream 	imageFileList;
    ifstream 	imageFile;
    string 		line;
    string      valueString;
    string      imageFileNameSkel;
    string      imageFileListName;
    int32_t 	imageCounter;
    int32_t		loopCounter;
    int32_t		width;
    int32_t 	depth;
    int32_t 	height;
    int32_t		mode;
    int32_t		colorFilterId;
    int32_t   * imageRecordingtimeArray;

    string intToString(int n);
    
  protected:

    // -> realtime context
    int  moduleOn(void);
    void moduleOff(void);
    int  moduleLoop(void);
    int  moduleCommand(MessageInfo *msgInfo);

    // -> non realtime context
    void moduleCleanup(void);

  public:
    // constructor und destructor
    CameraFile();
    ~CameraFile() {};

    // -> non realtime context
    int  moduleInit(void);

};

#endif // __CAMERA_FILE_H__
