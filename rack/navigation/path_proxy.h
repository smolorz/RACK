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
 *      Matthias Hentschel  <hentschel@rts.uni-hannover.de>
 *
 */
#ifndef __PATH_PROXY_H__
#define __PATH_PROXY_H__

/*!
 * @ingroup rtsnavigation
 * @defgroup path Path
 *
 * Navigation components that do path planning.
 *
 * @{
 */

#include <main/rack_proxy.h>
#include <main/defines/polar_spline.h>
#include <main/defines/position2d.h>

//######################################################################
//# Path Message Types
//######################################################################

#define MSG_PATH_MAKE                   (RACK_PROXY_MSG_POS_OFFSET + 1)
#define MSG_PATH_REPLAN                 (RACK_PROXY_MSG_POS_OFFSET + 2)
#define MSG_PATH_SET_DESTINATION        (RACK_PROXY_MSG_POS_OFFSET + 3)
#define MSG_PATH_SET_RDDF               (RACK_PROXY_MSG_POS_OFFSET + 4)
#define MSG_PATH_GET_DBG_GRIDMAP        (RACK_PROXY_MSG_POS_OFFSET + 5)
#define MSG_PATH_GET_LAYER              (RACK_PROXY_MSG_POS_OFFSET + 6)
#define MSG_PATH_SET_LAYER              (RACK_PROXY_MSG_POS_OFFSET + 7)

#define MSG_PATH_DBG_GRIDMAP            (RACK_PROXY_MSG_NEG_OFFSET - 1)
#define MSG_PATH_LAYER                  (RACK_PROXY_MSG_POS_OFFSET - 2)

//######################################################################
//# Path data defines
//######################################################################
#define PATH_SPLINE_MAX            5000

//######################################################################
//# PathData (!!! VARIABLE SIZE !!! MESSAGE !!!)
//######################################################################

/* CREATING A MESSAGE :

typedef struct {
    path_data     data;
    polar_spline  spline[ ... ];
} __attribute__((packed)) path_data_msg;

path_data_msg msg;

ACCESS: msg.data.spline[...] OR msg.spline[...];

*/

typedef struct {
    rack_time_t     recordingTime; // has to be first element
    int32_t         splineNum;
    polar_spline    spline[0];
} __attribute__((packed)) path_data;

class PathData
{
    public:
        static void le_to_cpu(path_data *data)
        {
            int i;
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->splineNum     = __le32_to_cpu(data->splineNum);
            for (i=0; i<data->splineNum; i++)
            {
                PolarSpline::le_to_cpu(&data->spline[i]);
            }
        }

        static void be_to_cpu(path_data *data)
        {
            int i;
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->splineNum     = __be32_to_cpu(data->splineNum);
            for (i=0; i<data->splineNum; i++)
            {
                PolarSpline::be_to_cpu(&data->spline[i]);
            }
        }

        static path_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            path_data *p_data = (path_data *)msgInfo->p_data;

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
//# PathRddfData (!!! VARIABLE SIZE !!! MESSAGE !!!)
//######################################################################

/* CREATING A MESSAGE :

typedef struct {
    path_rddf_data     data;
    position_2d        basePoint[ ... ];
} __attribute__((packed)) path_rddf_data_msg;

path_rddf_data_msg msg;

ACCESS: msg.data.basePoint[...] OR msg.basePoint[...];
*/

typedef struct {
    int32_t       waypointNum;
    waypoint_2d   waypoint[0];
} __attribute__((packed)) path_rddf_data;

class PathRddfData
{
    public:
        static void le_to_cpu(path_rddf_data *data)
        {
            int i;
            data->waypointNum     = __le32_to_cpu(data->waypointNum);
            for (i = 0; i < data->waypointNum; i++)
            {
                Waypoint2d::le_to_cpu(&data->waypoint[i]);
            }
        }

        static void be_to_cpu(path_rddf_data *data)
        {
            int i;
            data->waypointNum     = __be32_to_cpu(data->waypointNum);
            for (i = 0; i < data->waypointNum; i++)
            {
                Waypoint2d::be_to_cpu(&data->waypoint[i]);
            }
        }

        static path_rddf_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            path_rddf_data *p_data = (path_rddf_data *)msgInfo->p_data;

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
//# Path Make Data (static size - MESSAGE)
//######################################################################
typedef struct {
  int32_t       vMax;
  int32_t       accMax;
  int32_t       decMax;
  float         omegaMax;
} __attribute__((packed)) path_make_data;

class PathMakeData
{
    public:
        static void le_to_cpu(path_make_data *data)
        {
            data->vMax     = __le32_to_cpu(data->vMax);
            data->accMax   = __le32_to_cpu(data->accMax);
            data->decMax   = __le32_to_cpu(data->decMax);
            data->omegaMax = __le32_float_to_cpu(data->omegaMax);
        }

        static void be_to_cpu(path_make_data *data)
        {
            data->vMax     = __be32_to_cpu(data->vMax);
            data->accMax   = __be32_to_cpu(data->accMax);
            data->decMax   = __be32_to_cpu(data->decMax);
            data->omegaMax = __be32_float_to_cpu(data->omegaMax);
        }

