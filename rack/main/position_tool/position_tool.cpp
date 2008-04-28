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

void PositionTool::wgs84ToUtm(position_wgs84_data *posWgs84, position_utm_data *posUtm)
{
    int    latDeg, lonDeg;
    double lat, lon;
    double dlam, dlam2, dlam3, dlam4;
    double dlam5, dlam6, dlam7, dlam8;
    double s, s2;
    double sn;
    double c, c2, c3, c5, c7;
    double t, tan2, tan3, tan4, tan5, tan6;
    double t1, t2, t3, t4, t5, t6, t7, t8, t9;
    double eta, eta2, eta3, eta4;
    double tn, tn2, tn3, tn4, tn5;
    double tmd;
    double tmdo;
    double tranMercB;
    double tranMercEs;
    double tranMercEbs;
    double tranMercAp, tranMercBp;
    double tranMercCp, tranMercDp, tranMercEp;
    double centralMeridian = 0.0;
    double easting, northing;
    double falseEasting    = 500000;
    double falseNorthing   = 0;
    double utmScale        = 0.9996;
    static double utmA     = 6378137.0;             // Semi-major axis of ellipsoid in meters
    static double utmF     = 1.0 / 298.257223563;   // flattening of ellipsoid

    lat = posWgs84->latitude;
    lon = posWgs84->longitude;
    if (lon < 0)
    {
        lon += (2.0 * M_PI) + 1.0e-10;
    }
    latDeg = (int)(lat * 180.0 / M_PI);
    lonDeg = (int)(lon * 180.0 / M_PI);

    // zone calculation
    if (posWgs84->longitude < M_PI)
    {
        posUtm->zone = (int)(31 + ((posWgs84->longitude * 180.0 / M_PI) / 6.0));
    }
    else
    {
      posUtm->zone = (int)(((posWgs84->longitude * 180.0 / M_PI) / 6.0) - 29);
    }
    if (posUtm->zone > 60)
    {
        posUtm->zone = 1;
    }

    // utm zone special cases 
    if ((latDeg > 55) && (latDeg < 64) && (lonDeg > -1) && (lonDeg < 3))
    {
        posUtm->zone = 31;
    }
    if ((latDeg > 55) && (latDeg < 64) && (lonDeg > 2) && (lonDeg < 12))
    {
        posUtm->zone = 32;
    }
    if ((latDeg > 71) && (lonDeg > -1) && (lonDeg < 9))
    {
        posUtm->zone = 31;
    }
    if ((latDeg > 71) && (lonDeg > 8) && (lonDeg < 21))
    {
        posUtm->zone = 33;
    }
    if ((latDeg > 71) && (lonDeg > 20) && (lonDeg < 33))
    {
        posUtm->zone = 35;
    }
    if ((latDeg > 71) && (lonDeg > 32) && (lonDeg < 42))
    {
        posUtm->zone = 37;
    }

    if (posUtm->zone >= 31)
    {
        centralMeridian = (6 * posUtm->zone - 183) * M_PI / 180.0;
    }
    else
    {
        centralMeridian = (6 * posUtm->zone + 177) * M_PI / 180.0;
    }

    if (centralMeridian > M_PI)
    {
        centralMeridian -= (2.0*M_PI);
    }

    if (lat < 0)
    {
        falseNorthing = 10000000;
    }


    //
    // convert geodetic position to transverse mercator
    //

    // calc transverse mercator parameter
    tranMercEs  = 2.0 * utmF - utmF * utmF;
    tranMercEbs = (1.0 / (1.0 - tranMercEs)) - 1.0;
    tranMercB   = utmA * (1.0 - utmF);
 
    // true meridianal constants
    tn = (utmA - tranMercB) / (utmA + tranMercB);
    tn2 = tn * tn;
    tn3 = tn2 * tn;
    tn4 = tn3 * tn;
    tn5 = tn4 * tn;

    tranMercAp = utmA * (1.0 - tn + 5.0 * (tn2 - tn3) / 4.0 +
                 81.0 * (tn4 - tn5) / 64.0 );
    tranMercBp = 3.0 * utmA * (tn - tn2 + 7.0 * (tn3 - tn4) / 
                 8.0 + 55.0 * tn5 / 64.0 ) / 2.0;
    tranMercCp = 15.0 * utmA  * (tn2 - tn3 + 3.0 * (tn4 - tn5 ) / 4.0) / 16.0;
    tranMercDp = 35.0 * utmA  * (tn3 - tn4 + 11.0 * tn5 / 16.0) / 48.0;
    tranMercEp = 315.0 * utmA * (tn4 - tn5) / 512.0;

    // delta longitude
    dlam = lon - centralMeridian;

    if (dlam > M_PI)
    {
        dlam -= (2.0 * M_PI);
    }
    if (dlam < -M_PI)
    {
        dlam += (2 * M_PI);
    }
    if (fabs(dlam) < 2.e-10)
    {
        dlam = 0.0;
    }

    dlam2 = dlam * dlam;
    dlam3 = dlam * dlam2;
    dlam4 = dlam2 * dlam2;
    dlam5 = dlam2 * dlam3;
    dlam6 = dlam3 * dlam3;
    dlam7 = dlam3 * dlam4;
    dlam8 = dlam4 * dlam4;
    s     = sin(lat);
    s2    = s * s;
    c     = cos(lat);
    c2    = c * c;
    c3    = c2 * c;
    c5    = c3 * c2;
    c7    = c5 * c2;
    t     = tan(lat);
    tan2  = t * t;
    tan3  = tan2 * t;
    tan4  = tan3 * t;
    tan5  = tan4 * t;
    tan6  = tan5 * t;
    eta   = tranMercEbs * c2;
    eta2  = eta * eta;
    eta3  = eta2 * eta;
    eta4  = eta3 * eta;

    // radius of curvature in prime vertical
    sn   = utmA / sqrt(1.0 - tranMercEs * s2);

    tmd  = tranMercAp * lat - tranMercBp * sin(2.0 * lat) + tranMercCp * sin(4 * lat) -
           tranMercDp * sin(6.0 * lat) + tranMercEp * sin(8.0 * lat);

    tmdo = 0.0;

    // northing
    t1 = (tmd - tmdo) * utmScale;
    t2 = sn * s * c * utmScale / 2.0;
    t3 = sn * s * c3 * utmScale * (5.0 - tan2 + 9.0 * eta + 4.0 * eta2) / 24.0; 
    t4 = sn * s * c5 * utmScale * (61.0 - 58.0 * tan2 + tan4 + 270.0 * eta - 
                                   330.0 * tan2 * eta + 445.0 * eta2 + 324.0 * eta3 - 
                                   680.0 * tan2 * eta2 + 88.0 * eta4 - 600.0 * tan2 * eta3 - 
                                   192.0 * tan2 * eta4) / 720.0;
    t5 = sn * s * c7 * utmScale * (1385.0 - 3111.0 * tan2 + 543.0 * tan4 - tan6) / 40320.0;

    northing = falseNorthing + t1 + dlam2 * t2 + dlam4 * t3 + dlam6 * t4 + dlam8 * t5;

    // easting
    t6 = sn * c * utmScale;
    t7 = sn * c3 * utmScale * ( 1.0 - tan2 + eta ) / 6.0;
    t8 = sn * c5 * utmScale * ( 5.0 - 18.0 * tan2 + tan4 +
                               14.0 * eta - 58.0 * tan2 * eta + 13.0 * eta2 + 4.0 * eta3 -
                               64.0 * tan2 * eta2 - 24.0 * tan2 * eta3 ) / 120.0;
    t9 = sn * c7 * utmScale * (61.0 - 479.0 * tan2 + 179.0 * tan4 - tan6 ) / 5040.0;

    easting  = falseEasting + dlam * t6 + dlam3 * t7 + dlam5 * t8 + dlam7 * t9;

    posUtm->northing = northing * 1000.0;
    posUtm->easting  = easting * 1000.0;
    posUtm->altitude = posWgs84->altitude;
    posUtm->heading  = posWgs84->heading;
}

