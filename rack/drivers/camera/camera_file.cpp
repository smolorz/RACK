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

// include own header file
#include "camera_file.h"

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE       0
#define INIT_BIT_LISTFILE          1
#define INIT_BIT_LIST_CREATED      2


//
// data structures
//

CameraFile *p_inst;



argTable_t argTab[] = {

  { 0,"",0,0,""}                                  // last entry
};

/*******************************************************************************
 *   !!! REALTIME CONTEXT !!!
 *
 *   moduleOn,
 *   moduleOff,
 *   moduleLoop,
 *   moduleCommand,
 *
 *   own realtime user functions
 ******************************************************************************/
 int  CameraFile::moduleOn(void)
{
    loopCounter = 0;

    return DataModule::moduleOn();   // have to be last command in moduleOn();
}

// realtime context
void CameraFile::moduleOff(void)
{
    DataModule::moduleOff();         // have to be first command in moduleOff();
}

// realtime context
int  CameraFile::moduleLoop(void)
{
//    int j;
    camera_data_msg   *p_data     = NULL;
    uint32_t          datalength = 0;
    string            imageFileName;

    // get datapointer from rackdatabuffer
    p_data = (camera_data_msg *)getDataBufferWorkSpace();

    p_data->data.recordingTime = imageRecordingtimeArray[loopCounter];
    datalength = width * height * depth / 8 + sizeof(camera_data);

    p_data->data.width  = width;
    p_data->data.height = height;
    p_data->data.depth  = depth;
    p_data->data.mode   = mode;
    p_data->data.colorFilterId = colorFilterId;

    //open image file and copy content to data buffer
    GDOS_DBG_INFO("Opening image file ... \n");

    imageFileName.assign(imageFileNameSkel);
    imageFileName.append(intToString(imageRecordingtimeArray[loopCounter]));
    imageFileName.append(".rcc");

    imageFile.open(imageFileName.c_str(), ios::in);
    if (!imageFile.is_open())
    {
//        GDOS_DBG_INFO("ERROR opening image file " + imageFileName + "\n");
        GDOS_DBG_INFO("ERROR opening image file no:%i. \n",loopCounter);
    }

/*  for (j = 0; j < imageSize; j++)
    {
        imageFile.get(dataPackage->body.byteStream[j]);
    }*/

    imageFile.get((char *)p_data->byteStream, width * height * depth / 8);

    imageFile.close();

    loopCounter ++;
    if (loopCounter > imageCounter)
        loopCounter = 0;

    // put data buffer slot (and send it to all listener)
    if (datalength > 0 && datalength <= getDataBufferMaxDataSize() )
    {
        putDataBufferWorkSpace( datalength );
        return 0;
    }

    return -ENOSPC;
}

