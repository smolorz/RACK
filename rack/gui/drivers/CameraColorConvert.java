/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2007 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Marko Reimer  <reimer@rts.uni-hannover.de>
 *
 */
package rack.gui.drivers;

public class CameraColorConvert {

	public CameraColorConvert() {
		super();
	}
	
	public int calcMono8(int color) {
		color = color & 0xff;
		return 0xFF000000 | (color << 16) | (color << 8) | (color << 0);
	}
	
	public int calcMono12(int color) {
		color = (color >> 4) & 0xff;
		return 0xFF000000 | (color << 16) | (color << 8) | (color << 0);
	}
	
	public int calcMono16(int color) {
		color = (color >> 8) & 0xff;
		return 0xFF000000 | (color << 16) | (color << 8) | (color << 0);
	}
	
	public int calcMono24(int color) {
		color = (color >> 16) & 0xff;
		return 0xff000000 | (color << 16) | (color << 8) | (color << 0);
	}
	
}
