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
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#ifndef __POSITION3D_H__
#define __POSITION3D_H__

/**
 * position 3d structure
 * @ingroup main_defines
 */
typedef struct {
    int32_t     x;                          /**< [mm] x-coordinate */
    int32_t     y;                          /**< [mm] y-coordinate */
    int32_t     z;                          /**< [mm] z-coordinate */
    float       phi;                        /**< [rad] rotation about x-axis, positive clockwise */
    float       psi;                        /**< [rad] rotation about y-axis, positive clockwise */
    float       rho;                        /**< [rad] rotation about z-axis, positive clockwise */
} __attribute__((packed)) position_3d;

class Position3D
{
    public:
        static void le_to_cpu(position_3d *data)
        {
            data->x   = __le32_to_cpu(data->x);
            data->y   = __le32_to_cpu(data->y);
            data->z   = __le32_to_cpu(data->z);
            data->phi = __le32_float_to_cpu(data->phi);
            data->psi = __le32_float_to_cpu(data->psi);
            data->rho = __le32_float_to_cpu(data->rho);
        }

        static void be_to_cpu(position_3d *data)
        {
            data->x   = __be32_to_cpu(data->x);
            data->y   = __be32_to_cpu(data->y);
            data->z   = __be32_to_cpu(data->z);
            data->phi = __be32_float_to_cpu(data->phi);
            data->psi = __be32_float_to_cpu(data->psi);
            data->rho = __be32_float_to_cpu(data->rho);
        }

};

#endif /*__POSITION3D_H__*/
