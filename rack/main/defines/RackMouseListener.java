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
package rack.main.defines;

import java.awt.event.*;
import javax.swing.JLabel;

public class RackMouseListener implements MouseListener, MouseMotionListener
{

    JLabel outputLabel;
    int mx, my; // the mouse coordinates
    boolean isButtonPressed = false;

    public RackMouseListener(JLabel positionLabel)
    {
        outputLabel = positionLabel;
    }

    public void mouseEntered(MouseEvent e)
    {
        // called when the pointer enters the applet's rectangular area
    }

    public void mouseExited(MouseEvent e)
    {
        // called when the pointer leaves the applet's rectangular area
    }

    public void mouseClicked(MouseEvent e)
    {
        // called after a press and release of a mouse button
        // with no motion in between
        // (If the user presses, drags, and then releases, there will be
        // no click event generated.)
    }

    public void mousePressed(MouseEvent e)
    { // called after a button is pressed down
        isButtonPressed = true;
        // "Consume" the event so it won't be processed in the
        // default manner by the source which generated it.
        e.consume();
    }

    public void mouseReleased(MouseEvent e)
    { // called after a button is released
        isButtonPressed = false;
        e.consume();
    }

    public void mouseMoved(MouseEvent e)
    { // called during motion when no buttons are down
        mx = e.getX();
        my = e.getY();
        outputLabel.setText("Mouse at (" + mx + "," + my + ")");
        // showStatus( "Mouse at (" + mx + "," + my + ")" );
        e.consume();
    }

    public void mouseDragged(MouseEvent e)
    { // called during motion with buttons down
        mx = e.getX();
        my = e.getY();
        outputLabel.setText("Mouse at (" + mx + "," + my + ")");
        // showStatus( "Mouse at (" + mx + "," + my + ")" );
        e.consume();
    }

}
