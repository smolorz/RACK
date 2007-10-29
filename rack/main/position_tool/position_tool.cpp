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
 *      Matthias Hentschel  <hentschel@rts.uni-hannover.de>
 *
 */

#include <main/position_tool.h>

PositionTool::PositionTool()
{
    gdos = NULL;
}

PositionTool::PositionTool(RackMailbox *p_mbx, int gdos_level)
{
    gdos = new GdosMailbox(p_mbx, gdos_level);
}

PositionTool::~PositionTool()
{
    if (gdos)
        delete gdos;
}

void PositionTool::wgs84ToGk(position_wgs84_data *posWgs84, position_gk_data *posGk)
{
    double  altitude;
    double  sinLat, cosLat;
    double  sinLon, cosLon;
    double  sinPhi;
    double  c, s, t;
    double  xq, yq, zq;
    double  xz, yz, zz;
    double  phi, lambda, h;
    double  eq, n;
    double  r, b1;
    double  l, l0;
    double  est, eta, eq2;
    double  cbess;
    double  a1, a2, a3, a4, a5;
    double  alpha, beta, gamma, delta;
    double  gb;
    double  x, y;

    // precalculate values
    sinLat   = sin(posWgs84->latitude);
    cosLat   = cos(posWgs84->latitude);
    sinLon   = sin(posWgs84->longitude);
    cosLon   = cos(posWgs84->longitude);
    altitude = (double)posWgs84->altitude / 1000.0;             // unit m

    // Transformation of elliptic coordinates (latitude, longitude, altitude) into
    // geocentric cartesian coordinates (xq, yq, zq) in wgs84 ellipsoid
    eq = (AWGS * AWGS - BWGS * BWGS) / (AWGS * AWGS);
    n  = AWGS / sqrt(1.0 - eq * sinLat * sinLat);
    xq = (n + altitude) * cosLat * cosLon;
    yq = (n + altitude) * cosLat * sinLon;
    zq = ((1.0 - eq) * n + altitude) * sinLat;

    // Helmert transformation
    xz = HELM_DX + HELM_SC * (       1.0 * xq  + HELM_ROTZ * yq - HELM_ROTY * zq);
    yz = HELM_DY + HELM_SC * (-HELM_ROTZ * xq  +         1 * yq + HELM_ROTX * zq);
    zz = HELM_DZ + HELM_SC * ( HELM_ROTY * xq  - HELM_ROTX * yq +       1.0 * zq);

    // Calculation of elliptic coordinates (phi, lambda, h) in Potsdam-Date
    // from geocentric cartesian coordinates (xz, yz, zz)
    eq     = (ABES * ABES - BBES * BBES) / (ABES * ABES);
    lambda = atan2(yz, xz);
    r      = sqrt(xz * xz + yz * yz);
    phi    = atan(zz / r);
    sinPhi = sin(phi);
    n      = ABES / sqrt(1.0 - eq * sinPhi * sinPhi);

    do
    {
        b1  = phi;
        phi = atan((zz + eq * n * sin(b1)) / r);
        sinPhi = sin(phi);
        n   = ABES / sqrt(1 - eq * sinPhi * sinPhi);
    }
    while (fabs(b1 - phi) > 10E-10);

    h = (xz / (cos(lambda) * cos(phi))) - n;

    // Transformation of elliptic coordinates (phi, lambda, h) in Potsdam - date
    // into Gauss-Krueger coordinates (northing, easting, altitude)
    l0      = 3.0 * round((lambda * 180.0 / M_PI) / 3.0) * M_PI / 180.0;
    l       = lambda - l0;
    c       = cos(phi);
    s       = sin(phi);
    t       = tan(phi);
    n       = ABES / sqrt(1.0 - eq * s * s);
    est     = sqrt(eq / (1.0 - eq));
    eta     = est * c;
    eq2     = (ABES * ABES - BBES * BBES) / (BBES * BBES);
    cbess   = ABES / sqrt(1.0 - eq);
    a1      = n * c;
    a2      = (1.0 /   2.0) * t * n * c * c;
    a3      = (1.0 /   6.0) * n * c * c * c * (1.0 - t * t + eta * eta);
    a4      = (1.0 /  24.0) * n * s * c * c * c * (5.0 - t * t + 9.0 * eta *eta +
              4.0 * eta *eta *eta *eta);
    a5      = (1.0 / 120.0) * n * c * c * c * c * c * (5.0 - 18.0 * t * t + t * t * t * t +
              14.0 * eta * eta - 58.0 * eta *eta * t * t + 13.0 * eta * eta * eta * eta -
              64.0 * eta * eta * eta * eta * t * t);
    alpha   = (1.0 - (3.0 / 4.0) * eq2  + (45.0 / 64.0) * eq2 * eq2 -
              (175.0 / 256.0) * eq2 * eq2 * eq2 + (11025.0 / 16384.0) * eq2 * eq2 * eq2 * eq2);
    beta    = (1.0 / 2.0) * ((3.0 / 4.0) * eq2  - (15.0 / 16.0) * eq2 * eq2 +
              (525.0 / 512.0) * eq2 * eq2 * eq2 -
              ( 2205.0 /  2048.0) * eq2 * eq2 * eq2 * eq2);
    gamma   = (1.0 / 4.0) * ((15.0 / 64.0) * eq2 * eq2 - (105.0 / 256.0) * eq2 * eq2 * eq2 +
              ( 2205.0 /  4096.0) * eq2 * eq2 * eq2 * eq2);
    delta   = (1.0 / 6.0) * (( 35.0 / 512.0) * eq2 * eq2 * eq2 +
              ( 2205.0 /  4096.0) * eq2 * eq2 * eq2 * eq2);
    gb      = (alpha * phi - beta * sin(2.0 * phi) + gamma * sin(4.0 * phi) -
              delta * sin(6.0 * phi)) * cbess;

    x       =          a2 * l * l     + a4 * l * l * l * l;
    y       = a1 * l + a3 * l * l * l + a5 * l * l * l * l * l;

    posGk->northing = (gb + x) * 1000.0;
    posGk->easting  = (1000000.0 * round((lambda * 180.0 / M_PI) / 3.0) + 500000.0 + y) * 1000.0;
    posGk->altitude = h * 1000.0;
}


