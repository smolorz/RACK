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

#include "camera_v4l.h"
#include <main/image_tool.h>

#define INIT_BIT_DATA_MODULE 0

CameraV4L *p_inst;

argTable_t argTab[] = {

  { ARGOPT_OPT, "width", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "width", (int) 320 },

  { ARGOPT_OPT, "height", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "height", (int) 240 },

  { ARGOPT_OPT, "depth", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "depth", (int) 24 },

  { ARGOPT_OPT, "mode", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "mode", (int) CAMERA_MODE_RGB24 },

  { ARGOPT_OPT, "videoId", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "videoId", (int) 0 },

  { ARGOPT_OPT, "minHue", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "min not under illum. hue value", (int) 15 },

  { ARGOPT_OPT, "maxHue", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "max not over illum. hue value", (int) 250 },

  { ARGOPT_OPT, "gainMult", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "Multiplikator for gain correction", (int) 1000 },

  { ARGOPT_OPT, "autoBrightnessSize", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "Button part of autobrightness part of image", (int) 1 },

  { 0,"",0,0,""}                                  // last entry
};

camera_param_data param = {
		calibration_width  : CALIBRATION_WIDTH,
		calibration_height : CALIBRATION_HEIGHT,
		f   : F,
		fx  : F_x,
		fy  : F_y,
		sx  : S_x,
		sy  : S_y,
		dx  : D_x,
		dy  : D_y,
		k1  : K1,
		k2  : K2,
		p1  : P1,
		p2  : P2,
		e0  : E0,
		n0  : N0,
};


//*** Method for automatic adaption of the lumination parameter****//
int CameraV4L::autoBrightness(camera_data_msg *dataPackage)
{
    int i, count, minCount, maxCount, brightness, start, actualHue;

    count    = camera.grab_size; 
    minCount = 0;
    maxCount = 0;
       
    start = (int) (count * (((double ) 1) - ((double) 1) / autoBrightnessSize)); 
    start -= start % 3; 

    for(i = start; i < count; i += 4)
    {
//        actualHue = (66 * dataPackage->byteStream[i] + 129 * dataPackage->byteStream[i + 1] + 25 * dataPackage->byteStream[i + 2] + 128 ) >> 8;
         actualHue = (dataPackage->byteStream[i] + dataPackage->byteStream[i + 1] + dataPackage->byteStream[i + 2])  / 3; 
        

    	if(actualHue <= minHue)
    	{
    		minCount++;
    	}
    	else if(actualHue >= maxHue)
    	{
    		maxCount++;
    	}
    }

    brightness = camera.vid_picture.brightness;
	if (abs(maxCount-minCount) > 5000)
   		 brightness -= gainMult * (maxCount - minCount)* 4 * autoBrightnessSize / count;//as only every 4.th pixel is used

	if(brightness > 65536)
	{
		brightness = 65536;
	}
	else if(brightness < 0)
	{
		brightness = 0;
	}

    camera.vid_picture.brightness = brightness;
    ioctl(camera.dev,VIDIOCSPICT,&camera.vid_picture);

    GDOS_DBG_DETAIL("count %i minCount %i maxCount %i brightness %i\n", count, minCount, maxCount, brightness);
    return 0;
}


