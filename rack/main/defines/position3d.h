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

//######################################################################
//#  Position3D (static size - no message)
//######################################################################

typedef struct {
    int32_t     x;      // in mm
    int32_t     y;      // in mm
    int32_t     z;      // in mm
    float32_t   phi;    // Drehung um x-Achse in rad von 0 <= x < 2PI
    float32_t   psi;    // Drehung um y-Achse in rad von 0 <= x < 2PI
    float32_t   rho;    // Drehung um z-Achse in rad von 0 <= x < 2PI
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
