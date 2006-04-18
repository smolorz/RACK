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
import java.awt.event.*;

import javax.swing.*;
import javax.swing.event.*;

import java.util.*;
import rack.drivers.*;

public class MapViewGui extends Thread
{

    public final int viewGridDistance = 1000; // mm

    private RackModuleGui[] moduleGui;
    private ArrayList moduleGuiList;

    private boolean driveDirectionX;
    private boolean updateNeeded = false;

    // Anfangswerte fuer das Sichtfenster setzen
    private Position2D viewPosition = new Position2D();
    private double     viewZoom     = 0.05;

    private Position2D robotPosition = new Position2D();

    private OdometryProxy odometryProxy;

    // basic Components
    private ViewPanel    viewPanel;
    private ActionCursor actionCursor;
    private MapNavigator mapNavigator;
    private JMenu        menuBar;
    private JPanel       panel;



    public MapViewGui(RackModuleGui[] n_moduleGui, boolean driveDirectionX)
    {

        this.moduleGui = n_moduleGui;
        this.driveDirectionX = driveDirectionX;

        this.setPriority(Thread.MIN_PRIORITY);

        // create new localisation proxy
        int mapViewLocMbx = RackName.create(RackName.MAP_VIEW, 0, 10);
        try
        {
            TimsMsgRouter.mbxInit(mapViewLocMbx);
        }
        catch (MsgException e)
        {
            e.printStackTrace();
        }

        odometryProxy = new OdometryProxy(0, mapViewLocMbx);

        // create MapView components
        menuBar = new JMenu();
        moduleGuiList = new ModuleGuiList();
        viewPanel = new ViewPanel();
        actionCursor = new ActionCursor();
        mapNavigator = new MapNavigator();

        // set MapView layout
        panel = new JPanel(new BorderLayout(2,2));
        panel.setBorder(BorderFactory.createEmptyBorder(4,4,4,4));
        panel.add(mapNavigator,BorderLayout.NORTH);
        panel.add(viewPanel,BorderLayout.CENTER);;

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
            Position3D position3D = odometryProxy.getData().position;
            robotPosition = new Position2D(position3D.x, position3D.y,
                        position3D.rho);
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
                    drawContext = new DrawContext(
                            actionCursor.worldCursorPosition);
                else
                    drawContext = new DrawContext(robotPosition);

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

    // **********************************************************
    // draw context
    // **********************************************************
    private class DrawContext extends BufferedImage implements
            MapViewDrawContext
    {

        private Graphics2D worldGraph;
        private Graphics2D robotGraph;
        private Position2D robotPosition;

        public DrawContext(Position2D robotPosition)
        {
            super(viewPanel.getWidth(), viewPanel.getHeight(),
                    BufferedImage.TYPE_INT_RGB);

            this.robotPosition = robotPosition;

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

            drawGrid();
        }

        public Graphics2D getWorldGraphics()
        {
            return worldGraph;
        }

        public Graphics2D getRobotGraphics()
        {
            return robotGraph;
        }

        public Position2D getRobotPosition()
        {
            return (Position2D) robotPosition.clone();
        }

        public boolean isDriveDirectionX()
        {
            return driveDirectionX;
        }

        private void drawGrid()
        {
            Rectangle viewBounds = worldGraph.getClipBounds();
            worldGraph.setBackground(Color.WHITE);
            worldGraph.clearRect(viewBounds.x, viewBounds.y, viewBounds.width,
                                 viewBounds.height);
            worldGraph.setColor(Color.LIGHT_GRAY);
            for (int x = (viewGridDistance * (int) (viewBounds.x / viewGridDistance)); x < (viewBounds.x + viewBounds.width); x += viewGridDistance)
            {
                worldGraph.drawLine(x, viewBounds.y, x, viewBounds.y
                        + viewBounds.height);
            }
            for (int y = (viewGridDistance * (int) (viewBounds.y / viewGridDistance)); y < (viewBounds.y + viewBounds.height); y += viewGridDistance)
            {
                worldGraph.drawLine(viewBounds.x, y, viewBounds.x
                        + viewBounds.width, y);
            }
            worldGraph.setColor(Color.ORANGE);
            worldGraph.drawArc(-75, -75, 150, 150, 0, 270);
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
            MouseWheelListener, MouseListener, KeyListener
    {
        private JComboBox   commandBox;
        protected JPanel    eastPanel;
        protected JPanel    centerPanel;
        protected JPanel    westPanel;
        private JButton     viewRobotButton;
        private JButton     viewOriginButton;
        private JLabel      coordinates;


        public MapNavigator()
        {
            this.setLayout(new BorderLayout(2,2));
            this.setBorder(BorderFactory.createEmptyBorder(4,4,4,4));

            viewPanel.addMouseWheelListener(this);
            viewPanel.addMouseListener(this);
            viewPanel.addKeyListener(this);

            // command
            westPanel = new JPanel();
            westPanel.setLayout(new BorderLayout(2,2));
            westPanel.setBorder(BorderFactory.createEmptyBorder(4,4,4,4));

            commandBox = new CommandBox(); //new JComboBox();
            commandBox.setEditable(false);
            commandBox.setPreferredSize(new Dimension(400,
                                        getPreferredSize().height));
            westPanel.add(commandBox, BorderLayout.WEST);

            // coordinates
            centerPanel = new JPanel();
            centerPanel.setLayout(new BorderLayout(2, 2));
            centerPanel.setBorder(BorderFactory.createEmptyBorder(4,4,4,4));

            coordinates = new JLabel("(X: 0 mm , Y: 0 mm)");
            centerPanel.add(coordinates, BorderLayout.CENTER);

            // view
            eastPanel = new JPanel();
            eastPanel.setLayout(new BorderLayout(2, 2));
            eastPanel.setBorder(BorderFactory.createEmptyBorder(4,4,4,4));

            viewOriginButton = new JButton("Origin");
            viewOriginButton.setActionCommand("origin");
            viewOriginButton.addActionListener(this);
            viewOriginButton.setToolTipText("zum Ursprung bewegen");
            viewRobotButton = new JButton("Robot");
            viewRobotButton.setToolTipText("Roboterposition verfolgen");

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
                if (driveDirectionX)
                    changePositionAndZoom(0, 0, -1, 0);
                else
                    changePositionAndZoom(0, 0, 1, 0);
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
                setCenter(new Position2D(0, 0));
                viewRobotButton.setSelected(false);
            }

            updateNeeded();
            viewPanel.grabFocus();
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


        private class CommandBox extends JComboBox implements
                      FocusListener
        {

            public CommandBox()
            {
                setToolTipText("Choose a command");
                this.addFocusListener(this);
            }

            public void focusGained(FocusEvent e)
            {
                this.removeAllItems();

                this.addItem("Move");
                this.addItem("Zoom");

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

                    String module = new String(actionList.title);

                    ListIterator actionListIterator = actionList.listIterator();
                    while (actionListIterator.hasNext())
                    {
                        this.addItem(module + "--->" +
                                     ((MapViewActionList.MapViewActionListItem)
                                       actionListIterator.next()).title);

                    }
                }
            }

            public void focusLost(FocusEvent e)
            {
            }
        }
    }

    // **********************************************************
    // Action Cursor
    // **********************************************************
    private class ActionCursor extends JPanel implements MouseListener,
            MouseMotionListener, ActionListener
    {

        public Position2D worldCursorPosition = new Position2D(0, 0, 0);
        public boolean active = false;

        private ModuleActionEvent actionEvent = null;

        private JToggleButton simRobotPositionButton;
        private JButton cancelButton;
        private JButton executeButton;
        private InfoBox infoBox;

        public ActionCursor()
        {
            super(new GridBagLayout());

            cancelButton = new JButton("Cancel");
            cancelButton.addActionListener(this);
            cancelButton.setToolTipText("Aktion abbrechen");

            executeButton = new JButton("Execute");
            executeButton.setActionCommand("execute");
            executeButton.addActionListener(this);
            executeButton.setToolTipText("Aktion ausfuehren");

            simRobotPositionButton = new JToggleButton("SimRobot", false);
            simRobotPositionButton.setToolTipText("Roboterposition am Cursor simulieren");
            infoBox = new InfoBox();
            infoBox.setToolTipText("dargestellte Information auswaehlen");

            GridBagConstraints gbc = new GridBagConstraints();
            gbc.gridy = 0;
            gbc.gridwidth = 1;
            gbc.gridheight = 1;
            gbc.weighty = 1;

            gbc.gridx = 0;
            gbc.fill = GridBagConstraints.VERTICAL;
            gbc.weightx = 0;
            JMenuBar menu = new JMenuBar();
            menu.add(new ActionMenu());
            this.add(menu, gbc);

            gbc.gridx = 1;
            gbc.fill = GridBagConstraints.NONE;
            gbc.weightx = 0;
            this.add(Box.createHorizontalStrut(10), gbc);
            gbc.gridx = 2;
            gbc.fill = GridBagConstraints.NONE;
            gbc.weightx = 0;
            this.add(infoBox, gbc);

            gbc.gridx = 3;
            gbc.fill = GridBagConstraints.HORIZONTAL;
            gbc.weightx = 1;
            this.add(Box.createHorizontalGlue(), gbc);

            gbc.gridx = 4;
            gbc.fill = GridBagConstraints.NONE;
            gbc.weightx = 0;
            this.add(simRobotPositionButton, gbc);

            gbc.gridx = 5;
            gbc.fill = GridBagConstraints.NONE;
            gbc.weightx = 0;
            this.add(Box.createHorizontalStrut(10), gbc);
            gbc.gridx = 6;
            gbc.fill = GridBagConstraints.NONE;
            gbc.weightx = 0;
            this.add(cancelButton, gbc);
            gbc.gridx = 7;
            gbc.fill = GridBagConstraints.NONE;
            gbc.weightx = 0;
            this.add(executeButton, gbc);

            deactivate();
        }

        public boolean isSimRobotPosition()
        {
            return simRobotPositionButton.isSelected();
        }

        public void actionPerformed(ActionEvent event)
        {
            if (event.getActionCommand().equals("selectAction"))
            {
                actionEvent = (ModuleActionEvent) event.getSource();
                activate();
            }
            if (event.getActionCommand().equals("selectCursor"))
            {
                actionEvent = null;
                activate();
            }
            if (event.getActionCommand().equals("repeat"))
            {
                if (actionEvent != null && !actionEvent.moduleGuiProp.isOn())
                    actionEvent = null;
                activate();
            }
            if (event.getActionCommand().equals("cancel"))
            {
                deactivate();
            }
            if (event.getActionCommand().equals("execute"))
            {
                deactivate();
                if (actionEvent != null && actionEvent.moduleGuiProp.isOn())
                {
                    updateRobotPosition();
                    actionEvent.updatePositions(robotPosition,
                            worldCursorPosition);
                    actionEvent.moduleGuiProp.getModuleGui()
                            .mapViewActionPerformed(actionEvent);
                }
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

        private void activate()
        {
            if (!active)
            {
                viewPanel.addMouseListener(this);
                viewPanel.addMouseMotionListener(this);
                worldCursorPosition = getPosOnScreen((int) (viewPanel
                        .getWidth() * 0.66),
                        (int) (viewPanel.getHeight() * 0.66));
            }

            simRobotPositionButton.setEnabled(true);
            if (actionEvent != null)
                executeButton.setEnabled(true);
            else
                executeButton.setEnabled(false);

            cancelButton.setText("Cancel");
            cancelButton.setActionCommand("cancel");

            infoBox.setEnabled(true);
            infoBox.updateEvent(actionEvent);
            infoBox.updateCoord(worldCursorPosition);

            active = true;
            viewPanel.repaint();
        }

        private void deactivate()
        {
            active = false;

            simRobotPositionButton.setEnabled(false);
            simRobotPositionButton.setSelected(false);
            executeButton.setEnabled(false);

            cancelButton.setText("Repeat");
            cancelButton.setActionCommand("repeat");

            viewPanel.removeMouseListener(this);
            viewPanel.removeMouseMotionListener(this);

            infoBox.setEnabled(false);
            updateNeeded();
        }

        private void drawDefaultCursor(Graphics2D cursorGraphics)
        {
            cursorGraphics.setColor(new Color(0, 0, 255, 180));
            if (driveDirectionX)
            {
                cursorGraphics.fillArc(-200, -200, 400, 400, 30, 300);
            }
            else
            {
                cursorGraphics.fillArc(-200, -200, 400, 400, -60, 300);
            }
        }

        public void mouseDragged(MouseEvent event)
        {
            if ((event.getModifiersEx() & MouseEvent.BUTTON1_DOWN_MASK) > 0)
            {
                Position2D tempPosition = getPosOnScreen(event.getX(), event
                        .getY());
                worldCursorPosition.x = tempPosition.x;
                worldCursorPosition.y = tempPosition.y;
            }
            if ((event.getModifiersEx() & MouseEvent.BUTTON3_DOWN_MASK) > 0)
            {
                Position2D mousePsition = getPosOnScreen(event.getX(), event
                        .getY());
                if (driveDirectionX)
                    worldCursorPosition.phi = normalizePhi((float) Math.atan2(
                            mousePsition.y - worldCursorPosition.y,
                            mousePsition.x - worldCursorPosition.x));
                else
                    worldCursorPosition.phi = normalizePhi((float) Math.atan2(
                            worldCursorPosition.x - mousePsition.x,
                            mousePsition.y - worldCursorPosition.y));
            }

            infoBox.updateCoord(worldCursorPosition);
            if (isSimRobotPosition())
                updateNeeded();
            else
                viewPanel.repaint();
        }

        public Position2D translateRobotCursorPosition(
                Position2D worldCursorPosition, Position2D robotPosition)
        {
            Position2D robotCursorPosition;
            robotCursorPosition = worldCursorPosition.coordTrafo(0,
                    -robotPosition.x, -robotPosition.y);
            robotCursorPosition.phi = worldCursorPosition.phi
                    - robotPosition.phi;
            robotCursorPosition.phi = normalizePhi(robotCursorPosition.phi);
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

        private class InfoBox extends JComboBox
        {

            public TextString actionString = new TextString();
            public TextString worldCoordString = new TextString();
            public TextString robotCoordString = new TextString();

            public InfoBox()
            {
                setEditable(false);
                setPreferredSize(new Dimension(400, getPreferredSize().height));
                addItem(actionString);
                addItem(worldCoordString);
                addItem(robotCoordString);
            }

            public void updateEvent(ModuleActionEvent actionEvent)
            {
                if (actionEvent != null && actionEvent.moduleGuiProp.isOn())
                    actionString.string = actionEvent.moduleGuiProp.toString()
                            + " -> " + actionEvent.getText();
                else
                    actionString.string = "Cursor";
                repaint();
            }

            public void updateCoord(Position2D worldCursorPosition)
            {
                worldCoordString.string = "World -> X=" + worldCursorPosition.x
                        + "mm   Y=" + worldCursorPosition.y + "mm   Phi="
                        + Math.round(Math.toDegrees(worldCursorPosition.phi))
                        + "�";

                Position2D robotCursorPosition = translateRobotCursorPosition(
                        worldCursorPosition, robotPosition);
                robotCoordString.string = "Robot -> X=" + robotCursorPosition.x
                        + "mm   Y=" + robotCursorPosition.y + "mm   Phi="
                        + Math.round(Math.toDegrees(robotCursorPosition.phi))
                        + "�";
                repaint();
            }

            private class TextString
            {
                public String string = "";

                public String toString()
                {
                    return string;
                }
            }
        }
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

        public boolean isDriveDirectionX()
        {
            return driveDirectionX;
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
            cursorGraph.translate(actionCursor.worldCursorPosition.x,
                    actionCursor.worldCursorPosition.y);
            cursorGraph.rotate(actionCursor.worldCursorPosition.phi);

            robotPosition = drawContext.getRobotPosition();
            this.worldCursorPosition = worldCursorPosition;
            this.actionEvent = actionEvent;

            robotCursorPosition = actionCursor.translateRobotCursorPosition(
                    worldCursorPosition, robotPosition);
        }

        public Graphics2D getWorldGraphics()
        {
            return worldGraph;
        }

        public Graphics2D getRobotGraphics()
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

        public Position2D getRobotCursorPos()
        {
            return robotCursorPosition;
        }

        public Position2D getWorldCursorPos()
        {
            return worldCursorPosition;
        }

        public boolean isDriveDirectionX()
        {
            return driveDirectionX;
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