/*******************************************************************************
 *   !!! REALTIME CONTEXT !!!
 *
 *   moduleOn,
 * 	 moduleOff,
 * 	 moduleLoop,
 *   moduleCommand,
 *
 *   own realtime user functions
 ******************************************************************************/

 int CameraV4L::moduleOn(void)
{
    GDOS_DBG_INFO("camera_v4l on\n");

    RackTask::disableRealtimeMode();

    camera.dev=open(camera.devname,O_RDWR);

	bzero(&camera.vid_capability, sizeof(camera.vid_capability));
    ioctl(camera.dev,VIDIOCGCAP,&camera.vid_capability);//get possibilities of the device

	if(!(camera.vid_capability.type & VID_TYPE_CAPTURE))
	{
		GDOS_ERROR("No video capturer at /dev/video\n");
		return -EINVAL;
	}

    //is an image of maximal size possible?
    if (camera.vid_capability.maxwidth >= width) {
        camera.vid_window.width = width;
        camera.vid_window.x     = 0;//((camera[id].vid_capability.maxwidth - width[id]) / 2);//take the middle of possible area.

    } else {
        camera.vid_window.width = camera.vid_capability.maxwidth;
        camera.vid_window.x = 0;
    }

    if (camera.vid_capability.maxheight >= height) {
        camera.vid_window.height = height;
        camera.vid_window.y      = 0;//((camera[id].vid_capability.maxwidth - height[id]) / 2);//take the middle of possible area.
    } else {
        camera.vid_window.height=camera.vid_capability.maxheight;
        camera.vid_window.y = 0;
    }

    ioctl(camera.dev,VIDIOCSWIN,&camera.vid_window);//set the chosen windowsize
    ioctl(camera.dev,VIDIOCGWIN,&camera.vid_window);//get the chosen windowsize

    //get possible image parameter
	bzero(&camera.vid_picture, sizeof(camera.vid_picture));
    ioctl(camera.dev,VIDIOCGPICT,&camera.vid_picture);

    //try to set parameter according to preferences.
    switch(mode)
	{
	case CAMERA_MODE_MONO8:
					        camera.vid_picture.palette = VIDEO_PALETTE_GREY;
					        camera.vid_picture.depth = 8;
				            break;
    case CAMERA_MODE_RGB565:
                            camera.vid_picture.palette = VIDEO_PALETTE_RGB565;
                            camera.vid_picture.depth = 16;
                            break;
    case CAMERA_MODE_RGB24:
    default:
                            camera.vid_picture.palette = VIDEO_PALETTE_RGB24;
                            camera.vid_picture.depth = 24;
                            break;
    }
    GDOS_DBG_INFO("Camera on with parameter: width %i height %i depth %i mode %i\n", camera.vid_window.width, camera.vid_window.height, camera.vid_picture.depth, mode);

    //if needed set color parameter to half.
    camera.vid_picture.brightness = 30000;
//    camera->vid_picture.hue        = 32000;
//    camera->vid_picture.colour     = 32000;
//    camera->vid_picture.contrast   = 32000;
//    camera->vid_picture.whiteness  = 32000;
    ioctl(camera.dev,VIDIOCSPICT,&camera.vid_picture);
    ioctl(camera.dev,VIDIOCGPICT,&camera.vid_picture);

    switch(camera.vid_picture.palette)
    {
	case VIDEO_PALETTE_GREY:
					        camera.mode = CAMERA_MODE_MONO8;
				            break;
    case VIDEO_PALETTE_RGB24:
                            camera.mode = CAMERA_MODE_RGB24;
                            break;
    case VIDEO_PALETTE_RGB565:
                            camera.mode = CAMERA_MODE_RGB565;
                            break;
    default:
	    GDOS_DBG_INFO("Video mode not supported %i\n", camera.vid_picture.palette);
	    return -EINVAL;
    }

    camera.grab_size = camera.vid_window.width * camera.vid_window.height * camera.vid_picture.depth / 8;

    GDOS_DBG_INFO("Camera on with parameter: width %i height %i depth %i mode %i\n", camera.vid_window.width, camera.vid_window.height, camera.vid_picture.depth, camera.mode);
//    GDOS_DBG_INFO("brightness %i hue %i color %i contrast %i whiteness %i\n", camera.vid_picture.brightness, camera.vid_picture.hue, camera.vid_picture.colour, camera.vid_picture.contrast, camera.vid_picture.whiteness);

	bzero(&camera.m_buf, sizeof(camera.m_buf));
    ioctl(camera.dev,VIDIOCGMBUF,&camera.m_buf);//get max buffers

    if(camera.m_buf.frames >= 2)
    {
		if((camera.grab_data = (unsigned char *)mmap(0,camera.m_buf.size, PROT_READ|PROT_WRITE, MAP_SHARED,camera.dev,0)) == NULL)
		{
	    	GDOS_ERROR("Can't init memory map function\n");
	    	return -ENOMEM;
		}

	    camera.grab_buf.width  = camera.vid_window.width;
	    camera.grab_buf.height = camera.vid_window.height;
	   	camera.grab_buf.format = camera.vid_picture.palette;

		for(camera.currentFrame = 0; camera.currentFrame < camera.m_buf.frames; camera.currentFrame++)
		{
		    camera.grab_buf.frame  = camera.currentFrame;

		    if (-1 == ioctl(camera.dev,VIDIOCMCAPTURE,&camera.grab_buf)) {
		        GDOS_ERROR("Error grabbing picture (buffer)\n");
		        return -EINVAL;
		    }
	    }

		camera.currentFrame = 0;
	}
    else
    {
    	GDOS_WARNING("Not enough memory map buffers %i < 2. Using read\n", camera.m_buf.frames);
    }

    return DataModule::moduleOn();  // have to be last command in moduleOn();
}


void CameraV4L::moduleOff(void)
{
   DataModule::moduleOff();        // have to be first command in moduleOff();

    GDOS_DBG_INFO("camera_v4l off\n");

    close(camera.dev);

    if(camera.m_buf.frames >= 2)
    {
	    munmap(camera.grab_data, camera.m_buf.size);
	}

}

