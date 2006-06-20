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

import rack.main.gui.*;
import rack.main.defines.*;
import rack.main.naming.*;
import rack.main.tims.router.*;
import rack.main.tims.exceptions.*;

import java.awt.*;
import java.awt.image.*;
import java.awt.geom.AffineTransform;
import java.awt.event.*;
import java.io.File;

import javax.swing.*;
import javax.swing.event.*;
import javax.imageio.*;

import java.util.*;
import rack.drivers.*;
import rack.navigation.*;

public class MapViewGui extends Thread
{
    private RackModuleGui[] moduleGui;
    private ArrayList       moduleGuiList;

    private boolean         updateNeeded = false;
    private Position2D      viewPosition = new Position2D();
    private double          viewZoom     = 0.02;
    public final int        viewGridDistance = 10000; // in mm

    private Position2D      robotPosition = new Position2D();
    private PositionDataMsg[] robotPositionList;
    private int             robotPositionListIndex = 0;
    private Position2D      worldCursorPosition = new Position2D(0, 0, 0);

    // basic Components
    private ViewPanel       viewPanel;
    private ActionCursor    actionCursor;
    private MapNavigator    mapNavigator;
    private JMenu           menuBar;
    private JPanel          panel;
    private BufferedImage   backGndImg;
    private double          backGndResX,        // in mm/pixel
                            backGndResY;        // in mm/pixel
    private Position2D      backGndOffset = new Position2D();

    // proxies
    private PositionProxy   positionProxy;
    private ChassisProxy    chassisProxy;

    // Messages
    ChassisParamMsg         chassisParam;


    public MapViewGui(RackModuleGui[] n_moduleGui)
    {
        this.moduleGui = n_moduleGui;
        this.setPriority(Thread.MIN_PRIORITY);

        // create work Mailbox
        int workMbx = RackName.create(RackName.MAP_VIEW, 0, 10);
        try
        {
            TimsMsgRouter.mbxInit(workMbx);
        }
        catch (MsgException e)
        {
            e.printStackTrace();
        }

        // create MapView proxies
        positionProxy = new PositionProxy(0, workMbx);
        chassisProxy  = new ChassisProxy(0, workMbx);

        // get chassis parameter message
        if (chassisProxy != null)
            chassisParam = chassisProxy.getParam();

        // create MapView components
        menuBar = new JMenu();
        moduleGuiList = new ModuleGuiList();
        viewPanel = new ViewPanel();
        actionCursor = new ActionCursor();
        mapNavigator = new MapNavigator();

        // set MapView layout
        panel = new JPanel(new BorderLayout(2,2));
        panel.setBorder(BorderFactory.createEmptyBorder(2,2,2,2));
        panel.add(mapNavigator,BorderLayout.NORTH);
        panel.add(viewPanel,BorderLayout.CENTER);;

        // set MapView background
        File file = new File("mapViewBackground.jpg");        
/*        backGndOffset.x   = 106432;
        backGndOffset.y   = 204500;
//        backGndOffset.phi = (float)(-18.0 / 180.0 * Math.PI);
        backGndOffset.phi = 0.0f;
        backGndResX       = 220.0;
        backGndResY       = 230.0;*/
        backGndOffset.x   = 362800;
        backGndOffset.y   = 96500;
        backGndOffset.phi = (float)(-0.1/ 180.0 * Math.PI);
//        backGndOffset.phi = 0.0f;
        backGndResX       = 250.0;
        backGndResY       = 250.0;

        // read background image
        try
        {
            backGndImg = ImageIO.read(file);
        }
        catch (Exception exc)
        {
            System.out.println("No background image found!\n"+exc);
        }

        robotPositionList = new PositionDataMsg[25];
        for(int i = 0; i < 25; i++)
        {
            robotPositionList[i] = new PositionDataMsg();
        }

        this.start();
    }


    public JComponent getComponent()
    {
        return panel;
    }


    private void changePositionAndZoom(int changeX, int changeY, int changePhi,
            int changeZoom)
    {
        Position2D newCenter = getPosOnScreen(viewPanel.getWidth() / 2
                + changeX * viewPanel.getWidth() / 5, viewPanel.getHeight() / 2
                + changeY * viewPanel.getHeight() / 5);
        viewPosition.phi += changePhi * Math.PI / 10;
        if (viewPosition.phi > Math.PI)
            viewPosition.phi -= (2 * Math.PI);
        if (viewPosition.phi < -Math.PI)
            viewPosition.phi += (2 * Math.PI);
        if (changeZoom > 0)
            viewZoom = viewZoom * 1.5;
        if (changeZoom < 0)
            viewZoom = viewZoom / 1.5;
        setCenter(newCenter);
    }


