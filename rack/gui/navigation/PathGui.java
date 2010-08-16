/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
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
package rack.gui.navigation;

import java.awt.*;
import java.awt.geom.Arc2D;
import java.awt.geom.GeneralPath;
import java.awt.geom.Line2D;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.*;
import rack.main.defines.*;
import rack.navigation.PathDataMsg;
import rack.navigation.PathDestMsg;
import rack.navigation.PathProxy;
import rack.navigation.PathRddfMsg;

public class PathGui extends RackModuleGui implements MapViewInterface {
    protected PathDataMsg pathData = new PathDataMsg();
    protected PathProxy path;
    protected MapViewComponent mapComponent;

    protected static final int basePointMax = 50;

    protected PathDestMsg pathDest = new PathDestMsg();
    protected PathRddfMsg pathRddf = new PathRddfMsg(basePointMax);
    protected GeneralPath splinePath = null;
    protected int actualPathVersion = -1;

    protected String setDestinationCommand;
    protected String setBasePointForwardCommand;
    protected String setBasePointBackwardCommand;
    protected String clearBasePointListCommand;

    protected boolean mapViewIsShowing;
    protected MapViewGui mapViewGui;

    public PathGui(GuiElementDescriptor guiElement) {
        super(guiElement);

        path = (PathProxy) proxy;

        mapComponent = new MapViewComponent();
        mapComponent.addMapView(this);

        JPanel leftButtonPanel = new JPanel(new GridLayout(0, 1));
        JPanel westPanel = new JPanel(new BorderLayout());
        JPanel northPanel = new JPanel(new BorderLayout(2, 2));
        JPanel wButtonPanel = new JPanel(new GridLayout(1, 0, 4, 2));

        onButton.addKeyListener(mapComponent.keyListener);
        offButton.addKeyListener(mapComponent.keyListener);

        wButtonPanel.add(onButton);
        wButtonPanel.add(offButton);
        northPanel.add(wButtonPanel, BorderLayout.WEST);
        northPanel.add(mapComponent.zoomPanel, BorderLayout.EAST);
        westPanel.add(leftButtonPanel, BorderLayout.NORTH);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(mapComponent, BorderLayout.CENTER);
        rootPanel.add(westPanel, BorderLayout.WEST);

        setEnabled(false);
    }

    protected void setEnabled(boolean enabled) {
        mapComponent.setEnabled(enabled);
    }

    protected void runStart() {
        mapViewGui = MapViewGui.findMapViewGui(ge);
        if (mapViewGui != null) {
            mapViewGui.addMapView(this);

            setDestinationCommand = this.getName() + " - set destination";
            setBasePointForwardCommand = this.getName()
                    + " - set base boint forward";
            setBasePointBackwardCommand = this.getName()
                    + " - set base boint backward";
            clearBasePointListCommand = this.getName()
                    + " - clear base point list";

            mapViewGui.addMapViewAction(setDestinationCommand, this);
            mapViewGui.addMapViewAction(setBasePointForwardCommand, this);
            mapViewGui.addMapViewAction(setBasePointBackwardCommand, this);
            mapViewGui.addMapViewAction(clearBasePointListCommand, this);
        }
    }

    protected void runStop() {
        if (mapViewGui != null) {
            mapViewGui.removeMapView(this);
            mapViewGui.removeMapViewActions(this);
        }
    }

    protected boolean needsRunData() {
        return (super.needsRunData() || mapViewIsShowing);
    }

    protected void runData() {
        PathDataMsg data;

        data = path.getData();

        if ((data != null) && (data.recordingTime != actualPathVersion)) {
            synchronized (this) {
                pathData = data;
                actualPathVersion = data.recordingTime;
                createSplinePath(data);
            }

            mapComponent.repaint();

            setEnabled(true);
        } else {
            setEnabled(false);
        }
        mapViewIsShowing = false;
    }

