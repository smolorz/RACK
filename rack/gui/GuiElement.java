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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
package rack.gui;

import java.awt.Container;
import java.awt.Cursor;
import java.awt.Image;
import java.awt.Point;
import java.awt.Toolkit;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.MemoryImageSource;

import javax.swing.JComponent;
import javax.swing.JFrame;

/**
 * Diese Klasse ist die Superklasse aller GUI Elemente. Dadurch das die abtrakten Methoden implementiert werden muessen,
 * enthaelt jedes GUI Standardeigenschaften die von dem MainGUI genutzt werden.
 */
public abstract class GuiElement extends Thread
{
    protected GuiElementDescriptor ge;
    protected Gui                  mainGui;

    protected boolean              terminate  = false;

    protected FullScreenFrame      fullScreen = null;
    protected Container            prevContainer;

    protected class FullScreenFrame extends JFrame
    {
        private static final long serialVersionUID = 1L;

        public FullScreenFrame()
        {
            setUndecorated(true);
            setSize(Toolkit.getDefaultToolkit().getScreenSize());
            setAlwaysOnTop(true);
        }

        public void hideCursor()
        {
            Image image = Toolkit.getDefaultToolkit().createImage(new MemoryImageSource(0, 0, null, 0, 16));
            Cursor transparentCursor = Toolkit.getDefaultToolkit().createCustomCursor(image, new Point(0, 0), "");
            setCursor(transparentCursor);
        }
    }

    public GuiElement(GuiElementDescriptor guiElement)
    {
        ge = guiElement;
        mainGui = ge.getMainGui();
        this.setName(ge.getName());
    }

    public abstract JComponent getComponent();

    /**
     * Wird bei Beendigung des "MainGUIs" aufgerufen und soll ein sofortiges Ende der GUI-Komponente bewirken
     */
    public void terminate()
    {
        terminate = true;
        try
        {
            this.interrupt();
            this.join(1000);
        }
        catch (Exception e)
        {
        }
    }

    /**
     * Called when component was switched to/from full-screen mode
     */
    public synchronized void toggleFullScreen()
    {
        if (fullScreen == null)
        {
            fullScreen = new FullScreenFrame();
            prevContainer = getComponent().getParent();
            fullScreen.add(getComponent());
            fullScreen.addWindowListener(new WindowAdapter() {
                public void windowDeactivated(WindowEvent e)
                {
                    if (fullScreen != null)
                    {
                        prevContainer.add(getComponent());
                        fullScreen = null;
                    }
                }
            });
            fullScreen.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            fullScreen.setVisible(true);
        }
        else
        {
            fullScreen.dispose();
            prevContainer.add(getComponent());
            fullScreen = null;
        }
    }
}