    private void setCenter(Position2D newCenter)
    {
        Position2D screenCenter;

        // sets center on screen in world coordinates
        screenCenter = (new Position2D((int)Math.round(-viewPanel.getHeight() /
                                                       (2 * viewZoom)),
                                       (int) Math.round(viewPanel.getWidth() /
                                                       (2 * viewZoom)))).
                                                  coordTrafo(viewPosition.phi);

        viewPosition.x = newCenter.x - screenCenter.x;
        viewPosition.y = newCenter.y - screenCenter.y;
    }

    private Position2D getPosOnScreen(int x, int y)
    {
        Position2D posOnScreen;

        // returns position in wold coordinates
        posOnScreen = new Position2D((int) Math.round(-y / viewZoom),
                                     (int) Math.round(x / viewZoom));
        return posOnScreen.coordTrafo(viewPosition.phi, viewPosition.x,
                                      viewPosition.y);
    }

    private void updateRobotPosition()
    {
        try
        {
        	PositionDataMsg position = new PositionDataMsg();
        	
        	if (positionProxy != null)
        	    position = positionProxy.getData();

            robotPosition = new Position2D(position.pos.x, position.pos.y,
                                           position.pos.rho);

            synchronized(robotPositionList)
            {
                robotPositionListIndex++;
                if(robotPositionListIndex >= 25)
                    robotPositionListIndex = 0;
                robotPositionList[robotPositionListIndex] = position;
            }
        }
        catch (Exception exc)
        {
            robotPosition = new Position2D();
        }

        if (mapNavigator.viewRobot())
        {
            viewPosition.phi = robotPosition.phi;
            setCenter(robotPosition);
        }

        // center mapView
        else
        {
        	Position2D upLeftBound     = getPosOnScreen(viewPanel.getWidth() / 4,
            										    viewPanel.getHeight() / 4);
            Position2D lowRightBound   = getPosOnScreen(viewPanel.getWidth() - 
            										    viewPanel.getWidth() / 4,
            										    viewPanel.getHeight() -
            										    viewPanel.getHeight() / 4);
            if ((robotPosition.x > upLeftBound.x) |
               (robotPosition.y < upLeftBound.y) |
               (robotPosition.x < lowRightBound.x) |
               (robotPosition.y > lowRightBound.y))
            {
            	setCenter(robotPosition);
            }
        }

    }

    public void updateNeeded()
    {
        updateNeeded = true;
        wakeup();
    }


    public void run()
    {
        Thread.yield();
        Thread.yield();
        Thread.yield();
        setCenter(new Position2D());

        while (!Gui.terminate)
        {
            do
            {
                updateNeeded = false;
                if (!viewPanel.isShowing())
                    break;

                updateRobotPosition();

                // create a new draw context
                DrawContext drawContext;
                if (actionCursor.isSimRobotPosition())
                {
                    drawContext = new DrawContext(worldCursorPosition, null);
                }
                else
                {
                    drawContext = new DrawContext(robotPosition, robotPositionList);
                }

                actionCursor.drawDefaultCursor(drawContext.getRobotGraphics());

                ListIterator moduleGuiIterator = moduleGuiList.listIterator();
                while (moduleGuiIterator.hasNext())
                {
                    ModuleGuiProp moduleGuiProp = (ModuleGuiProp) moduleGuiIterator
                            .next();
                    if (!moduleGuiProp.getPaintIntoMap())
                        continue;
                    try
                    {
                        moduleGuiProp.getModuleGui().paintMapView(drawContext);
                    }
                    catch (Exception exc)
                    {
                        exc.printStackTrace();
                    }
                } // while
              viewPanel.setDrawContext(drawContext);
            }
            while (updateNeeded == true);
            delay(200);
        }
    } // run()

    private synchronized void delay(long time)
    {
        try
        {
            this.wait(time);
        }
        catch (InterruptedException e)
        {
        }
    }

    private synchronized void wakeup()
    {
        this.notifyAll();
    }

