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

import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;
import java.io.File;
import java.util.*;

import javax.swing.*;
import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;
import javax.imageio.*;

import rack.main.RackName;
import rack.main.defines.*;
import rack.drivers.*;
import rack.gui.GuiElement;
import rack.gui.GuiElementDescriptor;
import rack.navigation.*;

public class MapViewGui extends GuiElement implements MapViewInterface
{
    protected JPanel                       rootPanel;
    protected JPanel                       northPanel;
    protected MapViewComponent             mapComponent;
    
    protected JComboBox                    actionMenu;
    protected Vector<String>               actionCommands = new Vector<String>();
    protected Vector<MapViewInterface>     actionTargets = new Vector<MapViewInterface>();
    protected MapViewMouseListener         mouseListener = new MapViewMouseListener();
    protected ActionMenuListener           actionMenuListener = new ActionMenuListener();

    protected String                       positionUpdateCommand;
    
    protected BufferedImage                bgImg;
    protected double                       bgResX, bgResY;  // in mm/pixel
    protected Position2d                   bgOffset = new Position2d();

    protected PositionProxy                positionProxy;
    protected Vector<PositionDataMsg>      robotPosition;
    protected ChassisProxy                 chassisProxy;
    protected ChassisParamMsg              chassisParam;

    protected boolean                      terminate = false;

    public MapViewGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        // create MapView proxies
        positionProxy = (PositionProxy) mainGui.getProxy(RackName.POSITION, 0);
        chassisProxy = (ChassisProxy) mainGui.getProxy(RackName.CHASSIS, 0);

        // get chassis parameter message
        if (chassisProxy != null)
        {
            chassisParam = chassisProxy.getParam();
        }
        if (chassisParam == null)
        {
            chassisParam = new ChassisParamMsg();
            chassisParam.boundaryFront = 400;
            chassisParam.boundaryBack = 400;
            chassisParam.boundaryLeft = 400;
            chassisParam.boundaryRight = 400;
        }

        // create MapView components
        mapComponent = new MapViewComponent();
        mapComponent.addMouseListener(mouseListener);
        mapComponent.setPreferredSize(new Dimension(600,400));
        mapComponent.showCursor = true;

        actionMenu = new JComboBox();
        actionMenu.addKeyListener(mapComponent.keyListener);
        actionMenu.addPopupMenuListener(actionMenuListener);

        // set MapView layout
        rootPanel = new JPanel(new BorderLayout(2, 2));
        rootPanel.setBorder(BorderFactory.createEmptyBorder(2, 2, 2, 2));

        northPanel = new JPanel(new BorderLayout(2, 2));
        northPanel.add(actionMenu, BorderLayout.CENTER);
        northPanel.add(mapComponent.zoomPanel, BorderLayout.EAST);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(mapComponent, BorderLayout.CENTER);

