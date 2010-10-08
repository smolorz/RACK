/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Daniel Lecking <lecking@rts.uni-hannover.de>
 */
#ifndef __FEATURE_MAP_PROXY_H__
#define __FEATURE_MAP_PROXY_H__

#include <main/rack_proxy.h>
#include <main/defines/position3d.h>

//######################################################################
//# FEATURE MAP Message Types
//######################################################################
#define MSG_FEATURE_MAP_LOAD_MAP                (RACK_PROXY_MSG_POS_OFFSET + 1)
#define MSG_FEATURE_MAP_ADD_LINE                (RACK_PROXY_MSG_POS_OFFSET + 2)
#define MSG_FEATURE_MAP_SAVE_MAP                (RACK_PROXY_MSG_POS_OFFSET + 3)
#define MSG_FEATURE_MAP_GIVE_MAP                (RACK_PROXY_MSG_POS_OFFSET + 4)
#define MSG_FEATURE_MAP_DELETE_LINE             (RACK_PROXY_MSG_POS_OFFSET + 5)
#define MSG_FEATURE_MAP_DISPLACE_LINE           (RACK_PROXY_MSG_POS_OFFSET + 6)
#define MSG_FEATURE_MAP_GET_LAYER               (RACK_PROXY_MSG_POS_OFFSET + 7)
#define MSG_FEATURE_MAP_SET_LAYER               (RACK_PROXY_MSG_POS_OFFSET + 8)

#define MSG_FEATURE_MAP_LAYER                   (RACK_PROXY_MSG_POS_OFFSET - 1)

//#define MSG_FEATURE_MAP_ADD_LINE_OK            (RACK_PROXY_MSG_NEG_OFFSET - 10)

#define FEATURE_MAP_FEATURE_MAX 200         /**< maximum number of map features */
#define FEATURE_MAP_LAYER_MAX   20

#define FEATURE_MAP_TYPE_LINE_FEATURE   10

//######################################################################
//# FEATURE MAP DataPoint
//######################################################################

/**
 * feature map data point data structure
 */
typedef struct {
    double x;                               /**< [mm] global x-coordinate of the start point
                                                      of the line feature */
    double y;                               /**< [mm] global y-coordinate of the start point
                                                      of the line feature */
    double x2;                              /**< [mm] global x-coordinate of the end point
                                                      of the line feature */
    double y2;                              /**< [mm] global y-coordinate of the end point
                                                      of the line feature */
    double l;                               /**< [mm] length of the line feature */
    double rho;                             /**< [rad] global orientation of the line feature */
    double sin;                             /**< sin(rho) */
    double cos;                             /**< cos(rho) */
    int32_t layer;                          /**< layer id */
    int32_t type;                           /**< point type */
} __attribute__((packed)) feature_map_data_point;

class FeatureMapDataPoint
{
    public:
        static void le_to_cpu(feature_map_data_point *data)
        {
            data->x         = __le64_float_to_cpu(data->x);
            data->y         = __le64_float_to_cpu(data->y);
            data->x2        = __le64_float_to_cpu(data->x2);
            data->y2        = __le64_float_to_cpu(data->y2);
            data->l         = __le64_float_to_cpu(data->l);
            data->rho       = __le64_float_to_cpu(data->rho);
            data->sin       = __le64_float_to_cpu(data->sin);
            data->cos       = __le64_float_to_cpu(data->cos);
            data->layer     = __le32_to_cpu(data->layer);
            data->type      = __le32_to_cpu(data->type);
        }

        static void be_to_cpu(feature_map_data_point *data)
        {
            data->x         = __be64_float_to_cpu(data->x);
            data->y         = __be64_float_to_cpu(data->y);
            data->x2        = __be64_float_to_cpu(data->x2);
            data->y2        = __be64_float_to_cpu(data->y2);
            data->l         = __be64_float_to_cpu(data->l);
            data->rho       = __be64_float_to_cpu(data->rho);
            data->sin       = __be64_float_to_cpu(data->sin);
            data->cos       = __be64_float_to_cpu(data->cos);
            data->layer     = __be32_to_cpu(data->layer);
            data->type      = __be32_to_cpu(data->type);
        }
};


/**
 * feature map feature data structure
 */
typedef struct{
    rack_time_t             recordingTime;  /**< [ms] global timestamp (has to be first element)*/
    feature_map_data_point   mapFeature;    /**< line feature */
    int32_t                  featureNumTemp;
} __attribute__((packed)) feature_map_feature;

class FeatureMapFeature
{
    public:
        static void le_to_cpu(feature_map_feature *data)
        {
            data->recordingTime    = __le32_to_cpu(data->recordingTime);
            data->featureNumTemp    = __le32_to_cpu(data->featureNumTemp);
            FeatureMapDataPoint::le_to_cpu(&data->mapFeature);
        }

        static void be_to_cpu(feature_map_feature *data)
        {
            data->recordingTime    = __le32_to_cpu(data->recordingTime);
            data->featureNumTemp    = __be32_to_cpu(data->featureNumTemp);
            FeatureMapDataPoint::be_to_cpu(&data->mapFeature);
        }

        static feature_map_feature* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            feature_map_feature *p_data = (feature_map_feature *)msgInfo->p_data;

            if (msgInfo->isDataByteorderLe())
            {
                le_to_cpu(p_data);
            }
            else
            {
                be_to_cpu(p_data);
            }
            msgInfo->setDataByteorder();
            return p_data;
        }
};


//######################################################################
//# FeatureMapData (!!! VARIABLE SIZE !!! MESSAGE !!!)
//######################################################################

/*// CREATING A MESSAGE :

typedef struct {
    feature_map_data     data;
    feature_map_data_point    feature[ ... ];
} __attribute__((packed)) feature_map_data_msg;

feature_map_data_msg msg;

//ACCESS: msg.data.point[...] OR msg.point[...];*/