    public void mapViewActionPerformed(MapViewActionEvent event) {
        String command = event.getActionCommand();

    	if (event.getEventId() == MapViewActionEvent.MOUSE_CLICKED_EVENT)
        {
            if (command.equals(setDestinationCommand))
        	{
	        	pathDest.pos   = event.getWorldCursorPos();
	        	pathDest.layer = 0;

	            // set movement direction
	            if (event.getRobotCursorPos().x >= 0) {
	                if (Math.abs(event.getRobotCursorPos().rho) <= Math
	                        .toRadians(130.0))
	                    pathDest.moveDir = 1.0f;
	                else
	                    pathDest.moveDir = -1.0f;
	            } else {
	                if (Math.abs(event.getRobotCursorPos().rho) <= Math
	                        .toRadians(130.0))
	                    pathDest.moveDir = -1.0f;
	                else
	                    pathDest.moveDir = 1.0f;
	            }

	            // send proxy commands
	            path.setDestination(pathDest);
	            path.make();

	            pathRddf.basePointNum = 0;
	        }
            else if (command.equals(setBasePointForwardCommand))
	        {
	            pathRddf.basePoint[pathRddf.basePointNum] = new Waypoint2d();
	            pathRddf.basePoint[pathRddf.basePointNum].x = event
	                    .getWorldCursorPos().x;
	            pathRddf.basePoint[pathRddf.basePointNum].y = event
	                    .getWorldCursorPos().y;
	            pathRddf.basePoint[pathRddf.basePointNum].speed = 1;

	            if (pathRddf.basePointNum < basePointMax)
	                pathRddf.basePointNum++;

	            // send proxy commands
	            path.setRddf(pathRddf);
	            path.make();
	        }
	        else if (command.equals(setBasePointBackwardCommand))
	        {
	            pathRddf.basePoint[pathRddf.basePointNum] = new Waypoint2d();
	            pathRddf.basePoint[pathRddf.basePointNum].x = event
	                    .getWorldCursorPos().x;
	            pathRddf.basePoint[pathRddf.basePointNum].y = event
	                    .getWorldCursorPos().y;
	            pathRddf.basePoint[pathRddf.basePointNum].speed = -1;

	            if (pathRddf.basePointNum < basePointMax)
	                pathRddf.basePointNum++;

	            // send proxy commands
	            path.setRddf(pathRddf);
	            path.make();
	        }
	        else if (command.equals(clearBasePointListCommand))
	        {
	            pathRddf.basePointNum = 0;

	            // send proxy commands
	            path.setRddf(pathRddf);
	            path.make();
	        }

        }
    }
    public void paintMapView(MapViewGraphics drawContext) {
        int size = 200;
        int dx, dy;
        double openAngle;
        double a;

        mapViewIsShowing = true;
        Graphics2D worldGraph = drawContext.getWorldGraphics();

        synchronized (this) {
            if ((splinePath != null) && (pathData != null)) {
                // draw path
                worldGraph.setColor(Color.BLUE);
                worldGraph.draw(splinePath);

                Point2d prevBasePoint = new Point2d(Integer.MAX_VALUE,
                        Integer.MAX_VALUE);
                int prevRequest = 0;

                if (pathData.splineNum > 0 && pathData.spline[0].request != 0) {
                    prevBasePoint.x = pathData.spline[0].startPos.x;
                    prevBasePoint.y = pathData.spline[0].startPos.y;
                    prevRequest = pathData.spline[0].request;
                }

                // draw basepoints
                for (int i = 0; i < pathData.splineNum - 1; i++) {
                    worldGraph.setColor(Color.BLUE);

                    // curved spline
                    if (pathData.spline[i].radius != 0) {
                        openAngle = Math.PI
                                - (double) pathData.spline[i].length
                                / (double) Math.abs(pathData.spline[i].radius);
                        a = (double) Math.abs(pathData.spline[i].radius)
                                / Math.tan(openAngle / 2);
                        dx = (int) Math.rint(a
                                * Math.cos(pathData.spline[i].startPos.rho));
                        dy = (int) Math.rint(a
                                * Math.sin(pathData.spline[i].startPos.rho));

                        worldGraph.fillArc(pathData.spline[i].startPos.x + dx
                                - size / 2, pathData.spline[i].startPos.y + dy
                                - size / 2, size, size, 0, 360);
                    }

                    // handle direction change and path endpoint
                    if ((pathData.spline[i].length >= 0 & pathData.spline[i + 1].length < 0)
                            | (pathData.spline[i].length < 0 & pathData.spline[i + 1].length >= 0)) {
                        worldGraph.fillArc(pathData.spline[i].endPos.x - size
                                / 2, pathData.spline[i].endPos.y - size / 2,
                                size, size, 0, 360);
                    }

                    StringBuffer sb = new StringBuffer();

                    if (!prevBasePoint.equals(pathData.spline[i].basepoint)) {
                       if (pathData.spline[i].request != 0) {
                            sb.append(String.format("%x",
                                    pathData.spline[i].request));
                        }

                        if (pathData.spline[i].type != 0) {
                            //if (sb.length() == 0)
                            //    sb.append("        ");
                            sb.append(":").append(pathData.spline[i].type);
                        }
                    }


                    if (sb.length() > 0) {

                        worldGraph.setColor(Color.RED);
                        worldGraph.setFont(new Font("Monospaced",
                                Font.PLAIN, 400));

                        worldGraph.rotate(Math.PI / 2,
                                pathData.spline[i].basepoint.x,
                                pathData.spline[i].basepoint.y);

                        worldGraph.drawString(sb.toString(),
                                pathData.spline[i].basepoint.x+100,
                                pathData.spline[i].basepoint.y+400);

                        worldGraph.rotate(-Math.PI / 2,
                                pathData.spline[i].basepoint.x,
                                pathData.spline[i].basepoint.y);
                    }

                    if (prevRequest != 0) {
                        worldGraph.setColor(Color.RED);
                        worldGraph.drawLine(prevBasePoint.x, prevBasePoint.y,
                                pathData.spline[i].basepoint.x,
                                pathData.spline[i].basepoint.y);
                    }

                    prevBasePoint.x = pathData.spline[i].basepoint.x;
                    prevBasePoint.y = pathData.spline[i].basepoint.y;
                    prevRequest     = pathData.spline[i].request;
                }

                // draw path endpoint
                if (pathData.splineNum > 0) {
                    worldGraph.setColor(Color.BLUE);
                    worldGraph.fillArc(
                            pathData.spline[pathData.splineNum - 1].endPos.x
                                    - size / 2,
                            pathData.spline[pathData.splineNum - 1].endPos.y
                                    - size / 2, size, size, 0, 360);
                }
            }
        }
    }

