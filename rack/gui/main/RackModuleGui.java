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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
package rack.gui.main;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JPanel;

import rack.gui.GuiElement;
import rack.gui.GuiElementDescriptor;
import rack.main.RackProxy;

public abstract class RackModuleGui extends GuiElement
{
    public static int   DEFAULT_UPDATE_TIME = 500;

    protected RackProxy proxy;
    protected int       updateTime;

    protected JPanel    rootPanel;
    protected JButton   onButton;
    protected JButton   offButton;
    protected ActionListener    onButtonAction;
    protected ActionListener    offButtonAction;

    public RackModuleGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);
        
        proxy = ge.getProxy();

        updateTime = DEFAULT_UPDATE_TIME;
        
        rootPanel = new JPanel(new BorderLayout(2, 2));
        rootPanel.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));

        onButton = new JButton("On");
        onButtonAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                proxy.on();
            }
        };
        onButton.addActionListener(onButtonAction);

        offButton = new JButton("Off");
        offButtonAction = new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                proxy.off();
            }
        };
        offButton.addActionListener(offButtonAction);
    }

    public JComponent getComponent()
    {
        return rootPanel;
    }

    protected void runStart()
    {
    }

    protected boolean needsRunData()
    {
        return rootPanel.isShowing();
    }
    
    protected void runData()
    {
    }

    protected void runStop()
    {
    }
    
    public void start()
    {
        terminate = false;
        runStart();
        super.start();
    }
    
    public void run()
    {
        while (terminate == false)
        {
            if (needsRunData())
            {
                runData();
            }
            try
            {
                Thread.sleep(updateTime);
            }
            catch (InterruptedException e)
            {
            }
        }
        runStop();

        onButton.removeActionListener(onButtonAction);
        onButtonAction = null;
        offButton.removeActionListener(offButtonAction);
        offButtonAction = null;

        rootPanel.removeAll();
    }
}
