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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */

#ifndef __GRID_MAP_PROXY_H__
#define __GRID_MAP_PROXY_H__

#include <main/rack_proxy.h>

#define GRID_MAP_NUM_MAX 500 * 500          /**< maximum number of grid cells */


//######################################################################
//# GridMap Data (static size  - MESSAGE)
//######################################################################

typedef struct {
    rack_time_t recordingTime;              /**< [ms] global timestamp (has to be first element)*/
    int32_t     offsetX;                    /**< [mm] x-position of grid cell[0,0] */
    int32_t     offsetY;                    /**< [mm] y-position of grid cell[0,0] */
    int32_t     scale;                      /**< [mm/cell] scale of the map */
    int32_t     gridNumX;                   /**< number of following cells in x-direction */
    int32_t     gridNumY;                   /**< number of following cells in y-direction */
    uint8_t     occupancy[0];               /**< list of grid cells,
                                                 obstacle propability 0 - free, 255 - occupied */
} __attribute__((packed)) grid_map_data;

class GridMapData
{
    public:
        static void le_to_cpu(grid_map_data *data)
        {
            data->recordingTime = __le32_to_cpu(data->recordingTime);
            data->offsetX       = __le32_to_cpu(data->offsetX);
            data->offsetY       = __le32_to_cpu(data->offsetY);
            data->scale         = __le32_to_cpu(data->scale);
            data->gridNumX      = __le32_to_cpu(data->gridNumX);
            data->gridNumY      = __le32_to_cpu(data->gridNumY);
        }

        static void be_to_cpu(grid_map_data *data)
        {
            data->recordingTime = __be32_to_cpu(data->recordingTime);
            data->offsetX       = __be32_to_cpu(data->offsetX);
            data->offsetY       = __be32_to_cpu(data->offsetY);
            data->scale         = __be32_to_cpu(data->scale);
            data->gridNumX      = __be32_to_cpu(data->gridNumX);
            data->gridNumY      = __be32_to_cpu(data->gridNumY);
        }

        static grid_map_data* parse(RackMessage *msgInfo)
        {
            if (!msgInfo->p_data)
                return NULL;

            grid_map_data *p_data = (grid_map_data *)msgInfo->p_data;

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
 * Navigation components that build occupancy grid maps.
 *
 * @ingroup proxies_navigation
 */
class GridMapProxy : public RackDataProxy {

  public:

//
// constructor / destructor
// WARNING -> look at module class id in constuctor
//

    GridMapProxy(RackMailbox *workMbx, uint32_t sys_id, uint32_t instance)
              : RackDataProxy(workMbx, sys_id, GRID_MAP, instance)
    { };

    ~GridMapProxy()
    { };


//
// overwriting getData proxy function
// (includes parsing and type conversion)
//

    int getData(grid_map_data *recv_data, ssize_t recv_datalen,
                rack_time_t timeStamp)
    {
        return getData(recv_data, recv_datalen, timeStamp, dataTimeout);
    }

    int getData(grid_map_data *recv_data, ssize_t recv_datalen,
                rack_time_t timeStamp, uint64_t reply_timeout_ns);
};

#endif // __GRID_MAP_PROXY_H__