    private void createSplinePath(PathDataMsg splinePathPack) {
        splinePath = new GeneralPath();
        PolarSpline spline;

        for (int i = 0; i < splinePathPack.splineNum; i++) {
            spline = splinePathPack.spline[i];

            if (spline.radius == 0) {
                Line2D.Float line = new Line2D.Float(spline.startPos.x,
                        spline.startPos.y, spline.endPos.x, spline.endPos.y);
                splinePath.append((new BasicStroke(2 * spline.lbo,
                        BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER))
                        .createStrokedShape(line), false);
            } else {
                Arc2D.Float arc = new Arc2D.Float();
                double gamma = (double) spline.length / (double) spline.radius;

                int gammaSign;
                int lengthSign;

                if (gamma >= 0)
                    gammaSign = 1;
                else
                    gammaSign = -1;

                if (spline.length >= 0)
                    lengthSign = 1;
                else
                    lengthSign = -1;

                arc.setArcByCenter(spline.centerPos.x, spline.centerPos.y, Math
                        .abs(spline.radius), Math
                        .toDegrees(-spline.startPos.rho + lengthSign
                                * gammaSign * Math.PI / 2), Math
                        .toDegrees(-gamma), Arc2D.OPEN);
                splinePath.append((new BasicStroke(2 * spline.lbo,
                        BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER))
                        .createStrokedShape(arc), false);
            }
        }
    } // createSplinePath
}
