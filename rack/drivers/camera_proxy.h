/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Marko Reimer     <reimer@l3s.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */

#ifndef __CAMERA_PROXY_H__
#define __CAMERA_PROXY_H__

#include <main/rack_proxy.h>

//######################################################################
//# Camera Message Types
//######################################################################

#define MSG_CAMERA_GET_PARAMETER         (RACK_PROXY_MSG_POS_OFFSET + 1)
#define MSG_CAMERA_SET_FORMAT            (RACK_PROXY_MSG_POS_OFFSET + 2)
#define MSG_CAMERA_PARAMETER             (RACK_PROXY_MSG_NEG_OFFSET - 1)
#define MSG_CAMERA_FORMAT                (RACK_PROXY_MSG_NEG_OFFSET - 2)



#define CAMERA_MAX_WIDTH  1280 //1280
#define CAMERA_MAX_HEIGHT  960 //960
#define CAMERA_MAX_DEPTH    24 //max bit per pixel(YUV Mode) //actual max of tims = 16 * 1280 * 960
#define CAMERA_MAX_BYTES  (CAMERA_MAX_WIDTH * CAMERA_MAX_HEIGHT * CAMERA_MAX_DEPTH / 8)

#define CAMERA_MODE_MONO8  01
#define CAMERA_MODE_MONO12 02
#define CAMERA_MODE_MONO16 03
#define CAMERA_MODE_RGB24  11
#define CAMERA_MODE_RGB565 12
#define CAMERA_MODE_YUV422 21
#define CAMERA_MODE_RAW8   31
#define CAMERA_MODE_RAW12  32
#define CAMERA_MODE_RAW16  33
#define CAMERA_MODE_JPEG   41

#define COLORFILTER_RGGB  512
#define COLORFILTER_GBRG  513
#define COLORFILTER_GRBG  514
#define COLORFILTER_BGGR  515

//######################################################################
//# Camera Data (static size  - MESSAGE)
//######################################################################

typedef struct {
    RACK_TIME recordingTime;  // have to be first element
    uint16_t   width;
    uint16_t   height;
    uint16_t   depth;
    uint16_t   mode;
    uint32_t   colorFilterId;
    uint8_t    byteStream[0];
} __attribute__((packed)) camera_data;

class CameraData
{
    public:
        static void le_to_cpu(camera_data *data)
        {
        	int i;
        	int bytes;
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->width         = __le16_to_cpu(data->width);
            data->height        = __le16_to_cpu(data->height);
            data->depth         = __le16_to_cpu(data->depth);
            data->mode          = __le16_to_cpu(data->mode);
            data->colorFilterId = __le32_to_cpu(data->colorFilterId);

            bytes = data->width * data->height * data->depth / 8;

            for (i = 0; i < bytes; i++)
            {
                data->byteStream[i] = __le32_to_cpu(data->byteStream[i]);
            }
        }

        static void be_to_cpu(camera_data *data)
        {
        	int i;
        	int bytes;

            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->width         = __be16_to_cpu(data->width);
            data->height        = __be16_to_cpu(data->height);
            data->depth         = __be16_to_cpu(data->depth);
            data->mode          = __be16_to_cpu(data->mode);
            data->colorFilterId = __be32_to_cpu(data->colorFilterId);

            bytes = data->width * data->height * data->depth / 8;

            for (i = 0; i < bytes; i++)
            {
                data->byteStream[i] = __be32_to_cpu(data->byteStream[i]);
            }
        }

        static camera_data* parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            camera_data *p_data = (camera_data *)msgInfo->p_data;

            if (msgInfo->flags & MSGINFO_DATA_LE) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->usedMbx->setDataByteorder(msgInfo);
            return p_data;
        }

};

typedef struct {
    camera_data 	data;
    uint8_t		    byteStream[CAMERA_MAX_BYTES];
} __attribute__((packed)) camera_data_msg;


//######################################################################
//# Camera Parameter Data (static size  - MESSAGE)
//######################################################################

typedef struct
{
  int32_t   calibration_width;
  int32_t   calibration_height;
  float32_t    f;
  float32_t    fx;
  float32_t    fy;
  float32_t    sx;
  float32_t    sy;
  float32_t    dx;
  float32_t    dy;
  float32_t    k1;
  float32_t    k2;
  float32_t    p1;
  float32_t    p2;
  float32_t    e0;
  float32_t    n0;

} __attribute__((packed)) camera_param_data;

