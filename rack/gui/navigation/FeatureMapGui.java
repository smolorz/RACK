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
 *     Daniel Lecking <lecking@rts.uni-hannover.de>
 *
 */
package rack.gui.navigation;

import java.awt.*;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.*;
import rack.main.defines.Position2d;


import rack.navigation.*;


public class FeatureMapGui extends RackModuleGui implements MapViewInterface
{
    protected FeatureMapDataMsg     featureMapData;
    protected FeatureMapProxy       featureMap;
    protected MapViewComponent     	mapComponent;
    protected MapViewActionEvent    mapEvent;

    protected FeatureMapFilenameMsg featureMapFilename = new FeatureMapFilenameMsg();

    protected FeatureMapFeatureMsg 	featureMapFeature 	= new FeatureMapFeatureMsg();

    protected boolean              mapViewIsShowing;
    protected MapViewGui           mapViewGui;

    protected static int 		   FEATURE_MAX = 200;

    protected String 			   addLineCommandLayer0;
    protected String 			   addLineCommandLayer1;
    protected String 			   addLineCommandLayer2;
    protected String 			   saveMapCommand;
    protected String 			   markLineCommand;
    protected String 			   deleteLineCommand;
    protected String 			   displaceLineCommand;

    protected boolean              newFeature 	= true;
    protected boolean              showTmpLine 	= false;
    protected boolean              lineMarked 	= false;

    protected boolean 			   lineMoving[] = new boolean[200]; // 200 Features Max

    protected int				   mindistFeatureNum;

    protected boolean              showCursor 	= false;
    protected Position2d           cursorPosition = new Position2d();


    protected FeatureMapDataPoint 	mapPoint = new FeatureMapDataPoint();

    public FeatureMapGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        featureMap = (FeatureMapProxy) proxy;

