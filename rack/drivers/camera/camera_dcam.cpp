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

#include "camera_dcam.h"


#define INIT_BIT_DATA_MODULE 0

CameraDcam *p_inst;

argTable_t argTab[] = {

  { ARGOPT_OPT, "cameraGuid", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "cameraGuid", (int) 0 },

  { ARGOPT_OPT, "mode", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "mode", (int) CAMERA_MODE_YUV422 },

  { ARGOPT_OPT, "lossrate", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "lossrate", (int) 1 },

  { ARGOPT_OPT, "uValue", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "uValue", (int) 150 },

  { ARGOPT_OPT, "vValue", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "vValue", (int) 120 },

  { ARGOPT_OPT, "minHue", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "min not under illum. hue value", (int) 60 },

  { ARGOPT_OPT, "maxHue", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "max not over illum. hue value", (int) 250 },

  { ARGOPT_OPT, "gainMult", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "Multiplikator for gain correction", (int) 16 },

  { ARGOPT_OPT, "shutterMult", ARGOPT_REQVAL, ARGOPT_VAL_INT,
   "Multiplikator for shutter correction", (int) 64 },

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
int CameraDcam::autoBrightness(camera_data_msg *dataPackage)
{
	int i, count, minCount, maxCount, bytesPerPixel, gain, shutter;
	double unclearPixel;
    unsigned int ubrightness;
    unsigned int ushutter;
    unsigned int ugain;

	minCount = 0;
	maxCount = 0;

	count = dataPackage->data.width * dataPackage->data.height;

	bytesPerPixel = dataPackage->data.depth / 8;

    for(i = 1; i < count; i += bytesPerPixel)
    {
    	if((unsigned char)dataPackage->byteStream[i] <= minHue)
    	{
    		minCount++;
    	}
    	else if((unsigned char)dataPackage->byteStream[i] >= maxHue)
    	{
    		maxCount++;
    	}
    }

    if (dc1394_get_shutter(porthandle[dc1394CameraPortNo], camera_node, &ushutter) != DC1394_SUCCESS)
    {
        GDOS_WARNING("getting of shutter failed! ");
        return DC1394_FAILURE;
    }

    if (dc1394_get_gain(porthandle[dc1394CameraPortNo], camera_node, &ugain) != DC1394_SUCCESS)
    {
        GDOS_WARNING("getting of gain failed! ");
        return DC1394_FAILURE;
    }

    if (dc1394_get_brightness(porthandle[dc1394CameraPortNo], camera_node, &ubrightness) != DC1394_SUCCESS)
    {
        GDOS_WARNING("getting of brightness failed! ");
        return DC1394_FAILURE;
    }

    GDOS_DBG_DETAIL("count %i minCount %i maxCount %i actual shutter %i gain:%i brightness:%i \n", count, minCount, maxCount, ushutter, ugain, ubrightness);

    //if pixel are too dark, light them up!
    unclearPixel = ((double)(minCount - maxCount)) / count; //percent of over or under illum. pixel

    ubrightness = 124;

    if (unclearPixel > 0)
    {   //light up
        if (ushutter < 4095)
        {
            ushutter += (unsigned int) (unclearPixel * shutterMult);
          	if(ushutter > 4095)
          	    ushutter = 4095;
        } else
    	{   //increase gain
    		ugain += (unsigned int) (unclearPixel * gainMult);
    		if (ugain > 1023)
        		{ ugain = 1023;}
    	}
    } else
	{  //unclearPixel is negative, darken image
	   if (ugain > 192)
	   {
	       //to handle negative values
	       gain = (int) (ugain + unclearPixel * gainMult);
	       if (gain < 192)
	           gain = 192;
	       ugain = gain;
	   } else
	   {
	       shutter = (int) (ushutter + unclearPixel * shutterMult);
	       if (shutter < 1)
		       shutter = 1;
		   ushutter = shutter;
	   }
	}

    if (dc1394_set_shutter(porthandle[dc1394CameraPortNo], camera_node, ushutter) != DC1394_SUCCESS)
    {
        GDOS_WARNING("setting of shutter failed! is value:%i", ushutter  );
        return DC1394_FAILURE;
    }

    if (dc1394_set_gain(porthandle[dc1394CameraPortNo], camera_node, ugain) != DC1394_SUCCESS)
    {
        GDOS_WARNING("setting of gain failed! is value:%i", ugain  );
        return DC1394_FAILURE;
    }

    if (dc1394_set_brightness(porthandle[dc1394CameraPortNo], camera_node, ubrightness) != DC1394_SUCCESS)
    {
        GDOS_WARNING("setting of brightness failed! is value:%i", ubrightness  );
        return DC1394_FAILURE;
    }
    GDOS_DBG_DETAIL("set shutter:%i gain:%i brightness:%i \n", ushutter, ugain, ubrightness);
    return DC1394_SUCCESS;
}

/*
 * Initialising Firewirebus
 * Searching for 1394 Hostcontroller (not for nodes on the controller).
 * Testing if devices are accessable.
*/

int CameraDcam::getFirewirePortnum(void)
{
	/*
	 * handling of ports not yet correct.
	 * variable not used as intended.
	 */
	raw1394handle_t 		handle; //handle to raw 1394 bus
	struct raw1394_portinfo ports[MAX_PORTS];

    //try to open raw 1394 handle
    handle = raw1394_new_handle();
    if ( handle == NULL)
    {
        GDOS_ERROR("error: No raw1394 handle found.\n");
        return -DC1394_SUCCESS;
    }

    //get number of available ports
    firewireNumPorts = raw1394_get_port_info(handle, ports, firewireNumPorts);

    GDOS_DBG_INFO( "got portnum from 1394 bus.\n");

    raw1394_destroy_handle(handle);
    handle = NULL;
    return DC1394_SUCCESS;
}


/******************/
int CameraDcam::findCameraByGuid(void)
{
   /*-----------------------------------------------------------------------
    *  get the camera nodes and describe them as we find them
    * Number of available ports is known globally.
    *-----------------------------------------------------------------------*/
    //as the root node is not determined statically we may reset the bus in order to make a camera become
    //NOT a root node.
    //Uses generally the first camera found on bus.

    int portNum,resetNum, numNodes, foundCamerasOnBus;
    int resetBus = 1;
    nodeid_t * camera_nodes = NULL;

    for (resetNum=0; resetNum < MAX_RESETS && resetBus == 1; resetNum++)
    {
        resetBus = 0;
        foundCamerasOnBus = 0;
        for (portNum=0; portNum < firewireNumPorts; portNum++)
        {
            porthandle[portNum] = dc1394_create_handle(portNum);//handle for portNum
            if (porthandle[portNum]==NULL)
            {
                GDOS_WARNING("error: Unable to aquire handle for port %i.\n", portNum);
            }

            //each port can have multiple cameras attached.
            numNodes = 0;
            camera_nodes = dc1394_get_camera_nodes(porthandle[portNum], &numNodes, 0); //last parameter defines if output is given.

            //if any camera is found on this port...
            if (numNodes > 0)
            {
                GDOS_DBG_INFO("found %i cameras to port %i.\n",numNodes, portNum);
                //try to put them all at their position.
                int k;
                 for (k = 0; k < numNodes; k++)
                 {
                    //try to get camera guid
                    dc1394_camerainfo info;
                    if (dc1394_get_camera_info(porthandle[portNum], camera_nodes[k], &info) == DC1394_SUCCESS)
                    {
                        //test if camera node is root, if so reset bus...
                        if (camera_nodes[k] == raw1394_get_nodecount(porthandle[portNum])-1)
                        {
                            //reset and retry if root
                            GDOS_WARNING("error: camera found as root node - resetting.\n");
                            raw1394_reset_bus(porthandle[portNum]);
                            int node_pos = 0;
                            for (node_pos=0; node_pos < 10; node_pos++)
                                resetBus = 1;
                                foundCamerasOnBus = foundCamerasOnBus - k; //no -1 as the array stqarts with 0
                        }
                        GDOS_DBG_INFO("found camera with guid:%x \n", info.euid_64);
                        //put camera in the global variable
                        if ((cameraGuid == 0) || (cameraGuid == (int) info.euid_64))
                        {
                            GDOS_DBG_INFO("using camera node:%i with guid:%x \n",camera_nodes[k],  info.euid_64);
                            camera_node = camera_nodes[k];
                            dc1394CameraPortNo = portNum;
                        }
                        foundCamerasOnBus = foundCamerasOnBus + 1;
                    }//if camera_info
                }//for numCameras
            }//if numCamera > 0
            else {
        	   GDOS_ERROR("No cameras found! (%d nodes on the bus)\n"
		       "  - could be you need to try a different 1394 device (modify code to fix)\n",
		       numNodes );
            }
        }//for firewireNumPorts
    }//for MAXRESETS

    GDOS_DBG_INFO("bus init complete found %i cameras.\n",foundCamerasOnBus);
    if (resetNum == MAX_RESETS-1)
        return FW_ERROR;
    return DC1394_SUCCESS;
}

//
// Data handling
//

int CameraDcam::setupCaptureFormat2()
{
    int lokalmode = 0;
    /*correct mode parameter*/
    switch (mode)
    {
        case CAMERA_MODE_MONO8: //used in format 7 for external trigger. format 2 possible
            lokalmode = MODE_1280x960_MONO;
            bytesPerPixel = 1;
            break;
       case CAMERA_MODE_MONO12: //only possible in format 2 !!!
            lokalmode = MODE_1280x960_MONO16;
            bytesPerPixel = 2;
            break;
       case CAMERA_MODE_YUV422:
            lokalmode = MODE_1280x960_YUV422;
            bytesPerPixel = 2;
            break;
       default:
            GDOS_ERROR("Unable to set mode in format 2.\n");
            return DC1394_FAILURE;
    }

       /* Setup the capture mode */
   GDOS_DBG_INFO("Setting capture mode: porthandle:%i camera_node:%i channel:%i format:%i lokalmode:%i frameRate:%i device:",porthandle[dc1394CameraPortNo], camera_node, channel, format, lokalmode, frameRate);
   if (dc1394_dma_setup_capture( porthandle[dc1394CameraPortNo],
				     camera_node,
				     channel,
				     format,
				     lokalmode,//mode[id],
				     SPEED_400,
				     frameRate,
				     8,	// number of buffers
				     1,	// drop frames
				     device,
				     &dc1394Camera ) != DC1394_SUCCESS)
   {
      GDOS_WARNING("Unable to setup capture format 2.\n");
      return DC1394_FAILURE;
   }
   GDOS_DBG_INFO("DCAM CameraParameter: node%i add_capture_buffer:%i framesize%i\n", dc1394Camera.node, dc1394Camera.capture_buffer, dc1394Camera.dma_frame_size);
   GDOS_DBG_INFO("DCAM CameraParameter: dma_last_buffer:%i num_dma_buffers%i dma_buffer_size%i dma_fd:%i\n", dc1394Camera.dma_last_buffer, dc1394Camera.num_dma_buffers, dc1394Camera.dma_buffer_size, dc1394Camera.dma_fd);
   GDOS_DBG_INFO("DCAM Parameter setting successful.");
   return DC1394_SUCCESS;
}



int CameraDcam::setupCaptureFormat7()
{
    switch(mode)
    {
        case CAMERA_MODE_RAW8:
		    format7image.colorCodingId = COLOR_FORMAT7_RAW8;
		    bytesPerPixel = 1;
		    break;
	    case CAMERA_MODE_RAW12:
		    format7image.colorCodingId = COLOR_FORMAT7_RAW16;
		    bytesPerPixel = 2;
		    break;
        case CAMERA_MODE_MONO8:
		    format7image.colorCodingId = COLOR_FORMAT7_MONO8;
		    bytesPerPixel = 1;
		    break;
        default:
            GDOS_WARNING("Unable to setup capture format 7.\n");
            return DC1394_FAILURE;

    }
   /*-----------------------------------------------------------------------
   *  setup capture for format 7
   *-----------------------------------------------------------------------*/
   if (dc1394_dma_setup_format7_capture( porthandle[dc1394CameraPortNo],
				     camera_node,
				     channel,
				     format7image.mode,
				     SPEED_400,
				     format7image.bytesPerPacket,
				     format7image.leftImagePosition,
				     format7image.topImagePosition,
				     format7image.width,
				     format7image.height,
				     8,	// number of buffers
				     1,	// drop frames
				     device,
				     &dc1394Camera ) != DC1394_SUCCESS)
   {
      GDOS_ERROR("Unable to setup camera format 7.\n");
      return DC1394_FAILURE;
   }

  if( dc1394_set_format7_color_coding_id(
                     porthandle[dc1394CameraPortNo],
				     camera_node,
				     format7image.mode,
                     format7image.colorCodingId) != DC1394_SUCCESS)
  {
      GDOS_ERROR("Unable to setup camera color mode 7.\n");
      return DC1394_FAILURE;
  }
   GDOS_DBG_INFO("Parameter setting successful.");

  if( dc1394_query_format7_color_filter_id(
                     porthandle[dc1394CameraPortNo],
				     camera_node,
				     format7image.mode,
                     &format7image.colorFilterId) != DC1394_SUCCESS)
  {
      GDOS_ERROR("Unable to query camera color filter id 7.\n");
      return DC1394_FAILURE;
  }
  GDOS_DBG_INFO("Found color filter id:%i .\n",&format7image.colorFilterId);


  /* set trigger mode */
  /* not used with shot function!) */
  if( dc1394_set_trigger_mode(
                     porthandle[dc1394CameraPortNo],
				     camera_node,
                     TRIGGER_MODE_0)
      != DC1394_SUCCESS) //falling edge of extTrig shutter time set in register (no parameter)
  {
      GDOS_WARNING("Unable to setup format 7 trigger mode.\n");
  }

/*  if (dc1394_query_format7_total_bytes(handle, camera_nodes[0], MODE_FORMAT7_0, &total_bytes) != DC1394_SUCCESS)
  {
    printf("dc1394_query_format7_total_bytes error\n");
    return DC1394_FAILURE;
  }
  printf( "camera reports total bytes per frame = %lld bytes\n",
          total_bytes);
          */

  return DC1394_SUCCESS;
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

 int CameraDcam::moduleOn(void)
{
    unsigned int uvValue, uuValue;
    //need to get max size of image from this camera and also set dataBuffer according
    if (CAMERA_MAX_WIDTH < 1280 || CAMERA_MAX_HEIGHT < 960) //##tobe replaced by a dynamic call
    {
        GDOS_ERROR("Size parameter set too small in camera.h!! EXITING! \n");
        return ENOMEM;
    }

    RackTask::disableRealtimeMode();

    if ( getFirewirePortnum() != DC1394_SUCCESS)
        return FW_ERROR;
    if ( findCameraByGuid() != DC1394_SUCCESS)
    {
        GDOS_ERROR("Camera by given guid not found. \n");
        return FW_ERROR;
    }

    GDOS_DBG_INFO("camera_dcam initalising dcam part\n");

   /*-----------------------------------------------------------------------
    *  setup capture format 2 + 7
    *-----------------------------------------------------------------------*/
   if ( dc1394_get_iso_channel_and_speed( porthandle[dc1394CameraPortNo],
					  camera_node,
					  &channel,
					  &speed) !=DC1394_SUCCESS )
   {
      GDOS_WARNING("Unable to get the iso channel number\n" );
      return FW_ERROR;
   }

   switch(mode) {
    case CAMERA_MODE_YUV422:
    case CAMERA_MODE_MONO12:
        setupCaptureFormat2();
        break;
    case CAMERA_MODE_MONO8: //format 2 is possible but no external trigger
    case CAMERA_MODE_RAW8:
    case CAMERA_MODE_RAW12:
        setupCaptureFormat7();
        break;
    default:
        GDOS_ERROR("Set undefined ColorMode.\n" );
        return -EINVAL;
    }


   /*-----------------------------------------------------------------------
    *  have the camera start sending us data
    *-----------------------------------------------------------------------*/
    if ( dc1394_start_iso_transmission( porthandle[dc1394CameraPortNo], dc1394Camera.node )
	!=DC1394_SUCCESS )
   {
      GDOS_WARNING("Unable to start camera iso transmission\n" );
      return DC1394_FAILURE;
   }

   GDOS_DBG_INFO("Started iso transmission.\n" );

    if (dc1394_get_white_balance(porthandle[dc1394CameraPortNo], camera_node, &uuValue, &uvValue) != DC1394_SUCCESS)
    {
        GDOS_WARNING("getting of white balance failed! ");
        return DC1394_FAILURE;
    }

    uvValue = vValue;
    uuValue = uValue;

    if (dc1394_set_white_balance(porthandle[dc1394CameraPortNo], camera_node, uuValue, uvValue) != DC1394_SUCCESS)
    {
        GDOS_WARNING("setting of white balance failed u:%i v:%i!\n", uuValue, uvValue);
        return DC1394_FAILURE;
    }

    GDOS_DBG_INFO("set u:%i v:%i \n", uuValue, uvValue);


    return DataModule::moduleOn();  // have to be last command in moduleOn();
}


void CameraDcam::moduleOff(void)
{
   DataModule::moduleOff();        // have to be first command in moduleOff();


   if ( dc1394_stop_iso_transmission( porthandle[dc1394CameraPortNo],dc1394Camera.node ) != DC1394_SUCCESS )
   {
      GDOS_WARNING("Couldn't stop the iso transmission!!\n");
   }
   if ( dc1394_dma_unlisten( porthandle[dc1394CameraPortNo], &dc1394Camera ) != DC1394_SUCCESS)
   {
      GDOS_WARNING("Couldn't unlisten the dma!!\n");
   }
   if( dc1394_dma_release_camera( porthandle[dc1394CameraPortNo], &dc1394Camera ) != DC1394_SUCCESS)
   {
      GDOS_WARNING("Couldn't release the dma!!\n");
   }

}

int CameraDcam::moduleLoop(void)
{
    camera_data_msg*   p_data = NULL;
    ssize_t            datalength = 0;
    RACK_TIME          starttime;
    int h=0,w=0,p=0;
    int ret;

    starttime = get_rack_time();

    // get datapointer from databuffer
    p_data = (camera_data_msg *)getDataBufferWorkSpace();

    p_data->data.recordingTime   = get_rack_time();
    p_data->data.mode            =  mode;
    p_data->data.colorFilterId   =  format7image.colorFilterId;

    switch (mode){
    	case CAMERA_MODE_MONO8:
    	case CAMERA_MODE_RAW8:
    	    p_data->data.depth = 8;
    	    break;
    	case CAMERA_MODE_RAW12:
    	case CAMERA_MODE_YUV422:
    	    p_data->data.depth = 16;
    		break;
   		default:
            GDOS_ERROR("Unknown mode %i \n",mode);
            return -1;
    }

   /*-----------------------------------------------------------------------
    *  capture one frame and send to mbx
    *-----------------------------------------------------------------------*/
    GDOS_DBG_INFO("Capturing one image\n");
	if ((ret = dc1394_dma_single_capture( &dc1394Camera ))!=DC1394_SUCCESS)
   	{
      GDOS_WARNING("Unable to capture a frame. ret = %i \n", ret);
    }
    else
    {
        p_data->data.width  = dc1394Camera.frame_width  / lossRate;
        p_data->data.height = dc1394Camera.frame_height / lossRate;

        //shrink data if set to.
    	if (lossRate == 1)
    	{
    	    memcpy(p_data->byteStream, dc1394Camera.capture_buffer, dc1394Camera.dma_frame_size );
    	}
    	else
        {
            /*lossRate != 1 so we need to throw some pixel away. -> iterate over array.
             *The loss in YUV422 format must lose pairs of pixel! as UYVY pairs cannot be divided up!!
             * ->from 4 pixel in the original only one pixelPair! is transmitted.
             * eg. lossrate of 2 in yuv -> take 4 byte, leave 4 byte, take 4 byte.... leave a row...
             *
             *The loss in RAW format must also lose pairs of pixel! ohterwise only one color may result.
             */
            int pairfactor = 2; //depending on colorMode (Yuv=Raw=2, mono=1)
            int bytesPerPixelPair = 0;

            if (p_data->data.mode==CAMERA_MODE_MONO8 ||p_data->data.mode==CAMERA_MODE_RGB24 || p_data->data.mode==CAMERA_MODE_MONO12)
                pairfactor = 1;

            bytesPerPixelPair = bytesPerPixel * pairfactor;//to save multiplications

            //copy needed to change datatypes
            memcpy(dataBuffer, dc1394Camera.capture_buffer, dc1394Camera.dma_frame_size );

            for (h=0; h < p_data->data.height; h++)
            { //for every line in the smaller image
                for(w=0; w < p_data->data.width / pairfactor; w++)
                { //for every pixelPair in the smaller image
                    for (p=0; p<bytesPerPixelPair; p++)
                    { //for every byte per pixelPair
                        p_data->byteStream[   (h*bytesPerPixel*p_data->data.width) + (w*bytesPerPixelPair)              + p] =
                        dataBuffer[(h*bytesPerPixel*dc1394Camera.frame_width*lossRate) + (w*bytesPerPixelPair*lossRate) + p];
                    }
                }
            }
            GDOS_DBG_DETAIL("Data width %i pixelPairs to be send height %i pixel(!) bppp %i\n", w, h, p);
        } //end of lossRate calculation

        //doing auto shutter / gain / brightness control
        autoBrightness(p_data);

        GDOS_DBG_DETAIL("Data recordingtime %i width %i height %i depth %i mode %i\n", p_data->data.recordingTime, p_data->data.width, p_data->data.height, p_data->data.depth, p_data->data.mode);

        datalength = p_data->data.width * p_data->data.height * bytesPerPixel + sizeof(camera_data);
        putDataBufferWorkSpace(datalength);

        dc1394_dma_done_with_buffer( &dc1394Camera );//return the buffer handle to library.
    }

    //rt_sleep(timeCount1s/fps);//## zeitverwaltung zentral erledigt

    RackTask::sleep((1000000000llu/fps) - (get_rack_time() - starttime));

    return 0;
}



//
// Command handling
//
int CameraDcam::moduleCommand(MessageInfo *msgInfo)
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

			if(p_format->width  >= 0)
				 lossRate = CAMERA_MAX_WIDTH / p_format->width; //##use actual camera instead
			if(p_format->height >= 0)
			     {;}
			if(p_format->depth  >= 0)
     		     {;}
			if(p_format->mode >= 0)
	               mode = p_format->mode;
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

int CameraDcam::moduleInit(void)
{
    int ret;

    // call DataModule init function (first command in init)
    ret = DataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    if((dataBuffer = (char *)malloc( CAMERA_MAX_WIDTH * CAMERA_MAX_WIDTH * 2)) == NULL)
    {
        GDOS_ERROR("Can't allocate output data buffer\n");
        return ENOMEM;
    }

    return 0;

}

void CameraDcam::moduleCleanup(void)
{
    //raw1394_destroy_handle( porthandle[dc1394CameraPortNo] );

    // call DataModule cleanup function (last command in cleanup)
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        DataModule::moduleCleanup();
    }
}


CameraDcam::CameraDcam()
        : DataModule( MODULE_CLASS_ID,
                      5000000000llu,        // 5s cmdtask error sleep time
                      5000000000llu,        // 5s datatask error sleep time
                      100000000llu,         // 100ms datatask disable sleep time
                      16,                   // command mailbox slots
                      48,                   // command mailbox data size per slot
                      MBX_IN_KERNELSPACE | MBX_SLOT, // command mailbox flags //## it should be user space
                      25,                    // max buffer entries
                      10)                   // data buffer listener
{
    // get value(s) out of your argument table
    format 			= FORMAT_SVGA_NONCOMPRESSED_2;
	frameRate		= FRAMERATE_7_5; // only used in continuous grabbing mode (con. iso transmission, not implemented yet.)
	                                     // not used in singleshot opiton! Must be set in protocol.
	device 			=  "/dev/video1394"; // only possible with only one device!
	mode		    	= getIntArg("mode", argTab);
                    	//bytesPerPixel[i]	included in mode
	vValue  	        = getIntArg("vValue", argTab);
	uValue   	        = getIntArg("uValue", argTab);
	minHue  	        = getIntArg("minHue", argTab);
	maxHue   	        = getIntArg("maxHue", argTab);
	gainMult  	        = getIntArg("gainMult", argTab);
	shutterMult         = getIntArg("shutterMult", argTab);
	lossRate	        = getIntArg("lossrate", argTab);
                        //fps[i]	is handled as global parameter

	format7image.mode              = MODE_FORMAT7_0;//fixed!!
    format7image.bytesPerPacket    = USE_MAX_AVAIL;
	format7image.leftImagePosition =   54;//take middle part of image at max image size //values for OUR camera
	format7image.topImagePosition  =   39;//take middle part of image at max image size
	format7image.width             = 1280;//1388
	format7image.height            =  960;//1038
	format7image.colorFilterId     = COLORFILTER_RGGB;
	format7image.colorCodingId     = COLOR_FORMAT7_MONO8; //COLOR_FORMAT7_RAW8; //COLOR_FORMAT7_RAW16;

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(camera_data_msg));

    // set databuffer period time
    setDataBufferPeriodTime(100); // 100 ms (10 per sec)
    fps = 10;
}

int main(int argc, char *argv[])
{
    int ret;


    // get args
    ret = Module::getArgs(argc, argv, argTab, "CameraDcam");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new CameraDcam

    p_inst = new CameraDcam();
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