void PositionTool::gkToWgs84(position_gk_data *posGk, position_wgs84_data *posWgs84)
{
    double  northing, easting, altitude;
    double  sinPhi, cosPhi;
    double  sinLambda, cosLambda;
    double  sinLat;
    double  eq, n, r;
    double  xz, yz, zz;
    double  xq, yq, zq;
    double  lon, lat;
    double  y, yOld;
    double  l0, est, i;
    double  grd, sigma, bf;
    double  s, c, t;
    double  nf, mf, eta;
    double  b1, b2, b3, b4, b5;
    double  phi, lambda, h;

    // unit m
    northing = posGk->northing / 1000.0;
    easting  = posGk->easting / 1000.0;
    altitude = posGk->altitude / 1000.0;

    l0      = -1.0;
    y       = easting;
    yOld    = y;

    do
    {
        l0   = l0 + 1.0;
        yOld = y;
        y    = easting - 500000.0 - 1000000.0 * l0 / 3.0;
    }
    while (fabs(y) <= fabs(yOld));

    l0     = (l0 - 1.0)* M_PI / 180.0;
    y      = easting - 500000.0 - 1000000 * l0 * (180.0 / M_PI) / 3.0;
    eq     = (ABES * ABES - BBES * BBES) / (ABES * ABES);
    est    = sqrt(ABES * ABES - BBES * BBES) / BBES;
    i      = (ABES - BBES) / (ABES + BBES);
    grd    = (ABES / (1.0 + i)) * (1.0 + 0.25 * i * i + (1.0 / 64.0) * i * i * i * i);
    sigma  = northing / grd;
    bf     = sigma + (3.0 / 2.0) * (i - (9.0 / 16.0) * i * i * i) * sin(2.0 * sigma) +
             (21.0 / 16.0) * i * i * sin(4.0 * sigma) + (151.0 / 96.0) * i * i * i * sin(6.0 * sigma);
    s      = sin(bf);
    c      = cos(bf);
    t      = tan(bf);
    nf     = ABES / sqrt(1.0 - eq * s * s);
    mf     = (ABES * (1.0 - eq)) / pow((1.0 - eq * s * s), 3/2);
    eta    = est * c;

    b1     =  1.0 / (nf * c);
    b2     = -t / (2.0 * mf * nf);
    b3     = -(1.0 + 2.0 * t * t + eta * eta) / (6.0 * nf * nf * nf * c);
    b4     =  (t / (24.0 * mf * nf * nf * nf)) * (5.0 + 3.0 * t * t + eta * eta - 9.0 * eta * eta * t * t -
                                                                        4.0 * eta * eta * eta * eta);
    b5     =  (1.0 / (120.0 * nf * nf * nf * nf * nf * c)) * (28.0 * t * t + 24.0 * t * t * t * t +
                                                            6.0 * eta * eta + 8.0 * eta * eta * t * t);

    phi    = bf     + b2 * y * y       + b4 * y * y * y * y;
    lambda = b1 * y + b3 * y * y * y   + b5 * y * y * y * y * y;
    lambda = lambda + l0;
    h      = altitude;

    // transformation of elliptic coordinates (phi, lambda, h) in Potsdam - date
    // to geocentric cartesian coordinates (xz, yz, zz) in wgs84 ellipsoid
    sinPhi    = sin(phi);
    cosPhi    = cos(phi);
    cosLambda = cos(lambda);
    sinLambda = sin(lambda);
    eq        = (ABES * ABES - BBES * BBES) / (ABES * ABES);
    n         = ABES / sqrt(1.0 - eq * sinPhi * sinPhi);
   xz        = (n + h)   * cosPhi * cosLambda;
    yz        = (n + h)   * cosPhi * sinLambda;
    zz        = ((1.0 - eq) * n + h) * sinPhi;

    // inverse helmert transformation
    xz = xz - HELM_DX;
    yz = yz - HELM_DY;
    zz = zz - HELM_DZ;
    xq = (       1.0 * xz - HELM_ROTZ * yz + HELM_ROTY * zz) / HELM_SC;
    yq = ( HELM_ROTZ * xz +       1.0 * yz - HELM_ROTX * zz) / HELM_SC;
    zq = (-HELM_ROTY * xz + HELM_ROTX * yz +       1.0 * zz) / HELM_SC;

    // transformation of geocentric cartesian coordinates  (xq, yq, zq) in
    // elliptic coordinates (lat, lon, alt) in wgs84 ellipsoid
    eq     = (AWGS * AWGS - BWGS * BWGS) / (AWGS * AWGS);
    lon    = atan2(yq, xq);
    r      = sqrt(xq * xq + yq * yq);
    lat    = atan(zq / r);
    sinLat = sin(lat);
    n      = AWGS / sqrt(1.0 - eq * sinLat * sinLat);

    do
    {
        b1  = lat;
        lat = atan((zq + eq * n * sin(b1)) / r);
        n   = AWGS / sqrt(1.0 - eq * sin(lat) * sin(lat));
    }
    while (fabs(b1 - lat) > 10E-10);

    posWgs84->latitude  = lat;
    posWgs84->longitude = lon;
    posWgs84->altitude  = (int)rint(((xq / (cos(lat) * cos(lon))) - n) * 1000.0);
}