int  CameraFile::moduleCommand(message_info *msgInfo)
{
    // not for me -> ask DataModule
    return DataModule::moduleCommand(msgInfo);
}

 /*******************************************************************************
 *   !!! NON REALTIME CONTEXT !!!
 *
 *   moduleInit,
 *   moduleCleanup,
 *   Constructor,
 *   Destructor,
 *   main,
 *
 *   own non realtime user functions
 ******************************************************************************/

 int  CameraFile::moduleInit(void)
{
    int ret;
    string::size_type loc;
    string::size_type locbeg;
    string::size_type locend;
    const char *valueString;

    // call DataModule init function (first command in init)
    ret = DataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    //open file with imageList

    GDOS_DBG_INFO("Opening image file list... \n");
    imageFileList.open(imageFileListName.c_str());
    if (!imageFileList.is_open())
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_LISTFILE);

    //parseImageFileList and image parameter
    imageCounter = 0;
    GDOS_DBG_INFO("parsing number of images... \n");

    while (! imageFileList.eof() )
    {
      getline (imageFileList,line);

      if (imageCounter == 0)
      {
         loc = line.find( "width", 0 );
         locbeg = line.find( ":", loc );
         locend = line.find( " ", loc );

         GDOS_DBG_INFO("extracted width: ");

         if( (locbeg != string::npos ) && (locend != string::npos ))
         {
             valueString = line.substr(locbeg + 14, locend).c_str();
             width = atoi(valueString);
             GDOS_DBG_INFO("%i \n", atoi(valueString));
         }

         loc = line.find( "height", 0 );
         locbeg = line.find( ":", loc );
         locend = line.find( " ", loc );

         GDOS_DBG_INFO("extracted height: \n");

         if( (locbeg != string::npos ) && (locend != string::npos ))
         {
             valueString = line.substr(locbeg + 14, locend).c_str();
             height = atoi(valueString);
             GDOS_DBG_INFO("%i \n", atoi(valueString));
         }

         loc = line.find( "depth", 0 );
         locbeg = line.find( ":", loc );
         locend = line.find( " ", loc );

         GDOS_DBG_INFO("extracted depth: \n");

         if( (locbeg != string::npos ) && (locend != string::npos ))
         {
             valueString = line.substr(locbeg + 14, locend).c_str();
             depth = atoi(valueString);
             GDOS_DBG_INFO("%i \n", atoi(valueString));
         }

         loc = line.find( "mode", 0 );
         locbeg = line.find( ":", loc );
         locend = line.find( " ", loc );

         GDOS_DBG_INFO("extracted mode: \n");

         if( (locbeg != string::npos ) && (locend != string::npos ))
         {
             valueString = line.substr(locbeg + 14, locend).c_str();
             mode = atoi(valueString);
             GDOS_DBG_INFO("%i \n", atoi(valueString));
         }
         loc = line.find( "colorFilterId", 0 );
         locbeg = line.find( ":", loc );
         locend = line.find( " ", loc );

         GDOS_DBG_INFO("extracted colorFilterId: \n");

          if( (locbeg != string::npos ) && (locend != string::npos ))
         {
             valueString = line.substr(locbeg + 14, locend).c_str();
             colorFilterId = atoi(valueString);
             GDOS_DBG_INFO("%i \n", atoi(valueString));
         }
      }
      imageCounter ++;
    }

    GDOS_DBG_INFO("creating recordingtime array... \n");

    imageRecordingtimeArray = new int [imageCounter];

    initBits.setBit(INIT_BIT_LIST_CREATED);

    imageFileList.seekg(0, ios::beg); //jump back to start of file

    //parse file content to array
    while (! imageFileList.eof() )
    {
      getline (imageFileList,line);
      string::size_type locbeg = line.find( "recordingtime", 0 );
      string::size_type locend = line.find( " ", 0 );

      GDOS_DBG_INFO("extracted recording time: \n");

      if( (locbeg != string::npos ) && (locend != string::npos ))
      {
         valueString = line.substr(locbeg + 14, locend).c_str();
         imageRecordingtimeArray[imageCounter] = atoi(valueString);
         GDOS_DBG_INFO("%i \n", atoi(valueString));
      }
      imageCounter ++;
    }

    imageFileList.close();
    initBits.clearBit(INIT_BIT_LISTFILE);

    return 0;

init_error:
    // !!! call local cleanup function !!!
    CameraFile::moduleCleanup();
    return ret;
}

// non realtime context
void CameraFile::moduleCleanup(void)
{
    GDOS_DBG_INFO("Cleaning image recordingtime array... \n");

    if (initBits.testAndClearBit(INIT_BIT_LIST_CREATED))
    {
        delete [] imageRecordingtimeArray;
    }

    if (initBits.testAndClearBit(INIT_BIT_LISTFILE))
    {
        imageFileList.close();
    }

        // call DataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        DataModule::moduleCleanup();
    }
}

CameraFile::CameraFile()
      : DataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s cmdtask error sleep time
                    5000000000llu,    // 5s datatask error sleep time
                     100000000llu,    // 100ms datatask disable sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    5,                // max buffer entries
                    10)               // data buffer listener
{
  //
  // get value(s) out of your argument table
  //
  //req_val        = getIntArg("req_val", argTab);
  imageFileNameSkel = "camera_";
  imageFileListName = "camera_0.sav";

  // set dataBuffer size
  setDataBufferMaxDataSize(sizeof(camera_data_msg));

  // set databuffer period time
  setDataBufferPeriodTime(100); // 100 ms (10 per sec)
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = Module::getArgs(argc, argv, argTab, "CameraFile");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new Camera_file

    p_inst = new CameraFile();
    if (!p_inst)
    {
        printf("Can't create new Camera_file -> EXIT\n");
        return -ENOMEM;
    }

    // init

    ret = p_inst->moduleInit();
    if (ret)
        goto exit_error;

    p_inst->run();

    return 0;

exit_error:

    delete (p_inst);
    return ret;
}

string CameraFile::intToString(int n)
{
    ostringstream outstre;
    outstre << n;
    return outstre.str();
}
