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
 *      Oliver Wulf      <oliver.wulf@web.de>
 *
 */
package rack.gui.navigation;

import java.awt.*;
import java.awt.event.*;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.*;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.MapViewActionEvent;
import rack.gui.main.MapViewGraphics;
import rack.gui.main.MapViewGui;
import rack.gui.main.MapViewInterface;
import rack.gui.main.NaviComponent;
import rack.gui.main.RackModuleGui;

import rack.main.defines.Position3d;
import rack.navigation.PositionDataMsg;
import rack.navigation.PositionProxy;
import rack.navigation.PositionUtmDataMsg;

public class PositionNavi extends RackModuleGui implements MapViewInterface
{
    protected PositionProxy   position;
    protected Position3d      pos;

    protected NaviComponent   navi;
    protected Position3d[]    destList;
    protected int             destIndex;
    protected int             destTurnover;
    
    protected JLabel          totalDistanceLabel = new JLabel("Total distance 0 m");
    protected JLabel          destinationIndexLabel = new JLabel("Destination ( 0 / 0 )");
    protected JButton         nextDestinationButton = new JButton("Next destination");
    protected ActionListener  nextDestinationAction;
    protected JButton         previousDestinationButton = new JButton("Previous destination");
    protected ActionListener  previousDestinationAction;
    protected JButton         setDestinationButton = new JButton("Set destination");
    protected ActionListener  setDestinationAction;

    protected boolean         mapViewIsShowing;

    public PositionNavi(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        position = (PositionProxy) proxy;

        JPanel buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);

        setDestinationAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                String s = (String) JOptionPane.showInputDialog(null, "Set Destination:\n" + "x, y",
                        "Navi", JOptionPane.PLAIN_MESSAGE, null, null, "0,0");
                if ((s != null) && (s.length() > 0))
                {
                    Position3d dest = parseDestString(s);
                    
                    if(dest != null)
                    {
                        destList = new Position3d[1];
                        destList[0] = dest;
                        destIndex = 0;
                        setDestination();
                    }
                }
            }
        };
        setDestinationButton.addActionListener(setDestinationAction);

        nextDestinationAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                if((destIndex + 1) < destList.length)
                {
                    destIndex++;
                    setDestination();
                }
            }
        };
        nextDestinationButton.addActionListener(nextDestinationAction);

        previousDestinationAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                if(destIndex > 0)
                {
                    destIndex--;
                    setDestination();
                }
            }
        };
        previousDestinationButton.addActionListener(previousDestinationAction);

        JPanel destinationPanel = new JPanel(new GridLayout(0, 1, 4, 2));
        
        destinationPanel.add(totalDistanceLabel);
        destinationPanel.add(destinationIndexLabel);
        destinationPanel.add(nextDestinationButton);
        destinationPanel.add(previousDestinationButton);
        destinationPanel.add(setDestinationButton);

        rootPanel.add(buttonPanel, BorderLayout.NORTH);
        rootPanel.add(destinationPanel, BorderLayout.SOUTH);

        destList = new Position3d[1];
        destList[0] = new Position3d(0, 0, 0, 0, 0, 0);
        destIndex = 0;

        String param = ge.getParameter("destFile");
        if (param.length() > 0)
        {
            loadDestFile(param);
        }
        
        param = ge.getParameter("destTurnover");
        if (param.length() > 0)
        {
            destTurnover = Integer.parseInt(param);
        }
        else
        {
            destTurnover = 2000;
        }

        navi = new NaviComponent(Color.RED, true);
        setDestination();

        rootPanel.add(navi, BorderLayout.CENTER);

        setEnabled(false);
    }

    protected Position3d parseDestString(String destString)
    {
        destString = destString.replaceAll(",", " ");
        StringTokenizer st = new StringTokenizer(destString, " ");
        if (st.countTokens() == 2)
        {
            try
            {
                Position3d dest = new Position3d(
                        Integer.parseInt(st.nextToken().trim()),
                        Integer.parseInt(st.nextToken().trim()), 0, 0.0f, 0.0f, 0.0f);

                return dest;
            }
            catch(NumberFormatException e)
            {
                System.out.println("Can't parse destination \"" + destString + "\"");
                return null;
            }
        }
        else
        {
            System.out.println("Can't parse destination \"" + destString + "\"");
            return null;
        }
    }
    
    protected void loadDestFile(String fileName)
    {
        try
        {
            BufferedReader fileReader = new BufferedReader(new FileReader(fileName));
            
            String line;
            Vector<String> lines = new Vector<String>();
            
            while((line = fileReader.readLine()) != null)
            {
                if((line.startsWith("#") == false) &&
                   (line.length() > 0))
                {
                    lines.add(line);
                }
            }
            
            destList = new Position3d[lines.size()];

            for(int i = 0; i < lines.size(); i++)
            {
                PositionUtmDataMsg utm = new PositionUtmDataMsg();
                System.out.println("parse " + lines.elementAt(i));
                try
                {
                    utm.fromString(lines.elementAt(i));
                    System.out.println("result " + utm.easting + " " + utm.northing + " " + lines.elementAt(i));
                }
                catch(NumberFormatException e)
                {
                    System.out.println(e.toString());
                }
                
                Position3d dest = null;//parseDestString(lines.elementAt(i));
                
                if(dest != null)
                {
                    destList[i] = dest;
                }
                else
                {
                    destList[i] = new Position3d(0,0,0,0,0,0);
                }
            }
        }
        catch (IOException e)
        {
            destList = new Position3d[1];
            destList[0] = new Position3d(0, 0, 0, 0, 0, 0);

            e.printStackTrace();
        }
    }
    
    protected void setDestination()
    {
        navi.setDestination(destList[destIndex]);
        destinationIndexLabel.setText("Destination ( " + (destIndex+1) + " / " + destList.length + " )"); 
    }

    protected void setEnabled(boolean enabled)
    {
        navi.setEnabled(enabled);
        totalDistanceLabel.setEnabled(enabled);
        destinationIndexLabel.setEnabled(enabled);
        nextDestinationButton.setEnabled(enabled);
        previousDestinationButton.setEnabled(enabled);
        setDestinationButton.setEnabled(enabled);
    }
    
    protected void runStart()
    {
        MapViewGui mapViewGui = MapViewGui.findMapViewGui(ge);
        if(mapViewGui != null)
        {
            mapViewGui.addMapView(this);
        }
    }

    protected void runStop()
    {
        MapViewGui mapViewGui = MapViewGui.findMapViewGui(ge);
        if(mapViewGui != null)
        {
            mapViewGui.removeMapView(this);
        }

        setDestinationButton.removeActionListener(setDestinationAction);
    }
    
    protected boolean needsRunData()
    {
        return (super.needsRunData() || mapViewIsShowing);
    }
    
    protected void runData()
    {
        PositionDataMsg data;

        data = position.getData();

        if (data != null)
        {
            synchronized(this)
            {
                pos = data.pos;
            }

            navi.setPosition(data.pos);

            double dx = destList[destIndex].x - data.pos.x;
            double dy = destList[destIndex].y - data.pos.y;
            double dist = Math.sqrt(dx * dx + dy * dy);

            if((destIndex < destList.length - 1) && (dist < destTurnover))
            {
                destIndex++;
                setDestination();
            }
            else
            {
                double totalDist = dist;
                for(int i = destIndex + 1; i < destList.length; i++)
                {
                    dx = destList[i-1].x - destList[i].x;
                    dy = destList[i-1].y - destList[i].y;
                    dist = Math.sqrt(dx * dx + dy * dy);
                    
                    totalDist += dist;
                }
                totalDistanceLabel.setText("Total distance " + (int)(totalDist/1000) + " m");
            }
            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
    }
    
    public synchronized void paintMapView(MapViewGraphics mvg)
    {
        Graphics2D g = mvg.getWorldGraphics();

        synchronized(this)
        {
            if((destList != null) & (destList.length > 0))
            {
                if(pos != null)
                {
                    g.setColor(Color.GRAY);
                    
                    for(int i = 1; i < destIndex + 1; i++)
                    {
                        g.drawLine(destList[i-1].x, destList[i-1].y, destList[i].x, destList[i].y);
                    }

                    g.setColor(Color.RED);

                    g.drawLine(pos.x, pos.y, destList[destIndex].x, destList[destIndex].y);
                    
                    for(int i = destIndex + 1; i < destList.length; i++)
                    {
                        g.drawLine(destList[i-1].x, destList[i-1].y, destList[i].x, destList[i].y);
                    }
                    
                    g.drawArc(destList[destIndex].x - destTurnover, destList[destIndex].y - destTurnover, 2*destTurnover, 2*destTurnover, 0, 360);
                }
                else
                {
                    g.setColor(Color.GRAY);
                    
                    for(int i = 1; i < destList.length; i++)
                    {
                        g.drawLine(destList[i-1].x, destList[i-1].y, destList[i].x, destList[i].y);
                    }
                }
            }
        }
    }

    public void mapViewActionPerformed(MapViewActionEvent event)
    {
    }
}