/**
 * feature map data structure
 */
typedef struct {
    rack_time_t     recordingTime;          /**< [ms] global timestamp (has to be first element)*/
    position_3d     pos;                    /**< global position and orientation of the robot */
    int32_t         featureNum;             /**< number of following map features */
    feature_map_data_point  feature[0];     /**< list of map features */
} __attribute__((packed)) feature_map_data;


class FeatureMapData
{
    public:
        static void le_to_cpu(feature_map_data *data)
        {
            int i;
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            Position3D::le_to_cpu(&data->pos);
            data->featureNum      = __le32_to_cpu(data->featureNum);
            for (i=0; i < data->featureNum; i++)
            {
                FeatureMapDataPoint::le_to_cpu(&data->feature[i]);
            }
        }

        static void be_to_cpu(feature_map_data *data)
        {
            int i;
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            Position3D::be_to_cpu(&data->pos);
            data->featureNum      = __be32_to_cpu(data->featureNum);
            for (i=0; i < data->featureNum; i++)
            {
                FeatureMapDataPoint::be_to_cpu(&data->feature[i]);
            }
        }

        static feature_map_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            feature_map_data *p_data = (feature_map_data *)msgInfo->p_data;

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
 * feature map filename data structure
 */
typedef struct {
    int32_t     filenameLen;                /**< lenght of file name */
    char        filename[128];              /**< file name */
} __attribute__((packed)) feature_map_filename;

class FeatureMapFilename
{
    public:
        static void le_to_cpu(feature_map_filename *data)
        {
            data->filenameLen = __le32_to_cpu(data->filenameLen);
        }

        static void be_to_cpu(feature_map_filename *data)
        {
            data->filenameLen = __be32_to_cpu(data->filenameLen);
        }

        static feature_map_filename* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            feature_map_filename *p_data = (feature_map_filename*)msgInfo->p_data;

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
//# FeatureMap Layer Data (static size  - MESSAGE)
//######################################################################

/**
 * feature map layer data structure
 */
typedef struct {
    int32_t     layerNum;                   /**< number of following layer */
    int32_t     layer[0];                   /**< list of layer */
} __attribute__((packed)) feature_map_layer_data;

class FeatureMapLayerData
{
    public:
        static void le_to_cpu(feature_map_layer_data *data)
        {
            int i;
            data->layerNum = __le32_to_cpu(data->layerNum);

            for (i = 0 ; i < data->layerNum; i++)
            {
                data->layer[i] = __le32_to_cpu(data->layer[i]);
            }
        }

        static void be_to_cpu(feature_map_layer_data *data)
        {
            int i;
            data->layerNum = __be32_to_cpu(data->layerNum);

            for (i = 0 ; i < data->layerNum; i++)
            {
                data->layer[i] = __be32_to_cpu(data->layer[i]);
            }
        }

        static feature_map_layer_data *parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            feature_map_layer_data *p_data = (feature_map_layer_data *)msgInfo->p_data;

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
//# FeatureMap Proxy Functions
//######################################################################

/**
 * Navigation components that build feature maps.
 *
 * @ingroup proxies_navigation
 */
class FeatureMapProxy : public RackDataProxy {

  public:

//
// constructor / destructor
// WARNING -> look at module class id in constuctor
//

    FeatureMapProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
            : RackDataProxy(workMbx, sys_id, FEATURE_MAP, instance)
    {
    };

    ~FeatureMapProxy()
    {
    };


//
// overwriting getData proxy function
// (includes parsing and type conversion)
//

    int getData(feature_map_data *recv_data, ssize_t recv_datalen,
                rack_time_t timeStamp)
    {
        return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(feature_map_data *recv_data, ssize_t recv_datalen,
                rack_time_t timeStamp, uint64_t reply_timeout_ns);

/*
 * loadMap, addLine etc. implementieren
 */
    int addLine(feature_map_feature *recv_data, ssize_t recv_datalen,
                       uint64_t reply_timeout_ns);

    int addLine(feature_map_feature *recv_data, ssize_t recv_datalen)
    {
        return addLine(recv_data, recv_datalen, dataTimeout);
    }

    int saveMap(feature_map_filename *recv_data, ssize_t recv_datalen,
                       uint64_t reply_timeout_ns);

    int saveMap(feature_map_filename *recv_data, ssize_t recv_datalen)
    {
        return saveMap(recv_data, recv_datalen, dataTimeout);
    }

    int deleteLine(feature_map_feature *recv_data, ssize_t recv_datalen,
                       uint64_t reply_timeout_ns);

    int deleteLine(feature_map_feature *recv_data, ssize_t recv_datalen)
    {
        return deleteLine(recv_data, recv_datalen, dataTimeout);
    }

    int displaceLine(feature_map_feature *recv_data, ssize_t recv_datalen,
                       uint64_t reply_timeout_ns);

    int displaceLine(feature_map_feature *recv_data, ssize_t recv_datalen)
    {
        return displaceLine(recv_data, recv_datalen, dataTimeout);
    }

    int getLayer(feature_map_layer_data *recv_data, ssize_t recv_datalen)
    {
        return getLayer(recv_data, recv_datalen, dataTimeout);
    }

    int getLayer(feature_map_layer_data *recv_data, ssize_t recv_datalen,
                 uint64_t reply_timeout_ns);


    int setLayer(feature_map_layer_data *recv_data, ssize_t recv_datalen)
    {
        return setLayer(recv_data, recv_datalen, dataTimeout);
    }

    int setLayer(feature_map_layer_data *recv_data, ssize_t recv_datalen,
                 uint64_t reply_timeout_ns);
};

#endif // __FEATURE_MAP_PROXY_H__