    // *********************************************************
    // draw context
    // **********************************************************
    private class DrawContext extends BufferedImage implements
            MapViewDrawContext
    {

        private Graphics2D worldGraph;
        private Graphics2D robotGraph;
        private Position2D robotPosition;
        private PositionDataMsg robotPositionList[];

        public DrawContext(Position2D robotPosition, PositionDataMsg[] robotPositionList)
        {
            super(viewPanel.getWidth(), viewPanel.getHeight(),
                    BufferedImage.TYPE_INT_RGB);

            this.robotPosition = robotPosition;
            this.robotPositionList = robotPositionList;

            worldGraph = this.createGraphics();
            worldGraph.setClip(0, 0, this.getWidth(), this.getHeight());

            // prepare worldGraph
            worldGraph.scale(viewZoom, viewZoom);
            worldGraph.rotate(-viewPosition.phi - Math.PI / 2);
            worldGraph.translate(-viewPosition.x, -viewPosition.y);

            // prepare robotGraph
            robotGraph = this.createGraphics();
            robotGraph.setClip(0, 0, this.getWidth(), this.getHeight());
            robotGraph.setTransform(worldGraph.getTransform());
            robotGraph.translate(robotPosition.x, robotPosition.y);
            robotGraph.rotate(robotPosition.phi);

            drawBackgndImg(backGndImg, backGndResX, backGndResY,
                           backGndOffset);
            drawGrid();
        }

        public Graphics2D getFrameGraphics()
        {
            return (Graphics2D)this.getGraphics();
        }

        public Graphics2D getWorldGraphics()
        {
            return worldGraph;
        }

        public Graphics2D getRobotGraphics()
        {
            return robotGraph;
        }

        public Graphics2D getRobotGraphics(int time)
        {
            Position2D robotPositionTime = getRobotPosition(time);

            // prepare robotGraph
            Graphics2D robotGraphTime = this.createGraphics();

            robotGraphTime.setClip(0, 0, this.getWidth(), this.getHeight());
            robotGraphTime.setTransform(worldGraph.getTransform());
            robotGraphTime.translate(robotPositionTime.x, robotPositionTime.y);
            robotGraphTime.rotate(robotPositionTime.phi);

            return robotGraphTime;
        }

        public Position2D getRobotPosition()
        {
            return (Position2D) robotPosition.clone();
        }

        public Position2D getRobotPosition(int time)
        {
            if(robotPositionList != null)
            {
                Position2D robotPosition;
                
                synchronized(robotPositionList)
                {
                    int index = robotPositionListIndex;
                    int indexTimeDiff = Math.abs(time - robotPositionList[index].recordingTime);
                    
                    for(int i = 0; i < robotPositionList.length; i++)
                    {
                        int timeDiff = Math.abs(time - robotPositionList[i].recordingTime);
                        if(timeDiff < indexTimeDiff)
                        {
                            index = i;
                            indexTimeDiff = timeDiff;
                        }
                    }
                    robotPosition = new Position2D(robotPositionList[index].pos.x,
                                                   robotPositionList[index].pos.y,
                                                   robotPositionList[index].pos.rho);
                }
                return robotPosition;
            }
            else
                return (Position2D) robotPosition.clone();
        }

        private void drawGrid()
        {
            Rectangle viewBounds = worldGraph.getClipBounds();
            worldGraph.setColor(Color.LIGHT_GRAY);

            for (int x = (viewGridDistance * (int) (viewBounds.x / viewGridDistance));
                 x < (viewBounds.x + viewBounds.width); x += viewGridDistance)
            {
                worldGraph.drawLine(x, viewBounds.y, x, viewBounds.y +
                                                        viewBounds.height);
            }

            for (int y = (viewGridDistance * (int) (viewBounds.y / viewGridDistance));
                 y < (viewBounds.y + viewBounds.height); y += viewGridDistance)
            {
                worldGraph.drawLine(viewBounds.x, y, viewBounds.x +
                                                     viewBounds.width, y);
            }
            worldGraph.setColor(Color.ORANGE);
            worldGraph.drawArc(-75, -75, 150, 150, 0, 270);
        }


        private void drawBackgndImg(BufferedImage image,
                                       double resX, double resY, Position2D pos)
        {
            Rectangle viewBounds = worldGraph.getClipBounds();
            worldGraph.setBackground(Color.WHITE);
            worldGraph.clearRect(viewBounds.x, viewBounds.y, viewBounds.width,
                                 viewBounds.height);
            if (image != null)
            {
                AffineTransform at = new AffineTransform();
                at.scale(resX, resY);
                at.rotate(pos.phi + Math.PI/2);
                BufferedImageOp biop = new AffineTransformOp(at, AffineTransformOp.TYPE_NEAREST_NEIGHBOR);
                worldGraph.drawImage(image, biop, pos.x, pos.y);
            }
        }
    }

    // **********************************************************
    // view panel
    // **********************************************************
    private class ViewPanel extends JPanel implements ComponentListener
    {
        private DrawContext drawContext = null;

        public ViewPanel()
        {
            this.setDoubleBuffered(false);
            this.addComponentListener(this);
        }

        public synchronized void setDrawContext(DrawContext newDrawContext)
        {
            drawContext = newDrawContext;
            this.repaint();
        }

        public synchronized void paint(Graphics onGraph)
        {
            if (drawContext == null)
                return;
            onGraph.drawImage(drawContext, 0, 0, this);
            actionCursor.drawCursor((Graphics2D) onGraph, drawContext);
        }

