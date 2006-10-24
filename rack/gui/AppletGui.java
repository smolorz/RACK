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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
package rack.gui;

import java.applet.Applet;
import java.awt.BorderLayout;
import java.io.*;
import java.net.*;

import javax.swing.JOptionPane;

public final class AppletGui extends Applet {

    private static final long serialVersionUID = 1L;

    private Gui gui = null;

    public void init()
    {
    }

    public void start()
    {
        if(gui == null)
        {
            try
            {
                URL documentBase = this.getDocumentBase();
                System.out.println("Document base \"" + documentBase + "\"");
                
                // reading config file
                BufferedReader cfgReader;

                try
                {
                    URL configURL = new URL("http", documentBase.getHost(), "/gui.cfg");
        
                    System.out.println("Load config file \"" + configURL + "\"");
        
                    cfgReader = new BufferedReader(new InputStreamReader(configURL.openStream()));
                }
                catch(Exception e)
                {
                    JOptionPane.showMessageDialog(this,
                            "Can't read config file\n" +
                            "\"http:" + documentBase.getHost() + "/gui.cfg\"",
                            "RACK APPLET GUI", JOptionPane.ERROR_MESSAGE);
                    throw e;
                }
    
                // get router address
    
                InetAddress routerAdr;
                try
                {
                    routerAdr = InetAddress.getByName(documentBase.getHost());
                }
                catch(UnknownHostException e)
                {
                    JOptionPane.showMessageDialog(this,
                            "Unknown host \"" + documentBase.getHost() + "\"",
                            "RACK APPLET GUI", JOptionPane.ERROR_MESSAGE);
                    throw e;
                }
                
                this.setLayout(new BorderLayout());

                gui = new Gui(null, this, cfgReader, routerAdr, 2000);
            }
            catch (Exception e)
            {
                e.printStackTrace();
                gui = null;
            }
        }
        else
        {
            System.out.println("Gui is allready initialised");
        }
    }

    public void stop()
    {
        if(gui != null)
        {
            gui.terminate();
        }
        gui = null;
    }
}