        mapComponent = new MapViewComponent();
        mapComponent.addMapView(this);

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));
        JPanel wButtonPanel = new JPanel(new GridLayout(1, 0, 4, 2));
        JPanel leftButtonPanel = new JPanel(new GridLayout(0, 1));
        JPanel westPanel = new JPanel(new BorderLayout());

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

    protected void setEnabled(boolean enabled)
    {
        mapComponent.setEnabled(enabled);
    }

    protected void runStart()
    {
        mapViewGui = MapViewGui.findMapViewGui(ge);

        if(mapViewGui != null)
        {
            mapViewGui.addMapView(this);

            addLineCommandLayer0 = this.getName() + " - add line layer 0 feature";
            addLineCommandLayer1 = this.getName() + " - add line layer 1 feature";
            addLineCommandLayer2 = this.getName() + " - add line layer 2 feature";
            saveMapCommand       = this.getName() + " - save map";
            markLineCommand      = this.getName() + " - mark line";
            deleteLineCommand    = this.getName() + " - delete line";
            displaceLineCommand  = this.getName() + " - displace line";

            mapViewGui.addMapViewAction(addLineCommandLayer0, this);
            mapViewGui.addMapViewAction(addLineCommandLayer1, this);
            mapViewGui.addMapViewAction(addLineCommandLayer2, this);
            mapViewGui.addMapViewAction(saveMapCommand, this);
            mapViewGui.addMapViewAction(markLineCommand, this);
            mapViewGui.addMapViewAction(deleteLineCommand, this);
            mapViewGui.addMapViewAction(displaceLineCommand, this);
        }
    }

    protected void runStop()
    {
        if(mapViewGui != null)
        {
            mapViewGui.removeMapView(this);
            mapViewGui.removeMapViewActions(this);
        }
    }

    protected boolean needsRunData()
    {
        return (super.needsRunData() || mapViewIsShowing);
    }

    protected void runData()
    {
        FeatureMapDataMsg data;

        data = featureMap.getData();

        if (data != null)
        {
            synchronized(this)
            {
                featureMapData = data;
            }
            mapComponent.repaint();

            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
        mapViewIsShowing = false;
    }

    // MapView
    public void mapViewActionPerformed(MapViewActionEvent event)
    {
       String command = event.getActionCommand();

       int eventId = event.getEventId();

        if (command.equals(addLineCommandLayer0) || command.equals(addLineCommandLayer1)
        		|| command.equals(addLineCommandLayer2))
        {
 	       if ((eventId == MapViewActionEvent.MOUSE_MOVED_EVENT ) || (eventId == MapViewActionEvent.MOUSE_WHEEL_EVENT))
	       {
	    	   cursorPosition = event.getWorldCursorPos();
	       }

 	       if (eventId == MapViewActionEvent.MOUSE_CLICKED_EVENT)
 	       {

 	    	   if (newFeature == true)
 	    	   {
 	    		   featureMapFeature.dataPoint.x = event.getWorldCursorPos().x;
 	    		   featureMapFeature.dataPoint.y = event.getWorldCursorPos().y;
 	    		   newFeature = false;
 	    		   showTmpLine = true;
 	    	   }
 	    	   else
 	    	   {
 	    		   featureMapFeature.dataPoint.x2 = event.getWorldCursorPos().x;
 	    		   featureMapFeature.dataPoint.y2 = event.getWorldCursorPos().y;
 	    		   featureMapFeature.dataPoint.l  = Math.sqrt((featureMapFeature.dataPoint.x2 - featureMapFeature.dataPoint.x) *
 	    				   								 	  (featureMapFeature.dataPoint.x2 - featureMapFeature.dataPoint.x) +
 	    				   								 	  (featureMapFeature.dataPoint.y2 - featureMapFeature.dataPoint.y) *
 	    				   								 	  (featureMapFeature.dataPoint.y2 - featureMapFeature.dataPoint.y));
 	    		   featureMapFeature.dataPoint.type = FeatureMapDataPoint.LINE_FEATURE;
 	    		   if (command.equals(addLineCommandLayer0))
 	    		   {
 	    			   featureMapFeature.dataPoint.layer = 0;
 	    		   }
 	    		   if (command.equals(addLineCommandLayer1))
 	    		   {
 	    			   featureMapFeature.dataPoint.layer = 1;
 	    		   }
 	    		   if (command.equals(addLineCommandLayer2))
 	    		   {
 	    			   featureMapFeature.dataPoint.layer = 2;
 	    		   }
 	    		   newFeature = true;
 	    		   showTmpLine = false;
 	    		   featureMap.addLine(featureMapFeature);
 	    	   }
 	       }
        }

        if (command.equals(saveMapCommand))
        {
        	if ((eventId == MapViewActionEvent.MOUSE_MOVED_EVENT ) || (eventId == MapViewActionEvent.MOUSE_WHEEL_EVENT))
        	{
        		cursorPosition = event.getWorldCursorPos();
        	}

        	if (eventId == MapViewActionEvent.MOUSE_CLICKED_EVENT)
        	{
        		featureMapFilename.filename = "mapFile";
        		featureMap.saveMap(featureMapFilename);
        		System.out.println("File saved!");
        	}
        }

        if (command.equals(markLineCommand))
        {
        	if ((eventId == MapViewActionEvent.MOUSE_MOVED_EVENT ) || (eventId == MapViewActionEvent.MOUSE_WHEEL_EVENT))
        	{
        		cursorPosition = event.getWorldCursorPos();
        	}

        	if (eventId == MapViewActionEvent.MOUSE_CLICKED_EVENT)
        	{
        		System.out.println(""+event.getWorldCursorPos().x + "" +event.getWorldCursorPos().y);

        		double olddist = 0;
        		double newdist = 0;
        		double x       = 0;
        		double y       = 0;
        		double x2      = 0;
        		double y2      = 0;
        		double x3      = 0;
        		double y3      = 0;


        		y3 = (double) event.getWorldCursorPos().x;
        		x3 = (double) event.getWorldCursorPos().y;


        		for(int i = 0; i<featureMapData.featureNum; i++)
        		{

        			y  = (double) featureMapData.feature[i].x;
        			x  = (double) featureMapData.feature[i].y;
        			y2 = (double) featureMapData.feature[i].x2;
        			x2 = (double) featureMapData.feature[i].y2;

        			if(checkFootWithinFeatureBounds(x, y, x2, y2, x3, y3))
        			{
        				newdist = calcDistStraightPoint(x, y, x2, y2, x3, y3);
        			}
        			else
        			{
        				newdist = calcDistStraightVertex(x, y, x2, y2, x3, y3);
        			}

        			if(i==0)
        			{
        				olddist = newdist;
        			}

        			if(newdist<=olddist)
        			{
        				olddist = newdist;
        				mindistFeatureNum = i;
        			}
        		}
        		lineMarked = true;
        	}
        }

        if (command.equals(deleteLineCommand))
        {
        	if ((eventId == MapViewActionEvent.MOUSE_MOVED_EVENT ) || (eventId == MapViewActionEvent.MOUSE_WHEEL_EVENT))
        	{
        		cursorPosition = event.getWorldCursorPos();
        	}

        	if (eventId == MapViewActionEvent.MOUSE_CLICKED_EVENT)
        	{
        		if(lineMarked && !lineMoving[mindistFeatureNum])
        		{
        			System.out.println(""+event.getWorldCursorPos().x + "" +event.getWorldCursorPos().y);
        			featureMapFeature.featureNumTemp = mindistFeatureNum;
        			featureMap.deleteLine(featureMapFeature);
        			lineMarked = false;
        		}
        		else if(!lineMarked)
        		{
        			featureMapFeature.featureNumTemp = featureMapData.featureNum+1;
        			featureMap.deleteLine(featureMapFeature);
        		}
        	}
        }

        if (command.equals(displaceLineCommand))
        {
        	if ((eventId == MapViewActionEvent.MOUSE_MOVED_EVENT ) || (eventId == MapViewActionEvent.MOUSE_WHEEL_EVENT))
        	{
        		cursorPosition = event.getWorldCursorPos();
        	}

        	if (eventId == MapViewActionEvent.MOUSE_CLICKED_EVENT)
        	{
        		if(!lineMarked)
        		{
        			featureMapFeature.featureNumTemp = featureMapData.featureNum+1;
        			featureMap.displaceLine(featureMapFeature);
        		}

        		else
        		{
        			double linelength = 0;

        			double x  = 0;
        			double y  = 0;
        			double x2 = 0;
        			double y2 = 0;


        			x  = (double) featureMapData.feature[mindistFeatureNum].x;
        			y  = (double) featureMapData.feature[mindistFeatureNum].y;
        			x2 = (double) featureMapData.feature[mindistFeatureNum].x2;
        			y2 = (double) featureMapData.feature[mindistFeatureNum].y2;

        			linelength = Math.sqrt(Math.pow((x-x2),2)+Math.pow((y-y2),2));

        			if(!lineMoving[mindistFeatureNum] && lineMarked)
        			{
        				lineMoving[mindistFeatureNum] = true;
        				showCursor 	= true;
        			}
        			else if(lineMoving[mindistFeatureNum] && lineMarked)
        			{
        				showCursor 	= false;
        				lineMoving[mindistFeatureNum]= false;
        				lineMarked = false;

        				featureMapFeature.dataPoint.x  = (int) (cursorPosition.x + Math.sin(cursorPosition.rho)*linelength/2);
        				featureMapFeature.dataPoint.y  = (int) (cursorPosition.y - Math.cos(cursorPosition.rho)*linelength/2);
        				featureMapFeature.dataPoint.x2 = (int) (cursorPosition.x - Math.sin(cursorPosition.rho)*linelength/2);
        				featureMapFeature.dataPoint.y2 = (int) (cursorPosition.y + Math.cos(cursorPosition.rho)*linelength/2);
        				featureMapFeature.featureNumTemp = mindistFeatureNum;
        				featureMap.displaceLine(featureMapFeature);
        			}
        		}
        	}
        }
    }

    public Color LayerColor(int layer)
    {
    	Color layercolor;

    	if(layer<=2)
        {
    		Color colorarray[] = {Color.BLUE, Color.GREEN, Color.YELLOW};
    		layercolor = colorarray[layer];
        }
        else
        {
        	int red   = (int) (Math.random() * 255);
        	int blue  = (int) (Math.random() * 255);
        	int green = (int) (Math.random() * 255);
        	layercolor = new Color(red, blue, green);
        }

    	return layercolor;
    }

    public synchronized void paintMapView(MapViewGraphics mvg)
    {

        mapViewIsShowing = true;

		double x  = 0;
		double y  = 0;
		double x2 = 0;
		double y2 = 0;

        if(featureMapData == null)
            return;

		double linelength = 0;

        Graphics2D g = mvg.getWorldGraphics();


        if (showTmpLine == true)
        {
            g.setColor(Color.RED);
            g.setStroke(new BasicStroke(100));
            g.drawLine((int)featureMapFeature.dataPoint.x - 100, (int)featureMapFeature.dataPoint.y, (int)featureMapFeature.dataPoint.x + 100, (int)featureMapFeature.dataPoint.y);
            g.drawLine((int)featureMapFeature.dataPoint.x, (int)featureMapFeature.dataPoint.y - 100, (int)featureMapFeature.dataPoint.x, (int)featureMapFeature.dataPoint.y +100 );
        }

        // draw additional cursor
        if (showCursor == true)
        {
            x  = (double) featureMapData.feature[mindistFeatureNum].x;
            y  = (double) featureMapData.feature[mindistFeatureNum].y;
            x2 = (double) featureMapData.feature[mindistFeatureNum].x2;
            y2 = (double) featureMapData.feature[mindistFeatureNum].y2;

            linelength = Math.sqrt(Math.pow((x-x2),2)+Math.pow((y-y2),2));
            g.setColor(Color.BLACK);
            g.setStroke(new BasicStroke(100));
        	g.drawLine((int)(cursorPosition.x + Math.sin(cursorPosition.rho) * linelength/2),
        			   (int)(cursorPosition.y - Math.cos(cursorPosition.rho) * linelength/2),
        			   (int)(cursorPosition.x - Math.sin(cursorPosition.rho) * linelength/2),
        			   (int)(cursorPosition.y + Math.cos(cursorPosition.rho) * linelength/2));
        }

        // draw complete MAP-MODULE
        for (int i = 0; i < featureMapData.featureNum; i++)
        {
            if ((i != mindistFeatureNum || !lineMarked) && !lineMoving[i])
        	{
            	g.setColor(LayerColor(featureMapData.feature[i].layer));
                g.setStroke(new BasicStroke(100));
                g.drawLine((int)(featureMapData.feature[i].x),(int)(featureMapData.feature[i].y),
                           (int)(featureMapData.feature[i].x2), (int)(featureMapData.feature[i].y2));
        	}
            else if (i == mindistFeatureNum && lineMarked && !lineMoving[i])
            {
            	g.setColor(Color.CYAN);
                g.setStroke(new BasicStroke(100));
                g.drawLine((int)(featureMapData.feature[i].x),(int)(featureMapData.feature[i].y),
                           (int)(featureMapData.feature[i].x2), (int)(featureMapData.feature[i].y2));
            }
         }
    }

    private double calcDistStraightVertex(double x, double y, double x2, double y2, double x3, double y3) {

        double distVertex1 = 0;
        double distVertex2 = 0;
        double distVertex  = 0;

        distVertex1 = Math.sqrt( Math.pow((x-x3),2)+ Math.pow((y-y3),2) );
        distVertex2 = Math.sqrt( Math.pow((x2-x3),2)+ Math.pow((y2-y3),2) );

        if(distVertex1<=distVertex2)
        {
             distVertex = distVertex1;
        }
        else
        {
             distVertex = distVertex2;
        }
        return distVertex;
   }

    private double calcDistStraightPoint(double x, double y, double x2, double y2, double x3, double y3) {

        double cross = 0;
        double distance = 0;
        double linelength = 0;

        cross =( ( (x-x2) * (y3-y) ) - ( (y-y2) * (x3-x) ) );
        linelength = Math.sqrt(Math.pow((x-x2),2)+Math.pow((y-y2),2));
        distance = cross/linelength;
        if(distance<0)
        {
        distance = distance * (-1);
        }
        return distance;
   }

    private boolean checkFootWithinFeatureBounds(double x, double y, double x2, double y2, double x3, double y3) {

        double  multiplier  = 0;
        double  nominator   = 0;
        double  denominator = 0;
        double  x0          = 0;
        double  y0          = 0;
        double  xMin        = 0;
        double  xMax        = 0;
        double  yMin        = 0;
        double 	yMax        = 0;
        boolean onFeature   = false;

        nominator = ( (x3-x) * (x-x2) + (y3-y) * (y-y2) );
        denominator = ( Math.pow((x-x2),2.0) + Math.pow((y-y2),2.0) );
        multiplier = nominator / denominator;

        x0 = x + (multiplier * (x-x2));
        y0 = y + (multiplier * (y-y2));

        if(x<=x2)
        {
            xMin = x;
            xMax = x2;
        }
        else if(x>x2)
        {
            xMax = x;
            xMin = x2;
        }

        if(y<=y2)
        {
            yMin = y;
            yMax = y2;
        }
        else if(y>y2)
        {
            yMax = y;
            yMin = y2;
        }

        if( (x0>=xMin) && (x0<=xMax) && (y0>=yMin) && (y0<=yMax) )
        {
            onFeature = true;
        }
        else
        {
            onFeature = false;
        }

        return onFeature;
   }
}
