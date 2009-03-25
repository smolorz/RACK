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

package rack.navigation;


public class PositionTool
{
	protected static final double utmScale = 0.9996;
    protected static final double utmA     = 6378137.0;             // Semi-major axis of ellipsoid in meters
    protected static final double utmF     = 1.0 / 298.257223563;   // flattening of ellipsoid

	protected static double offsetX = 5804519.0;					// offsets Uni Hannover, parkinglot hard-coded
	protected static double offsetY =  548406.0; 					// offsets Uni Hannover, parkinglot hard-coded

	public static PositionUtmDataMsg wgs84ToUtm(PositionWgs84DataMsg posWgs84)
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
	    
	    PositionUtmDataMsg posUtm = new PositionUtmDataMsg();
	    
	    lat = posWgs84.latitude;
	    lon = posWgs84.longitude;
	    if (lon < 0)
	    {
	        lon += (2.0 * Math.PI) + 1.0e-10;
	    }
	    latDeg = (int)Math.toDegrees(lat);
	    lonDeg = (int)Math.toDegrees(lon);

	    // zone calculation
	    if (posWgs84.longitude < Math.PI)
	    {
	        posUtm.zone = (int)(31 + (Math.toDegrees(posWgs84.longitude) / 6.0));
	    }
	    else
	    {
	      posUtm.zone = (int)((Math.toDegrees(posWgs84.longitude) / 6.0) - 29);
	    }
	    if (posUtm.zone > 60)
	    {
	        posUtm.zone = 1;
	    }

	    // utm zone special cases 
	    if ((latDeg > 55) && (latDeg < 64) && (lonDeg > -1) && (lonDeg < 3))
	    {
	        posUtm.zone = 31;
	    }
	    if ((latDeg > 55) && (latDeg < 64) && (lonDeg > 2) && (lonDeg < 12))
	    {
	        posUtm.zone = 32;
	    }
	    if ((latDeg > 71) && (lonDeg > -1) && (lonDeg < 9))
	    {
	        posUtm.zone = 31;
	    }
	    if ((latDeg > 71) && (lonDeg > 8) && (lonDeg < 21))
	    {
	        posUtm.zone = 33;
	    }
	    if ((latDeg > 71) && (lonDeg > 20) && (lonDeg < 33))
	    {
	        posUtm.zone = 35;
	    }
	    if ((latDeg > 71) && (lonDeg > 32) && (lonDeg < 42))
	    {
	        posUtm.zone = 37;
	    }

	    if (posUtm.zone >= 31)
	    {
	        centralMeridian = Math.toRadians(6 * posUtm.zone - 183);
	    }
	    else
	    {
	        centralMeridian = Math.toRadians(6 * posUtm.zone + 177);
	    }

	    if (centralMeridian > Math.PI)
	    {
	        centralMeridian -= (2.0*Math.PI);
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

	    if (dlam > Math.PI)
	    {
	        dlam -= (2.0 * Math.PI);
	    }
	    if (dlam < -Math.PI)
	    {
	        dlam += (2 * Math.PI);
	    }
	    if (Math.abs(dlam) < 2.e-10)
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
	    s     = Math.sin(lat);
	    s2    = s * s;
	    c     = Math.cos(lat);
	    c2    = c * c;
	    c3    = c2 * c;
	    c5    = c3 * c2;
	    c7    = c5 * c2;
	    t     = Math.tan(lat);
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
	    sn   = utmA / Math.sqrt(1.0 - tranMercEs * s2);

	    tmd  = tranMercAp * lat - tranMercBp * Math.sin(2.0 * lat) + tranMercCp * Math.sin(4 * lat) -
	           tranMercDp * Math.sin(6.0 * lat) + tranMercEp * Math.sin(8.0 * lat);

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

	    posUtm.northing = (northing - offsetX) * 1000.0;
	    posUtm.easting  = (easting - offsetY) * 1000.0;
	    posUtm.altitude = posWgs84.altitude;
	    posUtm.heading  = posWgs84.heading;
	    
	    return posUtm;
	}
}