        public void componentResized(ComponentEvent evnt)
        {
            updateNeeded();
            grabFocus();
        }

        public void componentHidden(ComponentEvent evnt)
        {
        }

        public void componentMoved(ComponentEvent evnt)
        {
        }

        public void componentShown(ComponentEvent evnt)
        {
            updateNeeded();
            grabFocus();
        }
    }

    // **********************************************************
    // Map Navigator
    // **********************************************************
    private class MapNavigator extends JPanel implements ActionListener,
            MouseWheelListener, MouseListener, MouseMotionListener, KeyListener
    {
        protected JPanel    eastPanel;
        protected JPanel    centerPanel;
        protected JPanel    westPanel;
        private JButton     viewRobotButton;
        private JButton     viewOriginButton;
        private JLabel      coordinateLabel;
        private ModuleActionEvent actionEvent = null;

        public MapNavigator()
        {
            this.setLayout(new BorderLayout(0,0));
//            this.setBorder(BorderFactory.createEmptyBorder(2,2,2,2));

         //   viewPanel.addMouseWheelListener(this);
            viewPanel.addMouseListener(this);
            viewPanel.addKeyListener(this);
            viewPanel.addMouseMotionListener(this);

           // command
            westPanel = new JPanel();
            westPanel.setLayout(new BorderLayout(2,2));
            westPanel.setBorder(BorderFactory.createEmptyBorder(2,2,2,2));

            JMenuBar menu = new JMenuBar();
            menu.add(new CommandMenu());
            westPanel.add(menu, BorderLayout.WEST);

            // coordinates
            centerPanel = new JPanel();
            centerPanel.setLayout(new BorderLayout(2, 2));
            centerPanel.setBorder(BorderFactory.createEmptyBorder(2,2,2,2));

            coordinateLabel = new JLabel("X: 0 mm , Y: 0 mm");
            centerPanel.add(coordinateLabel, BorderLayout.CENTER);

            // view
            eastPanel = new JPanel();
            eastPanel.setLayout(new BorderLayout(2, 2));
            eastPanel.setBorder(BorderFactory.createEmptyBorder(2,2,2,2));

            viewOriginButton = new JButton("Origin");
            viewOriginButton.setActionCommand("origin");
            viewOriginButton.addActionListener(this);
            viewOriginButton.setToolTipText("global view");
            viewRobotButton = new JButton("Robot");
            viewRobotButton.setActionCommand("robot");
            viewRobotButton.addActionListener(this);
            viewRobotButton.setToolTipText("robot centered view");

            eastPanel.add(viewOriginButton, BorderLayout.WEST);
            eastPanel.add(viewRobotButton, BorderLayout.EAST);

            add(westPanel, BorderLayout.WEST);
            add(centerPanel, BorderLayout.CENTER);
            add(eastPanel, BorderLayout.EAST);
        }

        public void actionPerformed(ActionEvent event)
        {
            if (event.getActionCommand().equals("north"))
            {
                changePositionAndZoom(0, -1, 0, 0);
                viewRobotButton.setSelected(false);
            }
            if (event.getActionCommand().equals("south"))
            {
                changePositionAndZoom(0, 1, 0, 0);
                viewRobotButton.setSelected(false);
            }
            if (event.getActionCommand().equals("west"))
            {
                changePositionAndZoom(-1, 0, 0, 0);
                viewRobotButton.setSelected(false);
            }
            if (event.getActionCommand().equals("east"))
            {
                changePositionAndZoom(1, 0, 0, 0);
                viewRobotButton.setSelected(false);
            }
            if (event.getActionCommand().equals("in"))
            {
                changePositionAndZoom(0, 0, 0, 1);
            }
            if (event.getActionCommand().equals("out"))
            {
                changePositionAndZoom(0, 0, 0, -1);
            }
            if (event.getActionCommand().equals("left"))
            {
                changePositionAndZoom(0, 0, -1, 0);
                viewRobotButton.setSelected(false);
            }
            if (event.getActionCommand().equals("right"))
            {
                changePositionAndZoom(0, 0, 1, 0);
                viewRobotButton.setSelected(false);
            }
            if (event.getActionCommand().equals("origin"))
            {
                viewPosition.phi = 0;
                setCenter(robotPosition);
                viewRobotButton.setSelected(false);
            }
            if (event.getActionCommand().equals("robot"))
            {
                viewRobotButton.setSelected(true);
            }

            updateNeeded();
            viewPanel.grabFocus();
        }

        public void mouseMoved(MouseEvent event)
        {
            Position2D tempPosition = getPosOnScreen(event.getX(),
                                                     event.getY());
            worldCursorPosition.x   = tempPosition.x;
            worldCursorPosition.y   = tempPosition.y;
            coordinateLabel.setText("X: "+worldCursorPosition.x+" mm, " +
                                    "Y: "+worldCursorPosition.y+" mm");
        }

        public void mouseDragged(MouseEvent event)
        {
        }

        public void mouseClicked(MouseEvent event)
        {
            if (event.getButton() == MouseEvent.BUTTON2)
            {
                setCenter(getPosOnScreen(event.getX(), event.getY()));
                viewRobotButton.setSelected(false);
                updateNeeded();
            }
        }

        public void mousePressed(MouseEvent event)
        {
        }

        public void mouseReleased(MouseEvent event)
        {
        }

        public void mouseEntered(MouseEvent e)
        {
            viewPanel.grabFocus();
        }

        public void mouseExited(MouseEvent e)
        {
        }

        public void mouseWheelMoved(MouseWheelEvent event)
        {
            changePositionAndZoom(0, 0, 0, -event.getWheelRotation());
            updateNeeded();
        }

        public void keyPressed(KeyEvent event)
        {
            switch (event.getKeyCode())
            {
                case KeyEvent.VK_RIGHT:
                    actionPerformed(new ActionEvent(this, 0, "east"));
                    break;
                case KeyEvent.VK_LEFT:
                    actionPerformed(new ActionEvent(this, 0, "west"));
                    break;
                case KeyEvent.VK_UP:
                    actionPerformed(new ActionEvent(this, 0, "north"));
                    break;
                case KeyEvent.VK_DOWN:
                    actionPerformed(new ActionEvent(this, 0, "south"));
                    break;

                case KeyEvent.VK_PLUS:
                    actionPerformed(new ActionEvent(this, 0, "in"));
                    break;
                case KeyEvent.VK_MINUS:
                    actionPerformed(new ActionEvent(this, 0, "out"));
                    break;

                case KeyEvent.VK_PAGE_UP:
                    actionPerformed(new ActionEvent(this, 0, "left"));
                    break;
                case KeyEvent.VK_PAGE_DOWN:
                    actionPerformed(new ActionEvent(this, 0, "right"));
                    break;
            }
        }

        public void keyReleased(KeyEvent e)
        {
        }

        public void keyTyped(KeyEvent e)
        {
        }

        public boolean viewRobot()
        {
            return viewRobotButton.isSelected();
        }
        

        private class CommandMenu extends JMenu implements MenuListener
        {

            public CommandMenu()
            {
                super("   Command   ");
                addMenuListener(this);
                setToolTipText("Choose a command");
            }

            public void menuCanceled(MenuEvent arg0)
            {
            }

            public void menuDeselected(MenuEvent arg0)
            {
            }

            public void menuSelected(MenuEvent arg0)
            {
                this.removeAll();

                ListIterator moduleGuiIterator = moduleGuiList.listIterator();
                while (moduleGuiIterator.hasNext())
                {
                    ModuleGuiProp moduleGuiProp = (ModuleGuiProp)moduleGuiIterator
                            .next();
                    if (!moduleGuiProp.isOn())
                        continue;

                    MapViewActionList actionList = moduleGuiProp.getModuleGui()
                            .getMapViewActionList();
                    if (actionList == null)
                        continue;

                    JMenu newSubmenu = new JMenu(actionList.title);
                    this.add(newSubmenu);

                    ListIterator actionListIterator = actionList.listIterator();
                    while (actionListIterator.hasNext())
                    {
                        newSubmenu.add(new ModuleActionEvent(moduleGuiProp,
                                      (MapViewActionList.
                                       MapViewActionListItem)actionListIterator.
                                          next()));
                    }
                }

                if (this.getComponentCount() > 0)
                    this.addSeparator();

                JMenuItem newMenuItem = new JMenuItem("Cursor");
                newMenuItem.setActionCommand("selectCursor");
                newMenuItem.addActionListener(actionCursor);
                this.add(newMenuItem);

                this.validate();
            }
        } // commandMenu
    }







