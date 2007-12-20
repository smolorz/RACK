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

#include "camera_jpeg.h"
#include <main/image_tool.h>

#define INIT_BIT_DATA_MODULE  0
#define INIT_BIT_MBX_WORK     1
#define INIT_BIT_MBX_CAMERA   2
#define INIT_BIT_PROXY_CAMERA 3
#define INIT_BIT_RGBARRAY     4
#define INIT_BIT_JPEGBUFFER   5


CameraJpeg *p_inst;

argTable_t argTab[] = {

    { ARGOPT_OPT, "cameraInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "cameraInst", { 0 } },

    { ARGOPT_OPT, "quality", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "quality", { 50 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
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

 int CameraJpeg::moduleOn(void)
{
    int ret;

    GDOS_DBG_DETAIL("Initialising static compression structs \n");

    RackTask::disableRealtimeMode();

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    jpeg_stdmem_dest( &cinfo, jpegBuffer );

    GDOS_DBG_DETAIL("Turn on Camera(%d) \n", cameraInst);

    ret = camera->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on Camera(%d), code = %d \n", cameraInst, ret);
        return ret;
    }
    GDOS_DBG_DETAIL("Camera(%d) has been turned on \n", cameraInst);

    //get one image to get all needed parameter
    GDOS_DBG_DETAIL("Request data from Camera(%d)\n", cameraInst);
    ret = camera->getData(&cameraInputMsg.data, sizeof(cameraInputMsg), 0);
    if (ret)
    {
        GDOS_ERROR("Can't get single data from Camera(%d), "
                   "code = %d \n", cameraInst, ret);
        return ret;
    }

    cinfo.image_width      = cameraInputMsg.data.width;
    cinfo.image_height     = cameraInputMsg.data.height;
    cinfo.input_components = (cameraInputMsg.data.depth / 8) > 1 ? 3 : 1; //16 bit werte sind yuv und werden zu rgb24 aufskaliert
    cinfo.in_color_space   = (cameraInputMsg.data.depth / 8) > 1 ? JCS_RGB : JCS_GRAYSCALE;

    jpeg_set_defaults( &cinfo );
    jpeg_set_quality( &cinfo, quality, TRUE /* limit to baseline-JPEG values */ );


    GDOS_DBG_DETAIL("Request continuous data from Camera(%d)\n", cameraInst);
    ret = camera->getContData(dataBufferPeriodTime, &cameraMbx, &dataBufferPeriodTime);
    if (ret)
    {
        GDOS_ERROR("Can't get continuous data from Camera(%d), "
                   "code = %d \n", cameraInst, ret);
        return ret;
    }

    return RackDataModule::moduleOn();  // has to be last command in moduleOn();
}


void CameraJpeg::moduleOff(void)
{
   RackDataModule::moduleOff();        // has to be first command in moduleOff();

   jpeg_destroy_compress(&cinfo);

   camera->stopContData(&cameraMbx);

}

int CameraJpeg::moduleLoop(void)
{
    camera_data_msg*   p_data = NULL;
    camera_data_msg*   dataCameraInput = NULL;
    ssize_t            datalength = 0;
//    rack_time_t          starttime;
    message_info        msgInfo;
    int                ret;

    GDOS_DBG_INFO("starting loop\n");

    // get datapointer from rackdatabuffer
    p_data = (camera_data_msg *)getDataBufferWorkSpace();

    GDOS_DBG_INFO("getting data from camera %i\n", cameraInst);

    // get Camera data
    ret = cameraMbx.peek(&msgInfo);
//    ret = cameraMbx.peekTimed(1000000000llu, &msgInfo); // 1s
    if (ret)
    {
        GDOS_ERROR("Can't receive camera data on DATA_MBX, "
                   "code = %d \n", ret);
        return ret;
    }

    GDOS_DBG_INFO("parsing camera data \n");

    if ((msgInfo.type != MSG_DATA) ||
        (msgInfo.src  != camera->getDestAdr()))
    {
        GDOS_ERROR("Received unexpected message from %n to %n type %d on "
                   "data mailbox\n", msgInfo.src, msgInfo.dest, msgInfo.type);

        cameraMbx.peekEnd();
        return -EINVAL;
    }

    dataCameraInput = (camera_data_msg *) CameraData::parse(&msgInfo);

    GDOS_DBG_INFO("converting camera data \n");

    //convert camera_bytestream (may be yuv/raw mode) to rgb mode
    switch(dataCameraInput->data.mode) {
        case CAMERA_MODE_YUV422:
            ImageTool::convertCharUYVY2BGR(rgbByteArray,
                                           dataCameraInput->byteStream,
                                           dataCameraInput->data.width,
                                           dataCameraInput->data.height);
            break;
        case CAMERA_MODE_MONO8:
        case CAMERA_MODE_RGB24:
            ImageTool::convertCharBGR2RGB(rgbByteArray,
                                           dataCameraInput->byteStream,
                                           dataCameraInput->data.width,
                                           dataCameraInput->data.height);
//memcpy(rgbByteArray, dataCameraInput->byteStream, sizeof(dataCameraInput->byteStream));
            break;
        default:
            GDOS_ERROR( "Unkown image mode %i for camera_jpeg\n", dataCameraInput->data.mode);
    }
    GDOS_DBG_INFO("starting compress \n");

    jpeg_start_compress(&cinfo, TRUE);//true for outputting all tables

    GDOS_DBG_INFO("compressing\n");

    compressByteStream2JpegStream(rgbByteArray, &cinfo);

    GDOS_DBG_INFO("finishing compress\n");

    jpeg_finish_compress(&cinfo);

    GDOS_DBG_INFO("sending data with jpeg size %i\n", ((jpeg_data_dst_ptr)cinfo.dest)->outstreamOffset);

    p_data->data.recordingTime = dataCameraInput->data.recordingTime;
    p_data->data.width         = dataCameraInput->data.width;
    p_data->data.height        = dataCameraInput->data.height;
    p_data->data.depth         = cinfo.input_components * 8;
    p_data->data.mode          = CAMERA_MODE_JPEG;
    p_data->data.colorFilterId = ((jpeg_data_dst_ptr)cinfo.dest)->outstreamOffset; //missused as array length here

    memcpy(&(p_data->byteStream), jpegBuffer, ((jpeg_data_dst_ptr)cinfo.dest)->outstreamOffset);

    datalength = sizeof(camera_data) +
                 ((jpeg_data_dst_ptr)cinfo.dest)->outstreamOffset; //special in jdatadstmem.c!!

    cameraMbx.peekEnd();
    putDataBufferWorkSpace(datalength);
    return 0;
}



//
// Command handling
//
int CameraJpeg::moduleCommand(message_info *msgInfo)
{

    switch (msgInfo->type)
    {
    case MSG_CAMERA_GET_PARAMETER:
        //weiterleitung an ursprungskamera
        camera_param_data param;

        camera->getParam(&param, sizeof(camera_param_data));

        cmdMbx.sendDataMsgReply(MSG_CAMERA_PARAMETER, msgInfo, 1, &param,
                                sizeof(camera_param_data));
        break;

    default:
        // not for me -> ask RackDataModule
        return RackDataModule::moduleCommand(msgInfo);
    }
    return 0;
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

int CameraJpeg::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    // work mailbox
    ret = createMbx(&workMbx, 1, sizeof(camera_data_msg), MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // camera-data mailbox
    ret = createMbx(&cameraMbx, 1, sizeof(camera_data_msg),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_CAMERA);

    // create Camera Proxy
    camera = new CameraProxy(&workMbx, 0, cameraInst);
    if (!camera)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_CAMERA);

    if((rgbByteArray = (uint8_t *) malloc(CAMERA_MAX_WIDTH*CAMERA_MAX_HEIGHT*CAMERA_MAX_DEPTH/8)) == NULL)
    {
        GDOS_ERROR("Can't allocate rgbByteArray buffer\n");
        return -ENOMEM;
    }
    initBits.setBit(INIT_BIT_RGBARRAY);

    if((jpegBuffer = (char *) malloc(CAMERA_MAX_WIDTH*CAMERA_MAX_HEIGHT*CAMERA_MAX_DEPTH/8)) == NULL)
    {
        GDOS_ERROR("Can't allocate jpegBuffer\n");
        return -ENOMEM;
    }
    initBits.setBit(INIT_BIT_JPEGBUFFER);

    return 0;

init_error:
    // !!! call local cleanup function !!!
    CameraJpeg::moduleCleanup();
    return ret;
}

void CameraJpeg::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    if (initBits.testAndClearBit(INIT_BIT_JPEGBUFFER))
    {
        free(jpegBuffer);
    }

    if (initBits.testAndClearBit(INIT_BIT_RGBARRAY))
    {
        free(rgbByteArray);
    }

   if (initBits.testAndClearBit(INIT_BIT_PROXY_CAMERA))
    {
        delete camera;
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_CAMERA))
    {
        destroyMbx(&cameraMbx);
    }

    // delete mailboxes
    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }
}