int CameraV4L::moduleLoop(void)
{
    camera_data_msg*   p_data = NULL;
    ssize_t            datalength = 0;
//    RACK_TIME          starttime;

    GDOS_DBG_DETAIL("camera_v4l looping\n");

    // get datapointer from databuffer
    p_data = (camera_data_msg *)getDataBufferWorkSpace();

    p_data->data.recordingTime  = get_rack_time();
    p_data->data.width          = camera.vid_window.width;
    p_data->data.height         = camera.vid_window.height;
    p_data->data.depth          = camera.vid_picture.depth;
    p_data->data.mode           = camera.mode;
    p_data->data.colorFilterId  = 0; //not used here


    if(camera.m_buf.frames >= 2)
    {
	    camera.grab_buf.frame = camera.currentFrame;
	    if (-1 == ioctl(camera.dev,VIDIOCSYNC,&camera.grab_buf)) {
	        GDOS_ERROR("Error synconizing buffer.\n");
	        return -ENOMEM;
	    }

	    memcpy(p_data->byteStream, camera.grab_data + camera.m_buf.offsets[camera.currentFrame], camera.grab_size);
	}
	else
	{
	    if(read(camera.dev, p_data->byteStream, camera.grab_size) <= 0)
	    {
	        GDOS_ERROR("Can't read image data.\n");
	        return -EINVAL;
	    }

	}

    GDOS_DBG_DETAIL("Data recordingtime %i width %i height %i depth %i mode %i\n", p_data->data.recordingTime, p_data->data.width, p_data->data.height, p_data->data.depth, p_data->data.mode);
    
    ImageTool::convertCharBGR2RGB((uint8_t *)&(p_data->byteStream), (uint8_t *) &(p_data->byteStream),(int) p_data->data.width, (int) p_data->data.height);

    datalength = sizeof(camera_data) + camera.grab_size;
    putDataBufferWorkSpace(datalength);

    RackTask::sleep(500000000llu);

	autoBrightness(p_data);

    if(camera.m_buf.frames >= 2)
    {
	    camera.grab_buf.frame = camera.currentFrame;
	    if (-1 == ioctl(camera.dev,VIDIOCMCAPTURE,&camera.grab_buf)) {
	        GDOS_ERROR("Error grabbing picture (buffer)\n");
	        return -EINVAL;
	    }

	    camera.currentFrame = (camera.currentFrame + 1) % camera.m_buf.frames;
	}

//    rt_sleep(timeCount1s/fps);//## zeitverwaltung zentral erledigt
    return 0;
}



//
// Command handling
//
int CameraV4L::moduleCommand(MessageInfo *msgInfo)
{
    camera_format_data      *p_format;

    switch (msgInfo->type)
    {
    case MSG_CAMERA_GET_PARAMETER:
        cmdMbx.sendDataMsgReply(MSG_CAMERA_PARAMETER, msgInfo, 1, &param,
                                sizeof(camera_param_data));
        break;

	case MSG_CAMERA_SET_FORMAT:
        if(status == MODULE_STATE_DISABLED)
        {
            p_format = CameraFormatData::parse(msgInfo);

            GDOS_DBG_INFO( "set format width=%i height=%i depth=%i mode=%i\n", p_format->width, p_format->height, p_format->depth, p_format->mode);

			if(p_format->width >= 0)
		        width    = p_format->width;
			if(p_format->height >= 0)
		        height   = p_format->height;
			if(p_format->depth >= 0)
		        depth    = p_format->depth;
			if(p_format->mode >= 0)
		        mode     = p_format->mode;

            cmdMbx.sendMsgReply(MSG_OK, msgInfo);
        }
        else
        {
            GDOS_WARNING("Camera needs to be turned off to set format\n");

            cmdMbx.sendMsgReply(MSG_ERROR, msgInfo);
            break;
        }
		break;

	default:
        // not for me -> ask DataModule
        return DataModule::moduleCommand(msgInfo);
    }
    return 0;
}


/*******************************************************************************
 *   !!! NON REALTIME CONTEXT !!!
 *
 *   moduleInit,
 * 	 moduleCleanup,
 * 	 Constructor,
 *   Destructor,
 *   main,
 *
 *   own non realtime user functions
 ******************************************************************************/

int CameraV4L::moduleInit(void)
{
    int ret;

    // call DataModule init function (first command in init)
    ret = DataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    sprintf(camera.devname, "/dev/video%i", videoId);
    return 0;

}

void CameraV4L::moduleCleanup(void)
{

    // call DataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        DataModule::moduleCleanup();
    }
}


CameraV4L::CameraV4L()
        : DataModule( MODULE_CLASS_ID,
                      5000000000llu,        // 5s cmdtask error sleep time
                      5000000000llu,        // 5s datatask error sleep time
                      100000000llu,         // 100ms datatask disable sleep time
                      16,                   // command mailbox slots
                      48,                   // command mailbox data size per slot
                      MBX_IN_KERNELSPACE | MBX_SLOT, // command mailbox flags //## it should be user space
                      20,                    // max buffer entries
                      10)                   // data buffer listener
{
	width  	        = getIntArg("width", argTab);
	height   	    = getIntArg("height", argTab);
	depth	        = getIntArg("depth", argTab);
	mode	        = getIntArg("mode", argTab);
	videoId	        = getIntArg("videoId", argTab);
	minHue  	    = getIntArg("minHue", argTab);
	maxHue   	    = getIntArg("maxHue", argTab);
	gainMult  	    = getIntArg("gainMult", argTab);
	autoBrightnessSize	= getIntArg("autoBrightnessSize", argTab);

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(camera_data_msg));

    // set databuffer period time
    setDataBufferPeriodTime(500); // hardcoded in loop!!! 
    //500000000llu
}

int main(int argc, char *argv[])
{
    int ret;


    // get args
    ret = Module::getArgs(argc, argv, argTab, "CameraV4L");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new CameraDcam

    p_inst = new CameraV4L();
    if (!p_inst)
    {
        printf("Can't create new CameraDcam -> EXIT\n");
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