    // **********************************************************
    // Action Cursor
    // **********************************************************
    private class ActionCursor extends JPanel implements MouseListener,
            MouseMotionListener, MouseWheelListener, ActionListener
    {
        public boolean active = false;
        private ModuleActionEvent actionEvent = null;
        private float dPhiPerClick = (float)Math.toRadians(10.0);

        public ActionCursor()
        {
            viewPanel.addMouseListener(this);
            viewPanel.addMouseMotionListener(this);
            viewPanel.addMouseWheelListener(this);
        }

        public boolean isSimRobotPosition()
        {
            return false;
//            return simRobotPositionButton.isSelected();
        }

        public void actionPerformed(ActionEvent event)
        {
            if (event.getActionCommand().equals("selectAction"))
            {
                actionEvent = (ModuleActionEvent) event.getSource();
                active = true;
            }
            if (event.getActionCommand().equals("repeat"))
            {
                if (actionEvent != null && !actionEvent.moduleGuiProp.isOn())
                    actionEvent = null;
                active = true;
            }
            if (event.getActionCommand().equals("cancel"))
            {
                actionEvent = null;
                active = false;
            }
            if (event.getActionCommand().equals("execute"))
            {
                if (actionEvent != null && actionEvent.moduleGuiProp.isOn())
                {
                    updateRobotPosition();
                    actionEvent.updatePositions(robotPosition,
                            worldCursorPosition);
                    actionEvent.moduleGuiProp.getModuleGui()
                            .mapViewActionPerformed(actionEvent);
                }
                active = true;
            }
        }

        public synchronized void drawCursor(Graphics2D screenGraph,
                DrawContext drawContext)
        {

            if (active)
            {
                CursorDrawContext cursorDrawContext = new CursorDrawContext(
                        screenGraph, drawContext, actionEvent,
                        worldCursorPosition);
                if (actionEvent != null
                        && actionEvent.moduleGuiProp.isOn()
                        && actionEvent.moduleGuiProp.getModuleGui()
                                .hasMapViewCursor())
                {
                    actionEvent.moduleGuiProp.getModuleGui()
                            .paintMapViewCursor(cursorDrawContext);
                }
                else
                {
                    drawDefaultCursor(cursorDrawContext.getCursorGraphics());
                }
            }
        }

        private void drawDefaultCursor(Graphics2D cursorGraphics)
        {
            int chassisWidth = chassisParam.boundaryLeft +
               chassisParam.boundaryRight;
            int chassisLength = chassisParam.boundaryBack +
                chassisParam.boundaryFront;

            cursorGraphics.setColor(Color.GRAY);
            cursorGraphics.fillRect(-chassisParam.boundaryBack -
                                     chassisParam.safetyMargin,
                                    -chassisParam.boundaryLeft -
                                     chassisParam.safetyMargin,
                                     chassisLength + 2 * chassisParam.safetyMargin +
                                     chassisParam.safetyMarginMove,
                                     chassisWidth + 2 * chassisParam.safetyMargin);

            cursorGraphics.setColor(Color.DARK_GRAY);
            cursorGraphics.fillRect(-chassisParam.boundaryBack,
                                    -chassisParam.boundaryLeft,
                                     chassisLength, chassisWidth);

            cursorGraphics.setColor(Color.BLACK);
            cursorGraphics.drawRect(-chassisParam.boundaryBack,
                                    -chassisParam.boundaryLeft,
                                     chassisLength, chassisWidth);
            cursorGraphics.drawLine(-chassisParam.boundaryBack +
                                     (int)(chassisLength * 0.5),
                                       -chassisParam.boundaryLeft,
                                        chassisParam.boundaryFront,
                                       -chassisParam.boundaryLeft +
                                        (int)(chassisWidth * 0.5));
            cursorGraphics.drawLine( chassisParam.boundaryFront,
                                    -chassisParam.boundaryLeft +
                                     (int)(chassisWidth * 0.5),
                                    -chassisParam.boundaryBack +
                                     (int)(chassisLength * 0.5),
                                     chassisParam.boundaryRight);
        }

        public Position2D translateRobotCursorPosition(
                Position2D worldCursorPosition, Position2D robotPosition)
        {
            Position2D robotCursorPosition = new Position2D();
            double sinRho = Math.sin(robotPosition.phi);
            double cosRho = Math.cos(robotPosition.phi);
            double x      = (double)(worldCursorPosition.x - 
                                      robotPosition.x);
            double y      = (double)(worldCursorPosition.y - 
                                      robotPosition.y);            

            robotCursorPosition.x   = (int)(  x * cosRho + y * sinRho);
            robotCursorPosition.y   = (int)(- x * sinRho + y * cosRho);
            robotCursorPosition.phi = normalizePhi(worldCursorPosition.phi - 
                                      robotPosition.phi);
            return robotCursorPosition;
        }

        private float normalizePhi(float phi)
        {
            if (phi > Math.PI)
                phi -= 2 * Math.PI;
            if (phi < -Math.PI)
                phi += 2 * Math.PI;
            return phi;
        }


        public void mouseDragged(MouseEvent event)
        {
            if ((event.getModifiersEx() & MouseEvent.BUTTON1_DOWN_MASK) > 0)
            {
                Position2D tempPosition = getPosOnScreen(event.getX(), event
                        .getY());
                worldCursorPosition.x = tempPosition.x;
                worldCursorPosition.y = tempPosition.y;
                actionPerformed(new ActionEvent(this, 0, "execute"));
            }
/*            if ((event.getModifiersEx() & MouseEvent.BUTTON3_DOWN_MASK) > 0)
            {
                Position2D mousePsition = getPosOnScreen(event.getX(), event
                        .getY());
                worldCursorPosition.phi = normalizePhi((float) Math.atan2(
                            mousePsition.y - worldCursorPosition.y,
                            mousePsition.x - worldCursorPosition.x));
            }*/

            if (isSimRobotPosition())
                updateNeeded();
            else
                viewPanel.repaint();
        }

        public void mouseWheelMoved(MouseWheelEvent event)
        {
            worldCursorPosition.phi = normalizePhi(
                                      worldCursorPosition.phi +
                                      event.getWheelRotation() * dPhiPerClick);
            updateNeeded();
        }

        public void mouseClicked(MouseEvent event)
        {
        }

        public void mouseEntered(MouseEvent arg0)
        {
        }

        public void mouseExited(MouseEvent arg0)
        {
        }

        public void mousePressed(MouseEvent event)
        {
            mouseDragged(event);
        }

        public void mouseReleased(MouseEvent arg0)
        {
        }

        public void mouseMoved(MouseEvent arg0)
        {
        }


        private class ActionMenu extends JMenu implements MenuListener
        {

            public ActionMenu()
            {
                super("   Action   ");
                addMenuListener(this);
                setToolTipText("Aktion auswaehlen");
            }

            public void menuCanceled(MenuEvent arg0)
            {
            }

            public void menuDeselected(MenuEvent arg0)
            {
            }

            public void menuSelected(MenuEvent arg0)
            {
                this.removeAll();

                ListIterator moduleGuiIterator = moduleGuiList.listIterator();
                while (moduleGuiIterator.hasNext())
                {
                    ModuleGuiProp moduleGuiProp = (ModuleGuiProp) moduleGuiIterator
                            .next();
                    if (!moduleGuiProp.isOn())
                        continue;

                    MapViewActionList actionList = moduleGuiProp.getModuleGui()
                            .getMapViewActionList();
                    if (actionList == null)
                        continue;

                    JMenu newSubmenu = new JMenu(actionList.title);
                    this.add(newSubmenu);

                    ListIterator actionListIterator = actionList.listIterator();
                    while (actionListIterator.hasNext())
                    {
                        newSubmenu
                                .add(new ModuleActionEvent(
                                        moduleGuiProp,
                                        (MapViewActionList.MapViewActionListItem) actionListIterator
                                                .next()));
                    }
                }

                if (this.getComponentCount() > 0)
                    this.addSeparator();
                JMenuItem newMenuItem = new JMenuItem("Cursor");
                newMenuItem.setActionCommand("selectCursor");
                newMenuItem.addActionListener(actionCursor);
                this.add(newMenuItem);

                this.validate();
            }
        } // actionMenu


    }

