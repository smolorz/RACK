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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */

#ifndef __POSITION_TOOL_H__
#define __POSITION_TOOL_H__

#include <main/rack_debug.h>
#include <navigation/position_proxy.h>
#include <math.h>

typedef struct
{
    double    northing;
    double    easting;
    double    altitude;
} position_gk_data;

#define AWGS        6378137.0        // Wgs84 semi-major axis  = equatorial radius in meter
#define BWGS        6356752.31425    // Wgs84 semi-minor axis  = polar radius in meter
#define ABES        6377397.15508    // Bessel semi-major axis = equatorial radius in meter
#define BBES        6356078.96290    // Bessel semi-minor axis = polar radius in meter

#define HELM_DX     -585.7           // Helmert translation parameter 1
#define HELM_DY     -87.0            // Helmert translation parameter 2
#define HELM_DZ     -409.2           // Helmert translation parameter 3
#define HELM_ROTX   2.540423689E-6   // Helmert rotation parameter 1
#define HELM_ROTY   7.514612057E-7   // Helmert rotation parameter 2
#define HELM_ROTZ   -1.368144208E-5  // Helmert rotation parameter 3
#define HELM_SC     1.0 / 0.99999122 // Helmert scaling factor

//######################################################################
//# Class PositionTool
//######################################################################

class PositionTool {
  private:

    // only for debugging:
    GdosMailbox    *gdos;

  public:

    PositionTool();
    PositionTool(RackMailbox *p_mbx, int gdos_level);

    void wgs84ToGk(position_wgs84_data *posWgs84, position_gk_data *posGk);
    void gkToWgs84(position_gk_data *posGk, position_wgs84_data *posWgs84);

    ~PositionTool();
};

#endif // __POSITION_TOOL_H__
