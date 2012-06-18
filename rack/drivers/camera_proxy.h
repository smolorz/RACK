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
 *      Marko Reimer     <reimer@rts.uni-hannover.de>
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



#define CAMERA_MAX_WIDTH  1280              /**< maximum width of camera image */
#define CAMERA_MAX_HEIGHT 1024              /**< maximum height of camera image */
#define CAMERA_MAX_DEPTH    24              /**< maximum depth per pixel of camera image */
#define CAMERA_MAX_BYTES  (CAMERA_MAX_WIDTH * CAMERA_MAX_HEIGHT * CAMERA_MAX_DEPTH / 8)
                                            /**< maximum bytes of camera image */

//CAMERA_MODE 01-54 are modi for the mono images
#define CAMERA_MODE_MONO8  01               /**< mono image with 8bit depth */
#define CAMERA_MODE_MONO12 02               /**< mono image with 12bit depth */
#define CAMERA_MODE_MONO16 03               /**< mono image with 16bit depth */
#define CAMERA_MODE_MONO24 04               /**< mono image with 24bit depth */
//CAMERA_MODE 11-12 are modi for rgb images
#define CAMERA_MODE_RGB24  11               /**< color image with 24bit depth */
#define CAMERA_MODE_RGB565 12               /**< color image in 565 format */
//CAMERA_MODE 21 is a modi for yuv images
#define CAMERA_MODE_YUV422 21               /**< color image in yuv422 format */
//CAMERA_MODE 31-33 are modi for raw (bayer) images
#define CAMERA_MODE_RAW8   31               /**< color image in raw8 (bayer) format */
#define CAMERA_MODE_RAW12  32               /**< color image in raw12 (bayer) format */
#define CAMERA_MODE_RAW16  33               /**< color image in raw16 (bayer) format */
//CAMERA_MODE 41 is modi for jpeg compressed images
#define CAMERA_MODE_JPEG   41               /**< jpeg compressed image */
//CAMERA_MODE 51-59 are modi for the 3d range images
#define CAMERA_MODE_RANGE          51       /**< 3d range image */
#define CAMERA_MODE_INTENSITY      52       /**< 3d intensity image */
#define CAMERA_MODE_TYPE_INTENSITY 53       /**< 3d type and intensity image */
#define CAMERA_MODE_RANGE_TYPE     54       /**< 3d range and type image */
#define CAMERA_MODE_SEGMENT        55       /**< 3d segment image */
#define CAMERA_MODE_ELEVATION      56       /**< 3d elevation image */
#define CAMERA_MODE_TYPE           57       /**< 3d type image */
#define CAMERA_MODE_EDGE           58       /**< 3d edge image */
#define CAMERA_MODE_DISPARITY      59       /**< 3d disparity image */

//Pattern for raw images
#define COLORFILTER_RGGB  512
#define COLORFILTER_GBRG  513
#define COLORFILTER_GRBG  514
#define COLORFILTER_BGGR  515

//######################################################################
//# Camera Data (static size  - MESSAGE)
//######################################################################

/**
 * camera data structure
 */
typedef struct {
    rack_time_t recordingTime;              /**< [ms] global timestamp (has to be first element)*/
    uint16_t    width;                      /**< [px] width of the camera image  */
    uint16_t    height;                     /**< [px] height of the camera image */
    uint16_t    depth;                      /**< [bit/px] depth per pixel of the camera image */
    uint16_t    mode;                       /**< color mode of the camera image */
    uint32_t    colorFilterId;              /**< id of the color filter pattern */
    uint8_t     byteStream[0];              /**< byte stream with the image data */
} __attribute__((packed)) camera_data;

class CameraData
{
    public:
        static void le_to_cpu(camera_data *data)
        {
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->width         = __le16_to_cpu(data->width);
            data->height        = __le16_to_cpu(data->height);
            data->depth         = __le16_to_cpu(data->depth);
            data->mode          = __le16_to_cpu(data->mode);
            data->colorFilterId = __le32_to_cpu(data->colorFilterId);
        }

