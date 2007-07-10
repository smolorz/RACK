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
package rack.gui.main;

import rack.main.defines.Position2d;
import rack.navigation.PositionDataMsg;

import java.awt.*;
import java.util.Vector;

public class MapViewGraphics
{
    protected Graphics2D frame;
    protected Graphics2D world;
    protected Vector<PositionDataMsg>robotPosition;
    
    public MapViewGraphics(Graphics2D frameGraphics, Graphics2D worldGraphics, Vector<PositionDataMsg>robotPosition)
    {
        this.frame = frameGraphics;
        this.world = worldGraphics;
        this.robotPosition = robotPosition;
    }
    
    public Graphics2D getFrameGraphics()
    {
        return frame;
    }

    public Graphics2D getWorldGraphics()
    {
        return world;
    }

    public Graphics2D getRobotGraphics()
    {
        Graphics2D robot = (Graphics2D) world.create();

        PositionDataMsg position = robotPosition.lastElement();

        robot.translate(position.pos.x, position.pos.y);
        robot.rotate(position.pos.rho);
        
        return robot;
    }

    public Graphics2D getRobotGraphics(int time)
    {
        return getRobotGraphics();
    }

    public Position2d getRobotPosition()
    {
        return new Position2d();
    }

    public Position2d getRobotPosition(int time)
    {
        return new Position2d();
    }
}