class CameraParamData
{
    public:
        static void le_to_cpu(camera_param_data *data)
        {
            data->calibration_width   = __le32_to_cpu(data->calibration_width);
            data->calibration_height  = __le32_to_cpu(data->calibration_height);
            data->f     = __le32_float_to_cpu(data->f);
            data->fx    = __le32_float_to_cpu(data->fx);
            data->fy    = __le32_float_to_cpu(data->fy);
            data->sx    = __le32_float_to_cpu(data->sx);
            data->sy    = __le32_float_to_cpu(data->sy);
            data->dx    = __le32_float_to_cpu(data->dx);
            data->dy    = __le32_float_to_cpu(data->dy);
            data->k1    = __le32_float_to_cpu(data->k1);
            data->k2    = __le32_float_to_cpu(data->k2);
            data->p1    = __le32_float_to_cpu(data->p1);
            data->p2    = __le32_float_to_cpu(data->p2);
            data->e0    = __le32_float_to_cpu(data->e0);
            data->n0    = __le32_float_to_cpu(data->n0);
        }

        static void be_to_cpu(camera_param_data *data)
        {
            data->calibration_width   = __be32_to_cpu(data->calibration_width);
            data->calibration_height  = __be32_to_cpu(data->calibration_height);
            data->f     = __be32_float_to_cpu(data->f);
            data->fx    = __be32_float_to_cpu(data->fx);
            data->fy    = __be32_float_to_cpu(data->fy);
            data->sx    = __be32_float_to_cpu(data->sx);
            data->sy    = __be32_float_to_cpu(data->sy);
            data->dx    = __be32_float_to_cpu(data->dx);
            data->dy    = __be32_float_to_cpu(data->dy);
            data->k1    = __be32_float_to_cpu(data->k1);
            data->k2    = __be32_float_to_cpu(data->k2);
            data->p1    = __be32_float_to_cpu(data->p1);
            data->p2    = __be32_float_to_cpu(data->p2);
            data->e0    = __be32_float_to_cpu(data->e0);
            data->n0    = __be32_float_to_cpu(data->n0);
        }

        static camera_param_data *parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            camera_param_data *p_data = (camera_param_data *)msgInfo->p_data;

            if (msgInfo->flags & MSGINFO_DATA_LE) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->usedMbx->setDataByteorder(msgInfo);
            return p_data;
        }

};


//######################################################################
//# Camera Format Data (static size  - MESSAGE)
//######################################################################

typedef struct
{
  int32_t   width;
  int32_t   height;
  int32_t   depth;
  int32_t   mode;

} __attribute__((packed)) camera_format_data;

class CameraFormatData
{
    public:
        static void le_to_cpu(camera_format_data *data)
        {
            data->width             = __le32_to_cpu(data->width);
            data->height            = __le32_to_cpu(data->height);
            data->depth             = __le32_to_cpu(data->depth);
            data->mode              = __le32_to_cpu(data->mode);
        }

        static void be_to_cpu(camera_format_data *data)
        {
            data->width             = __be32_to_cpu(data->width);
            data->height            = __be32_to_cpu(data->height);
            data->depth             = __be32_to_cpu(data->depth);
            data->mode              = __be32_to_cpu(data->mode);
        }

        static camera_format_data *parse(MessageInfo *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            camera_format_data *p_data = (camera_format_data *)msgInfo->p_data;

            if (msgInfo->flags & MSGINFO_DATA_LE) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->usedMbx->setDataByteorder(msgInfo);
            return p_data;
        }

};

//######################################################################
//# Camera Proxy Functions
//######################################################################

class CameraProxy : public RackDataProxy {

  public:

//
// constructor / destructor
// WARNING -> look at module class id in constuctor
//

    CameraProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
              : RackDataProxy(workMbx, sys_id, CAMERA, instance)
    { };

    ~CameraProxy()
    { };


//
// overwriting getData proxy function
// (includes parsing and type conversion)
//

    int getData(camera_data *recv_data, ssize_t recv_datalen, RACK_TIME timeStamp,
                MessageInfo *msgInfo)
    {
        return getData(recv_data, recv_datalen, timeStamp, dataTimeout, msgInfo);
    }

    int getData(camera_data *recv_data, ssize_t recv_datalen, RACK_TIME timeStamp,
                uint64_t reply_timeout_ns, MessageInfo *msgInfo);


// getParam


    int getParam(camera_param_data *recv_data, ssize_t recv_datalen,
                 MessageInfo *msgInfo)
    {
        return getParam(recv_data, recv_datalen, dataTimeout, msgInfo);
    }

    int getParam(camera_param_data *recv_data, ssize_t recv_datalen,
                 uint64_t reply_timeout_ns, MessageInfo *msgInfo);

};

#endif // __CAMERA_PROXY_H__
