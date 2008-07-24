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
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.gui.perception;

import java.awt.*;
import java.awt.event.*;

import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.*;
import rack.main.defines.*;
import rack.perception.Scan2dDataMsg;
import rack.perception.Scan2dProxy;

public class Scan2dGui extends RackModuleGui implements MapViewInterface {
    
    protected Scan2dDataMsg scan2dData;
    protected Scan2dProxy scan2d;
    protected MapViewComponent mapComponent;

    protected JButton storeContOnButton;
    protected JButton storeContOffButton;
    protected boolean contStoring = false;

    protected ActionListener storeContOnAction;
    protected ActionListener storeContOffAction;

    protected boolean mapViewIsShowing;

    protected boolean displayScanLine;

    public Scan2dGui(GuiElementDescriptor guiElement) {
        super(guiElement);

        displayScanLine=guiElement.getParameter("scanDiplayMode").length() > 0;

        scan2d = (Scan2dProxy) proxy;

        JPanel northPanel = new JPanel(new BorderLayout(2, 2));
        JPanel wButtonPanel = new JPanel(new GridLayout(1, 0, 4, 2));
        JPanel sButtonPanel = new JPanel(new GridLayout(1, 0, 4, 2));

        storeContOnButton = new JButton("StoreOn");
        storeContOffButton = new JButton("StoreOff");

        mapComponent = new MapViewComponent();
        mapComponent.addMapView(this);

        onButton.addKeyListener(mapComponent.keyListener);
        offButton.addKeyListener(mapComponent.keyListener);

        storeContOnAction = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                contStoring = true;
            }
        };
        storeContOnButton.addActionListener(storeContOnAction);
        storeContOnButton.addKeyListener(mapComponent.keyListener);

        storeContOffAction = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                contStoring = false;
            }
        };
        storeContOffButton.addActionListener(storeContOffAction);
        storeContOffButton.addKeyListener(mapComponent.keyListener);

        wButtonPanel.add(onButton);
        wButtonPanel.add(offButton);

        sButtonPanel.add(storeContOnButton, BorderLayout.WEST);
        sButtonPanel.add(storeContOffButton);

        northPanel.add(wButtonPanel, BorderLayout.WEST);
        northPanel.add(mapComponent.zoomPanel, BorderLayout.EAST);
        northPanel.add(sButtonPanel, BorderLayout.SOUTH);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(mapComponent, BorderLayout.CENTER);

        setEnabled(false);
    }

    protected void setEnabled(boolean enabled) {
        mapComponent.setEnabled(enabled);
        if (enabled) {
            if (contStoring) {
                storeContOnButton.setEnabled(false);
                storeContOffButton.setEnabled(true);
            } else {
                storeContOnButton.setEnabled(true);
                storeContOffButton.setEnabled(false);
            }
        } else {
            storeContOnButton.setEnabled(false);
            storeContOffButton.setEnabled(false);
        }
    }

    protected void runStart() {
        MapViewGui mapViewGui = MapViewGui.findMapViewGui(ge);
        if (mapViewGui != null) {
            mapViewGui.addMapView(this);
        }
    }

    protected void runStop() {
        MapViewGui mapViewGui = MapViewGui.findMapViewGui(ge);
        if (mapViewGui != null) {
            mapViewGui.removeMapView(this);
        }

        storeContOnButton.removeActionListener(storeContOnAction);
        storeContOffButton.removeActionListener(storeContOffAction);
        storeContOnAction = null;
        storeContOffAction = null;

        storeContOnButton.removeKeyListener(mapComponent.keyListener);
        storeContOffButton.removeKeyListener(mapComponent.keyListener);

        mapComponent.removeListener();
        mapComponent = null;

        synchronized (this) {
            scan2dData = null;
        }
    }

    protected boolean needsRunData() {
        return (super.needsRunData() || mapViewIsShowing);
    }

    protected void runData() {
        Scan2dDataMsg data;

        data = scan2d.getData();

        if (data != null) {
            synchronized (this) {
                scan2dData = data;
            }
            mapComponent.repaint();

            setEnabled(true);

            if (contStoring) {
                scan2d.storeDataToFile("scan2d-" + System.currentTimeMillis()
                        + ".txt");
            }
        } else {
            setEnabled(false);
        }
        mapViewIsShowing = false;
    }

    public synchronized void paintMapView(MapViewGraphics mvg) {
        mapViewIsShowing = true;

        if (scan2dData == null)
            return;

        Graphics2D g = mvg.getRobotGraphics(scan2dData.recordingTime);

        for (int i = 0; i < scan2dData.pointNum; i++) {
            ScanPoint point = scan2dData.point[i];
            int size = 100;

            if ((point.type & ScanPoint.TYPE_INVALID) != 0) {
                g.setColor(Color.GRAY);
            } else if ((point.type & ScanPoint.TYPE_REFLECTOR) != 0) {
                g.setColor(Color.YELLOW);
            } else if (point.segment == 0) {
                if ((point.type & ScanPoint.TYPE_MASK) == ScanPoint.TYPE_LANDMARK) {
                    g.setColor(Color.BLUE);
                } else if ((point.type & ScanPoint.TYPE_MASK) == ScanPoint.TYPE_OBSTACLE) {
                    g.setColor(Color.RED);
                } else if ((point.type & ScanPoint.TYPE_MASK) == ScanPoint.TYPE_DYN_OBSTACLE) {
                    g.setColor(Color.ORANGE);
                } else {
                    g.setColor(Color.BLACK);
                }
            } else // segment != 0
            {
                int seg = (point.segment - 1) % 5;
                switch (seg) {
                case 0:
                    g.setColor(Color.CYAN);
                    break;
                case 1:
                    g.setColor(Color.GREEN);
                    break;
                case 2:
                    g.setColor(Color.MAGENTA);
                    break;
                case 3:
                    g.setColor(Color.ORANGE);
                    break;
                case 4:
                    g.setColor(Color.PINK);
                    break;
                }
            }

            // draw scanpoints in mm
            if(displayScanLine)
            {
                if (i > 0) {
                    ScanPoint prePoint = scan2dData.point[i - 1];
                    g.drawLine(prePoint.x, prePoint.y, point.x, point.y);
                }
                
            }else{
                int dist = (int) Math.sqrt(point.x * point.x + point.y
                        * point.y);
                size += (int) (dist * 0.025);
            }               

            g.fillArc(point.x - size / 2, point.y - size / 2, size, size, 0,
                    360);
        }
    }

    public void mapViewActionPerformed(MapViewActionEvent event) {
    }
}