        static path_make_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            path_make_data *p_data = (path_make_data *)msgInfo->p_data;

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
//# Path Replan Data (static size - MESSAGE)
//######################################################################

typedef struct {
  int32_t       currSpline;
  position_2d   splinePos;
  int32_t       distToObstacle;
  int32_t       pathLength;
} __attribute__((packed)) path_replan_data;

class PathReplanData
{
    public:
        static void le_to_cpu(path_replan_data *data)
        {
            data->currSpline     = __le32_to_cpu(data->currSpline);
            Position2D::le_to_cpu(&data->splinePos);
            data->distToObstacle = __le32_to_cpu(data->distToObstacle);
            data->pathLength     = __le32_to_cpu(data->pathLength);
        }

        static void be_to_cpu(path_replan_data *data)
        {
            data->currSpline     = __be32_to_cpu(data->currSpline);
            Position2D::be_to_cpu(&data->splinePos);
            data->distToObstacle = __be32_to_cpu(data->distToObstacle);
            data->pathLength     = __be32_to_cpu(data->pathLength);
        }

        static path_replan_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            path_replan_data *p_data = (path_replan_data *)msgInfo->p_data;

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
//# Path Destination Data (static size  - MESSAGE)
//######################################################################

typedef struct
{
    position_2d pos;
    int32_t     speed;
    int32_t     layer;
} __attribute__((packed)) path_dest_data;

class PathDestData
{
    public:
        static void le_to_cpu(path_dest_data *data)
        {
            Position2D::le_to_cpu(&data->pos);
            data->speed = __le32_to_cpu(data->speed);
            data->layer = __le32_to_cpu(data->layer);
        }

        static void be_to_cpu(path_dest_data *data)
        {
            Position2D::be_to_cpu(&data->pos);
            data->speed = __be32_to_cpu(data->speed);
            data->layer = __be32_to_cpu(data->layer);
        }

        static path_dest_data *parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            path_dest_data *p_data = (path_dest_data *)msgInfo->p_data;

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
//# Path Layer Data (static size  - MESSAGE)
//######################################################################

typedef struct
{
    int32_t     layer;
} __attribute__((packed)) path_layer_data;

class PathLayerData
{
    public:
        static void le_to_cpu(path_layer_data *data)
        {
            data->layer = __le32_to_cpu(data->layer);
        }

        static void be_to_cpu(path_layer_data *data)
        {
            data->layer = __be32_to_cpu(data->layer);
        }

        static path_layer_data *parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            path_layer_data *p_data = (path_layer_data *)msgInfo->p_data;

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
//# Path Proxy Functions
//######################################################################

class PathProxy : public RackDataProxy {

  public:

//
// constructor / destructor
// WARNING -> look at module class id in constuctor
//

    PathProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
            : RackDataProxy(workMbx, sys_id, PATH, instance)
    {
    };

    ~PathProxy()
    {
    };


//
// overwriting getData proxy function
// (includes parsing and type conversion)
//

    int getData(path_data *recv_data, ssize_t recv_datalen,
                rack_time_t timeStamp)
    {
        return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(path_data *recv_data, ssize_t recv_datalen,
                rack_time_t timeStamp, uint64_t reply_timeout_ns);

    int getNextData(path_data *recv_data, ssize_t recv_datalen,
                 uint64_t reply_timeout_ns);

// path_make

    int make(path_make_data *recv_data, ssize_t recv_datalen)
    {
        return make(recv_data, recv_datalen, dataTimeout);
    }

    int make(path_make_data *recv_data, ssize_t recv_datalen,
             uint64_t reply_timeout_ns);


// path_replan

    int replan(path_replan_data *recv_data, ssize_t recv_datalen)
    {
        return replan(recv_data, recv_datalen, dataTimeout);
    }

    int replan(path_replan_data *recv_data, ssize_t recv_datalen,
               uint64_t reply_timeout_ns);


 // set_destination

    int setDestination(path_dest_data *recv_data, ssize_t recv_datalen)
    {
        return setDestination(recv_data, recv_datalen, dataTimeout);
    }

    int setDestination(path_dest_data *recv_data, ssize_t recv_datalen,
                       uint64_t reply_timeout_ns);


// set_rddf
    int setRddf(path_rddf_data *recv_data, ssize_t recv_datalen)
    {
        return setRddf(recv_data, recv_datalen, dataTimeout);
    }

    int setRddf(path_rddf_data *recv_data, ssize_t recv_datalen,
                uint64_t reply_timeout_ns);

// get_dbg_gridmap

    // still to do...

//get_layer
    int getLayer(path_layer_data *recv_data, ssize_t recv_datalen)
    {
        return getLayer(recv_data, recv_datalen, dataTimeout);
    }

    int getLayer(path_layer_data *recv_data, ssize_t recv_datalen,
                 uint64_t reply_timeout_ns);

//set_layer
    int setLayer(path_layer_data *recv_data, ssize_t recv_datalen)
    {
        return setLayer(recv_data, recv_datalen, dataTimeout);
    }

    int setLayer(path_layer_data *recv_data, ssize_t recv_datalen,
                 uint64_t reply_timeout_ns);
};

/*@}*/

#endif // __PATH_PROXY_H__
