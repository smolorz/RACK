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

                gui = new Gui(this, cfgReader, routerAdr, 2000);
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