        static void be_to_cpu(camera_data *data)
        {
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->width         = __be16_to_cpu(data->width);
            data->height        = __be16_to_cpu(data->height);
            data->depth         = __be16_to_cpu(data->depth);
            data->mode          = __be16_to_cpu(data->mode);
            data->colorFilterId = __be32_to_cpu(data->colorFilterId);
        }

        static camera_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            camera_data *p_data = (camera_data *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }

};

typedef struct {
    camera_data     data;
    uint8_t         byteStream[CAMERA_MAX_BYTES];
} __attribute__((packed)) camera_data_msg;


//######################################################################
//# Camera Parameter Data (static size  - MESSAGE)
//######################################################################

/**
 * camera parameter data structure
 */
typedef struct
{
  int32_t  calibration_width;               /**< [px] width  */
  int32_t  calibration_height;              /**< [px] height */
  float    f;                               /**< [mm] focus of lens */
  float    fx;                              /**< [px] focus in pixel= 4.8 mm */
  float    fy;                              /**< [px] focus in pixel= 4.8 mm */
  float    sx;                              /**< scaling factor in x-direction */
  float    sy;                              /**< scaling factor in y-direction */
  float    dx;                              /**< [mm] width of the sensor element */
  float    dy;                              /**< [mm] height of the sensor element */
  float    k1;                              /**< [1/mm^2] first radial coeffizient */
  float    k2;                              /**< [1/mm^2] second radial coeffizient */
  float    p1;                              /**< [1/mm] first tangential coeffizient */
  float    p2;                              /**< [1/mm] second tangential coeffizient */
  float    e0;                              /**< [px] z-axis intercept of camera coordinate
                                                      system x-axis */
  float    n0;                              /**< [px] z-axis intercept of camera coordinate system
                                                      y-axis, coordinates of center of radial lens
                                                      distortion (also used as the piercing point
                                                      of the camera coordinate frame's z-axis with
                                                      the camera's sensor plane) */
  float    coordinateRotation[9];           /**< [rad] rotation matrix (extrinsic parameter) */
  float    coordinateTranslation[3];        /**< [mm] translation vector (extrinsic parameter) */

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
            for(int i = 0; i < 9; i++){
                data->coordinateRotation[i] = __le32_float_to_cpu(data->coordinateRotation[i]);}
            for(int i = 0; i < 3; i++){
                data->coordinateTranslation[i] = __le32_float_to_cpu(data->coordinateTranslation[i]);}
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
            for(int i = 0; i < 9; i++){
                data->coordinateRotation[i] = __be32_float_to_cpu(data->coordinateRotation[i]);}
            for(int i = 0; i < 3; i++){
                data->coordinateTranslation[i] = __be32_float_to_cpu(data->coordinateTranslation[i]);}
        }

        static camera_param_data *parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            camera_param_data *p_data = (camera_param_data *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }

};


//######################################################################
//# Camera Format Data (static size  - MESSAGE)
//######################################################################

/**
 * camera format data structure
 */
typedef struct
{
  int32_t   width;                          /**< [px] width of the camera image */
  int32_t   height;                         /**< [px] height of the camera image */
  int32_t   depth;                          /**< [bit/px] depth per pixel of the camera image */
  int32_t   mode;                           /**< color mode of the camera image */

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

        static camera_format_data *parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            camera_format_data *p_data = (camera_format_data *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe()) // data in little endian
            {
                le_to_cpu(p_data);
            }
            else // data in big endian
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }

};

/**
 * Hardware abstraction for optical cameras.
 *
 * @ingroup proxies_drivers
 */
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

    int getData(camera_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp)
    {
        return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(camera_data *recv_data, ssize_t recv_datalen, rack_time_t timeStamp,
                uint64_t reply_timeout_ns);


// getParam


    int getParam(camera_param_data *recv_data, ssize_t recv_datalen)
    {
        return getParam(recv_data, recv_datalen, dataTimeout);
    }

    int getParam(camera_param_data *recv_data, ssize_t recv_datalen,
                 uint64_t reply_timeout_ns);

};

#endif // __CAMERA_PROXY_H__