    // **********************************************************
    // ModuleActionEvent
    // **********************************************************
    private class ModuleActionEvent extends JMenuItem implements
            MapViewActionEvent
    {

        public ModuleGuiProp moduleGuiProp = null;
        public String actionCommand = "";

        public Position2D robotPosition;
        public Position2D worldCursorPosition;
        public Position2D robotCursorPosition;

        public ModuleActionEvent(ModuleGuiProp n_moduleGuiProp,
                MapViewActionList.MapViewActionListItem listItem)
        {
            super(listItem.title);
            setActionCommand("selectAction");
            addActionListener(actionCursor);

            moduleGuiProp = n_moduleGuiProp;
            actionCommand = listItem.actionCommand;
        }

        public void updatePositions(Position2D robotPosition,
                Position2D worldCursorPosition)
        {
            this.robotPosition = robotPosition;
            this.worldCursorPosition = worldCursorPosition;
            robotCursorPosition = actionCursor.translateRobotCursorPosition(
                    worldCursorPosition, robotPosition);
        }

        public String getActionCommand()
        {
            return actionCommand;
        }

        public Position2D getWorldCursorPos()
        {
            return (Position2D) worldCursorPosition;
        }

        public Position2D getRobotCursorPos()
        {
            return (Position2D) robotCursorPosition;
        }

        public Position2D getRobotPosition()
        {
            return (Position2D) robotPosition;
        }
    }





