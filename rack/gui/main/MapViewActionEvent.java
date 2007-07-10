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

import rack.main.AngleTool;
import rack.main.defines.Position2d;

public class MapViewActionEvent
{
    protected String command;
    protected Position2d cursorPosition;
    protected Position2d robotPosition;
    
    public MapViewActionEvent(String command, Position2d cursorPosition, Position2d robotPosition)
    {
        this.command = command;
        this.cursorPosition = cursorPosition;
        this.robotPosition = robotPosition;
    }
    
    public String getActionCommand()
    {
        return command;
    }

    public Position2d getWorldCursorPos()
    {
        return cursorPosition;
    }

    public Position2d getRobotCursorPos()
    {
        Position2d robotCursorPos = new Position2d();
        
        double x = (double)(cursorPosition.x - robotPosition.x);
        double y = (double)(cursorPosition.y - robotPosition.y);
        float rho = cursorPosition.rho - robotPosition.rho;

        double cosRho = Math.cos(robotPosition.rho);
        double sinRho = Math.sin(robotPosition.rho);
        
        robotCursorPos.x = (int)(  x * cosRho + y * sinRho);
        robotCursorPos.y = (int)(- x * sinRho + y * cosRho);
        robotCursorPos.rho = AngleTool.normaliseSym0(rho);
        
        return robotCursorPos;
    }

    public Position2d getRobotPosition()
    {
        return robotPosition;
    }
}