CameraJpeg::CameraJpeg()
        : RackDataModule( MODULE_CLASS_ID,
                      5000000000llu,        // 5s datatask error sleep time
                      16,                   // command mailbox slots
                      48,                   // command mailbox data size per slot
                      MBX_IN_KERNELSPACE | MBX_SLOT, // command mailbox flags //## it should be user space
                      10,                    // max buffer entries
                      10)                   // data buffer listener
{
    // get value(s) out of your argument table
    cameraInst    = getIntArg("cameraInst", argTab);
    quality       = getIntArg("quality", argTab);

    dataBufferMaxDataSize = sizeof(camera_data_msg);
}

int main(int argc, char *argv[])
{
    int ret;


    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "CameraJpeg");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new CameraJpeg

    p_inst = new CameraJpeg();
    if (!p_inst)
    {
        printf("Can't create new CameraJpeg -> EXIT\n");
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

void CameraJpeg::compressByteStream2JpegStream(uint8_t* byteStream, j_compress_ptr cinfo)
{
    JSAMPLE* image_buffer = (JSAMPLE*) byteStream;
    JSAMPROW row_pointer[1];   /* pointer to a single row */
    int row_stride;            /* physical row width in buffer */

    row_stride = cinfo->image_width * cinfo->input_components;    /* JSAMPLEs per row in image_buffer */

    while (cinfo->next_scanline < cinfo->image_height) {
        row_pointer[0] = &(image_buffer[cinfo->next_scanline * row_stride]);
        jpeg_write_scanlines(cinfo, row_pointer, 1);
    }
        GDOS_DBG_INFO("end while\n");
}
