/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2009	   University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Matthias Hentschel	<hentschel@rts.uni-hannover.de>
 *
 */

package rack.gui.tools;


public class BackgroundImageDescriptor
{
    String                 descFileName = "";
    String                 imgFileName = "";
    int					   centerX;				// x-coordinate of the image center in mm
    int					   centerY;				// y-coordinate of the image center in mm
    int					   sizeX;				// x-size of the image in mm
    int					   sizeY;     	        // y-size of the image in mm
    int					   width;				// width of the image in px
    int					   height;				// height of the image px
    double				   scaleX;				// x-scale of the image in mm/px
    double				   scaleY;				// y-scale of the image in mm/px
    double				   upperLeftX;			// x-coodinate of the upperleft corner
    double				   upperLeftY;			// y-coordinate of the upperleft corner
}