    // **********************************************************
    // cursor draw context
    // **********************************************************
    private class CursorDrawContext implements MapViewCursorDrawContext
    {

        private Graphics2D worldGraph;
        private Graphics2D robotGraph;
        private Graphics2D cursorGraph;

        private Position2D robotPosition;
        private Position2D worldCursorPosition;
        private Position2D robotCursorPosition;

        private ModuleActionEvent actionEvent;

        public CursorDrawContext(Graphics2D screenGraph,
                DrawContext drawContext, ModuleActionEvent actionEvent,
                Position2D worldCursorPosition)
        {

            // prepare worldGraph
            worldGraph = (Graphics2D) screenGraph.create();
            worldGraph.setClip(0, 0, drawContext.getWidth(), drawContext
                    .getHeight());
            worldGraph.setTransform(drawContext.getWorldGraphics()
                    .getTransform());

            // prepare robotGraph
            robotGraph = (Graphics2D) screenGraph.create();
            robotGraph.setClip(0, 0, drawContext.getWidth(), drawContext
                    .getHeight());
            robotGraph.setTransform(drawContext.getRobotGraphics()
                    .getTransform());

            // prepare cursorGraph
            cursorGraph = (Graphics2D) screenGraph.create();
            cursorGraph.setClip(0, 0, drawContext.getWidth(), drawContext
                    .getHeight());
            cursorGraph.setTransform(worldGraph.getTransform());
            cursorGraph.translate(worldCursorPosition.x,
                                  worldCursorPosition.y);
            cursorGraph.rotate(worldCursorPosition.phi);

            robotPosition = drawContext.getRobotPosition();
            this.worldCursorPosition = worldCursorPosition;
            this.actionEvent = actionEvent;

            robotCursorPosition = actionCursor.translateRobotCursorPosition(
                    worldCursorPosition, robotPosition);
        }

