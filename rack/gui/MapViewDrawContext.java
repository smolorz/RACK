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
package rack.gui;

import rack.main.defines.Position2d;

import java.awt.*;

public interface MapViewDrawContext
{
    public Graphics2D getFrameGraphics();

    public Graphics2D getWorldGraphics();

    public Graphics2D getRobotGraphics();

    public Graphics2D getRobotGraphics(int time);

    public Position2d getRobotPosition();

    public Position2d getRobotPosition(int time);
}