void PositionTool::utmToWgs84(position_utm_data *posUtm, position_wgs84_data *posWgs84)
{
    double c;
    double de, de2, de3, de4;
    double de5, de6, de7, de8;
    double dlam;
    double eta, eta2, eta3, eta4;
    double ftphi;
    int    i;
    double a;
    double s;
    double sn, sn3, sn5, sn7;
    double sr;
    double t, tan2, tan4, tan6;
    double t10, t11, t12, t13;
    double t14, t15, t16, t17;
    double tmd, tmdo;
    double tn, tn2, tn3, tn4, tn5;
    double tranMercB;
    double tranMercEs;
    double tranMercEbs;
    double tranMercAp, tranMercBp;
    double tranMercCp, tranMercDp, tranMercEp;
    double utmScale2, utmScale3, utmScale4;
    double utmScale5, utmScale6, utmScale7, utmScale8;
    double easting, northing;
    double centralMeridian = 0;
    double falseEasting    = 500000;
    double falseNorthing   = 0;
    static double utmA     = 6378137.0;             // Semi-major axis of ellipsoid in meters
    static double utmF     = 1.0 / 298.257223563;   // flattening of ellipsoid
    double utmScale        = 0.9996;

    if (posUtm->zone >= 31)
    {
        centralMeridian = ((6 * posUtm->zone - 183) * M_PI / 180.0 /*+ 0.00000005*/);
    }
    else
    {
        centralMeridian = ((6 * posUtm->zone + 177) * M_PI / 180.0 /*+ 0.00000005*/);
    }
    if (posUtm->northing < 0)
    {
        falseNorthing = 10000000;
    }

    easting  = posUtm->easting / 1000.0;
    northing = posUtm->northing / 1000.0;

    //
    // transverse mercator projection
    //

    // calc transverse mercator parameter
    tranMercEs  = 2.0 * utmF - utmF * utmF;
    tranMercEbs = (1.0 / (1.0 - tranMercEs)) - 1.0;
    tranMercB   = utmA * (1.0 - utmF);
 
    // true meridianal constants
    tn = (utmA - tranMercB) / (utmA + tranMercB);
    tn2 = tn * tn;
    tn3 = tn2 * tn;
    tn4 = tn3 * tn;
    tn5 = tn4 * tn;

    tranMercAp = utmA * (1.0 - tn + 5.0 * (tn2 - tn3) / 4.0 +
                 81.0 * (tn4 - tn5) / 64.0 );
    tranMercBp = 3.0 * utmA * (tn - tn2 + 7.0 * (tn3 - tn4) / 
                 8.0 + 55.0 * tn5 / 64.0 ) / 2.0;
    tranMercCp = 15.0 * utmA  * (tn2 - tn3 + 3.0 * (tn4 - tn5 ) / 4.0) / 16.0;
    tranMercDp = 35.0 * utmA  * (tn3 - tn4 + 11.0 * tn5 / 16.0) / 48.0;
    tranMercEp = 315.0 * utmA * (tn4 - tn5) / 512.0;

    // true meridional distances for latitude of origin 
    tmdo = 0.0;

    // origin
    tmd = tmdo +  (northing - falseNorthing) / utmScale; 

    // first estimate
    sr    = utmA * (1.0 - tranMercEs);
    ftphi = tmd / sr;

    for (i = 0; i < 5 ; i++)
    {
        t10   = tranMercAp * ftphi - 
                tranMercBp * sin(2.0 * ftphi) + tranMercCp * sin(4.0 * ftphi) - 
                tranMercDp * sin(6.0 * ftphi) - tranMercEp * sin(8.0 * ftphi);
        s     = sin(ftphi);
        a     = sqrt(1.0 - tranMercEs * s * s);
        sr    = utmA * (1.0 - tranMercEs) / (a * a * a);
        ftphi = ftphi + (tmd - t10) / sr;
    }

    // radius of curvature in the meridian
    s   = sin(ftphi);
    a   = sqrt(1.0 - tranMercEs * s * s);
    sr  = utmA * (1.0 - tranMercEs) / (a * a * a);
    sn  = utmA / a;

    c   = cos(ftphi);
    sn3 = sn * sn * sn;
    sn5 = sn3 * sn * sn;
    sn7 = sn5 * sn * sn;

    // tangent value
    t = tan(ftphi);
    tan2 = t * t;
    tan4 = tan2 * tan2;
    tan6 = tan4 * tan2;
    eta  = tranMercEbs * pow(c,2);
    eta2 = eta * eta;
    eta3 = eta2 * eta;
    eta4 = eta3 * eta;
    de   = easting - falseEasting;

    if (fabs(de) < 0.0001)
    {
        de = 0.0;
    }

    de2  = de * de;
    de3  = de2 * de;
    de4  = de2 * de2;
    de5  = de3 * de2;
    de6  = de4 * de2;
    de7  = de4 * de3;
    de8  = de4 * de4;
    utmScale2 = utmScale * utmScale;
    utmScale3 = utmScale2 * utmScale;
    utmScale4 = utmScale2 * utmScale2;
    utmScale5 = utmScale3 * utmScale2;
    utmScale6 = utmScale4 * utmScale2;
    utmScale7 = utmScale4 * utmScale3;
    utmScale8 = utmScale6 * utmScale2;

    // latitude
    t10 = t / (2.0 * sr * sn * utmScale2);
    t11 = t * (5.0  + 3.0 * tan2 + eta - 4.0 * eta2 -
               9.0 * tan2 * eta) / (24.0 * sr * sn3 * utmScale4);
    t12 = t * (61.0 + 90.0 * tan2 + 46.0 * eta + 45.0 * tan4 -
               252.0 * tan2 * eta  - 3.0 * eta2 + 100.0 * eta3 - 
               66.0 * tan2 * eta2 - 90.0 * tan4 * eta + 
               88.0 * eta4 + 225.0 * tan4 * eta2 + 84.0 * tan2 * eta3 - 
               192.0 * tan2 * eta4) / ( 720.0 * sr * sn5 * utmScale6);
    t13 = t * (1385.0 + 3633.0 * tan2 + 4095.0 * tan4 + 1575.0 * tan6) / 
              (40320.0 * sr * sn7 * utmScale8);
    posWgs84->latitude = ftphi - de2 * t10 + de4 * t11 - de6 * t12 + de8 * t13;

    t14 = 1.0 / (sn * c * utmScale);
    t15 = (1.0 + 2.0 * tan2 + eta) / (6.0 * sn3 * c * utmScale3);
    t16 = (5.0 + 6.0 * eta + 28.0 * tan2 - 3.0 * eta2 +
           8.0 * tan2 * eta + 24.0 * tan4 - 4.0 * eta3 +
           4.0 * tan2 * eta2 + 24.0 * tan2 * eta3) / (120.0 * sn5 * c * utmScale5);
    t17 = (61.0 +  662.0 * tan2 + 1320.0 * tan4 + 720.0 * tan6) / 
          (5040.0 * sn7 * c * utmScale7);

    // difference in longitude
    dlam = de * t14 - de3 * t15 + de5 * t16 - de7 * t17;

    // longitude
    posWgs84->longitude = centralMeridian + dlam;

    if (posWgs84->longitude > (M_PI))
    {
        posWgs84->longitude -= (2.0 * M_PI);
    }

    posWgs84->altitude  = posUtm->altitude;
    posWgs84->heading   = posUtm->heading;
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
    posGk->altitude = (int)(h * 1000.0);
    posGk->heading  = posWgs84->heading;
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
    posWgs84->heading   = posGk->heading;
}