        public Graphics2D getFrameGraphics()
        {
            return null;
        }

        public Graphics2D getWorldGraphics()
        {
            return worldGraph;
        }

        public Graphics2D getRobotGraphics()
        {
            return robotGraph;
        }

        public Graphics2D getRobotGraphics(int time)
        {
            return robotGraph;
        }

        public Graphics2D getCursorGraphics()
        {
            return cursorGraph;
        }

        public Position2D getRobotPosition()
        {
            return robotPosition;
        }

        public Position2D getRobotPosition(int time)
        {
            return robotPosition;
        }

        public Position2D getRobotCursorPos()
        {
            return robotCursorPosition;
        }

        public Position2D getWorldCursorPos()
        {
            return worldCursorPosition;
        }

        public String getActionCommand()
        {
            if (actionEvent != null)
                return actionEvent.getActionCommand();
            else
                return "";
        }
    }

    // **********************************************************
    // ModuleGuiProp
    // **********************************************************
    private class ModuleGuiProp extends JMenu implements ActionListener
    {

        private JMenuItem paintItem;
        private int moduleGuiIndex;

        public ModuleGuiProp(int n_moduleGuiIndex)
        {
            super();

            moduleGuiIndex = n_moduleGuiIndex;

            paintItem = new JCheckBoxMenuItem("Paint", true);
            paintItem.setActionCommand("paint");
            paintItem.addActionListener(this);
            add(paintItem);
        }

        public RackModuleGui getModuleGui()
        {
            return moduleGui[moduleGuiIndex];
        }

        public boolean isOn()
        {
            return (getModuleGui() != null) && getModuleGui().hasMapView();
        }

        public boolean getPaintIntoMap()
        {
            return (getModuleGui() != null) && paintItem.isSelected();
        }

        public String toString()
        {
            if (isOn())
                return getModuleGui().getModuleName();
            else
                return "";
        }

        public void actionPerformed(ActionEvent event)
        {
            updateNeeded();

        }

    } // ModuleGuiProp


    // **********************************************************
    // ModuleGuiList
    // **********************************************************
    private class ModuleGuiList extends ArrayList
    {

        private GuiListMenu guiListMenu;

        public ModuleGuiList()
        {
            super(moduleGui.length);

            for (int g = 0; g < moduleGui.length; g++)
            {
                this.add(new ModuleGuiProp(g));
            }

            guiListMenu = new GuiListMenu();

            menuBar.add(guiListMenu);
        }

        private class GuiListMenu extends JMenu implements MenuListener
        {

            public GuiListMenu()
            {
                super("Options");
                addMenuListener(this);
                setToolTipText("Change MapView options");
            }

            public void menuCanceled(MenuEvent arg0)
            {
            }

            public void menuDeselected(MenuEvent arg0)
            {
            }

            public void menuSelected(MenuEvent arg0)
            {
                this.removeAll();

                ListIterator moduleGuiIterator = moduleGuiList.listIterator();
                while (moduleGuiIterator.hasNext())
                {
                    ModuleGuiProp moduleGuiProp = (ModuleGuiProp) moduleGuiIterator
                            .next();
                    if (!moduleGuiProp.isOn())
                        continue;

                    moduleGuiProp.setText(moduleGuiProp.toString());
                    add(moduleGuiProp);
                }
                this.validate();
            }
        }
    } // ModuleGuiList

}