        // set MapView background
        String param = ge.getParameter("bg");
        if (param.length() > 0)
        {
            try
            {
                File file = new File(param);
                bgImg = ImageIO.read(file);
            }
            catch (Exception e)
            {
                System.out.println("MapViewGui: Can't read background image \"" + param + "\"\n");
            }

            double ulx;
            param = ge.getParameter("ulx");
            if (param.length() > 0)
                ulx = (double)Integer.parseInt(param);
            else
                ulx = 0;

            double uly;
            param = ge.getParameter("uly");
            if (param.length() > 0)
                uly = (double)Integer.parseInt(param);
            else
                uly = 0;

            double llx;
            param = ge.getParameter("llx");
            if (param.length() > 0)
                llx = (double)Integer.parseInt(param);
            else
                llx = 0;

/*            double lly;
            param = ge.getParameter("lly");
            if (param.length() > 0)
                lly = (double)Integer.parseInt(param);
            else
                lly = 0;
*/

/*            double urx;
            param = ge.getParameter("urx");
            if (param.length() > 0)
                urx = (double)Integer.parseInt(param);
            else
                urx = 0;
*/
            double ury;
            param = ge.getParameter("ury");
            if (param.length() > 0)
                ury = (double)Integer.parseInt(param);
            else
                ury = 0;

            bgOffset.x = (int)ulx;
            bgOffset.y = (int)uly;
            bgOffset.rho = 0.0f;

            if (llx != 0)
                bgResX = (ulx - llx) / (double)bgImg.getHeight();
            else
                bgResX = 1;

            if (ury != 0)
                bgResY = (ury - uly) / (double)bgImg.getHeight();
            else
                bgResY = 1;
            
            System.out.println("offset x " + bgOffset.x + " y " + bgOffset.y + " rho " + bgOffset.rho);
            System.out.println("resolution x " + bgResX + " y " + bgResY);
            
            mapComponent.zoomCenterButton.grabFocus();
        }
    }

    public void addMapView(MapViewInterface mapView)
    {
        mapComponent.addMapView(mapView);
    }
    
    public void removeMapView(MapViewInterface mapView)
    {
        mapComponent.removeMapView(mapView);
    }
    
    public synchronized void addMapViewAction(String command, MapViewInterface mapView)
    {
        northPanel.remove(actionMenu);
        
        actionCommands.add(command);
        actionTargets.add(mapView);
        
        actionMenu = new JComboBox(actionCommands);
        actionMenu.addPopupMenuListener(actionMenuListener);
        
        northPanel.add(actionMenu, BorderLayout.CENTER);
    }
    
    public synchronized void removeMapViewActions(MapViewInterface mapView)
    {
        int index;

        northPanel.remove(actionMenu);
        
        while((index = actionTargets.indexOf(mapView)) >= 0)
        {
            actionCommands.removeElementAt(index);
            actionTargets.removeElementAt(index);
        }
        
        actionMenu = new JComboBox(actionCommands);
        actionMenu.addPopupMenuListener(actionMenuListener);
        
        northPanel.add(actionMenu, BorderLayout.CENTER);
    }
    
    public static MapViewGui findMapViewGui(GuiElementDescriptor ge)
    {
        Vector<GuiElementDescriptor> guiElements;
        guiElements = ge.getMainGui().getGuiElements();

        MapViewGui mapViewGui;
        
        for(int i = 0; i < guiElements.size(); i++)
        {
            try
            {
                mapViewGui = (MapViewGui) guiElements.get(i).getGui();
                return mapViewGui;
            }
            catch(ClassCastException e)
            {
            }
        }
        return null;
    }

    public JComponent getComponent()
    {
        return rootPanel;
    }

    public void start()
    {
        robotPosition = mapComponent.getRobotPositionVector();  // update robot position
        
        addMapView(this);  // paint current robot position 
        
        addMapViewAction("Choose action command", this);
        positionUpdateCommand = "Position - update";
        addMapViewAction(positionUpdateCommand, this);

        super.start();
    }
    
    public void run()
    {
        while (terminate == false)
        {
            if(rootPanel.isShowing())
            {
                PositionDataMsg position = positionProxy.getData(); 

                if(position != null)
                {
                    robotPosition.add(position);
                    
                    while(robotPosition.size() > 50)
                    {
                        robotPosition.removeElementAt(0);
                    }
                }
                
                mapComponent.repaint();
            }
            
            try
            {
                sleep(500);
            }
            catch (InterruptedException e)
            {
            }
        }
        mapComponent.removeMapView(this);

        System.out.println("MapViewGui terminated");
    }

    public synchronized void paintMapView(MapViewGraphics mvg)
    {
        Graphics2D rg = mvg.getRobotGraphics();
        paintRobot(rg);
    }

    public void mapViewActionPerformed(MapViewActionEvent event)
    {
        String command = event.getActionCommand();
        
        if(command.equals(positionUpdateCommand))
        {
            System.out.println("Position update " + event.getWorldCursorPos());
            
            if(positionProxy != null)
            {
                Position3d position = new Position3d(event.getWorldCursorPos());

                positionProxy.update(position, 0);
            }
        }
        //else if(command.matches("Choose action command"))
        //{
        //    System.out.println("action command " + event.getWorldCursorPos() + " " + event.getRobotCursorPos() + " " + event.getRobotPosition());
        //}
    }

    protected void paintRobot(Graphics2D g)
    {
        int chassisWidth = chassisParam.boundaryLeft + chassisParam.boundaryRight;
        int chassisLength = chassisParam.boundaryBack + chassisParam.boundaryFront;

        g.setColor(Color.LIGHT_GRAY);
        g.fillRect(-chassisParam.boundaryBack - chassisParam.safetyMargin,
                   -chassisParam.boundaryLeft - chassisParam.safetyMargin,
                   chassisLength + 2 * chassisParam.safetyMargin + chassisParam.safetyMarginMove,
                   chassisWidth + 2 * chassisParam.safetyMargin);

        g.setColor(Color.GRAY);
        g.fillRect(-chassisParam.boundaryBack,
                   -chassisParam.boundaryLeft,
                   chassisLength, chassisWidth);

        g.setColor(Color.BLACK);
        g.drawRect(-chassisParam.boundaryBack,
                   -chassisParam.boundaryLeft,
                   chassisLength, chassisWidth);
        g.drawLine(0,
                   -chassisParam.boundaryLeft,
                   chassisParam.boundaryFront,
                   0);
        
        g.drawLine(chassisParam.boundaryFront,
                   0,
                   0,
                   chassisParam.boundaryRight);
    }

    protected class MapViewMouseListener extends MouseAdapter
    {
        public void mouseClicked(MouseEvent e)
        {
            if(e.getClickCount() == 2)
            {
                String command;
                MapViewInterface mapView;

                synchronized(this)
                {
                    int index = actionMenu.getSelectedIndex();
                    
                    command = actionCommands.elementAt(index);
                    mapView = actionTargets.elementAt(index);
                }

                Position2d cursorPosition2d = mapComponent.getCursorPosition();

                PositionDataMsg robot = robotPosition.lastElement();
                Position2d robotPosition2d;
                robotPosition2d = new Position2d(robot.pos);

                MapViewActionEvent actionEvent = new MapViewActionEvent(command, cursorPosition2d, robotPosition2d);

                mapView.mapViewActionPerformed(actionEvent);
            }
        }
    }
    
    protected class ActionMenuListener implements PopupMenuListener
    {
        public void popupMenuCanceled(PopupMenuEvent arg0)
        {
        }

        public void popupMenuWillBecomeInvisible(PopupMenuEvent arg0)
        {
            System.out.println("invisible");

            if(actionMenu.getSelectedIndex() > 0)
            {
                mapComponent.showCursor = true;
            }
            else
            {
                mapComponent.showCursor = false;
            }

            mapComponent.zoomCenterButton.grabFocus();
        }

        public void popupMenuWillBecomeVisible(PopupMenuEvent arg0)
        {
        }
    };
}
