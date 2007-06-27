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
 *      Jan Kiszka <kiszka@rts.uni-hannover.de>
 *
 */
package rack.gui;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import java.util.Vector;

import javax.swing.*;
import javax.swing.event.*;

import rack.main.*;
import rack.main.tims.*;

public final class Gui extends Thread
{
    //
    // global variables
    //
    
    static final int GUI_MAX_INSTANCES = 4;

    // general
    Vector<GuiElementDescriptor>    elements    = new Vector<GuiElementDescriptor>();
    Vector<GuiGroupDescriptor>      groups      = new Vector<GuiGroupDescriptor>();
    Vector<GuiWorkspaceDescriptor>  workspaces  = new Vector<GuiWorkspaceDescriptor>();
    Vector<String>                  jarfiles    = new Vector<String>();

    boolean                 terminate = false;

    GuiCfg                  cfg;

    ClassLoader             guiCL;

    Tims                    tims;
    String                  timsClass = null;
    String                  timsParam = "";
    String                  rackName = "";

    byte                    getStatusSeqNr = 100;
    TimsMbx                 getStatusReplyMbx;

    // main frame
    JFrame                  mainFrame;
    Container               mainFrameContent;
    Point                   mainFrameLocation = new Point(0, 0);
    Dimension               mainFrameSize = new Dimension(800, 600);
    int                     fullScreenModule = -1;
    // navigation panel
    JPanel                  navPanel, navInterPanel;
    JScrollPane             navScrollPanel;
    Color                   navGroupButtonFG;
    Color                   navStatusButtonBG;
    // workspace
    JTabbedPane             jtp;

    // special windows
    GDOSGui                 gdosGui = null;
    JInternalFrame          gdosFrame;
    TimsMbx                 gdosMbx;
    
    boolean                 showMapView = false;
    MapViewGui              mapViewGui = null;
    int                     mapViewWorkSpace;
    JInternalFrame          mapViewFrame;
    TimsMbx                 mapViewReplyMbx;

    //
    // constructor
    //
    
    public Gui(JFrame mainFrame, Container mainFrameContent, BufferedReader cfgReader,
               String timsClass, String timsParam) throws Exception
    {
        this.mainFrame = mainFrame;
        this.mainFrameContent = mainFrameContent;

        try
        {
            // reading config file
            cfg = new GuiCfg(this);
            cfg.readConfig(cfgReader);

            guiCL = this.getContextClassLoader();
            
            // load additional jar files
            for (int i = 0; i < jarfiles.size(); i++)
            {
                try
                {
                    File jarfile = new File(jarfiles.get(i));
                    
                    URL urls[] = new URL[] { jarfile.toURI().toURL() };
                    guiCL = new URLClassLoader(urls, guiCL);

                    System.out.println("File " + jarfile + " has been loaded");
                }
                catch (Exception e)
                {
                    JOptionPane.showMessageDialog(mainFrameContent,
                                    "Can't load jar file.\n" +
                                    jarfiles.get(i),
                                    "RACK GUI", JOptionPane.ERROR_MESSAGE);
                    throw e;
                }
            }
            
            // load RackName extension
            if(rackName.length() > 0)
            {
                try
                {
                    guiCL.loadClass(rackName)
                        .getMethod("initClassStringTable", (Class<?>[])null)
                        .invoke((Object)null, (Object[])null);
                }
                catch (Exception e)
                {
                    JOptionPane.showMessageDialog(mainFrameContent,
                                    "Can't load RackName extension.\n" +
                                    rackName,
                                    "RACK GUI", JOptionPane.ERROR_MESSAGE);
                    throw e;
                }
            }
            else
            {
                RackName.initClassStringTable();
            }
    
            // connect to router ...
    
            // constructor parameter overwrites config file
            if(timsClass != null)
            {
                this.timsClass = timsClass;
            }
            
            if (this.timsClass == null)
            {
                this.timsClass = "rack.main.tims.TimsTcp";
            }
    
            if(timsParam != "")
            {
                this.timsParam = timsParam;
            }

            System.out.println("Connect to TimsRouter \"" + this.timsClass + "\" param \"" + this.timsParam + "\"");

            Class<?>[] timsConstrArgType = new Class<?>[] {String.class};
            Object[] timsConstrArg = new Object[1];
            timsConstrArg[0] = this.timsParam;

            try
            {
               tims = (Tims) guiCL.loadClass(this.timsClass)
                       .getConstructor(timsConstrArgType)
                       .newInstance(timsConstrArg);
            }
            catch (Exception e)
            {
                JOptionPane.showMessageDialog(mainFrameContent,
                        "Can't connect to TimsRouter\n" +
                        "\"" + this.timsClass + "\" param \"" + this.timsParam + "\"\n",
                        "RACK GUI",
                        JOptionPane.ERROR_MESSAGE);
                throw e;
            }
    
            System.out.println("Initializing Proxies ...");
            initMbx();
            initProxies();
    
            System.out.println("Initializing GUI ...");
            initGui();
            start();

            System.out.println("Restoring GuiElements ...");
            restoreGuiElements();

            if(mainFrame != null)
            {
                mainFrame.setTitle("RACK GUI (" + this.timsParam + ")");
                mainFrame.setLocation(mainFrameLocation);
                mainFrame.setSize(mainFrameSize);
                mainFrame.setVisible(true);
            }
            System.out.println("GUI started ...");
        }
        catch(Exception e)
        {
            terminate();
            throw e;
        }
    }

    //
    // public interface
    //
    
    public GuiElementDescriptor getGuiElement(int moduleName, int instance)
    {
        for(int i = 0; i < elements.size(); i++)
        {
            GuiElementDescriptor ge = elements.get(i);
            
            if((ge.proxy != null) && (ge.proxy.getCommandMbx() == RackName.create(moduleName, instance)))
            {
                return ge;
            }
        }
        return null;
    }

    public RackProxy getProxy(int moduleName, int instance)
    {
        GuiElementDescriptor ge = getGuiElement(moduleName, instance);
        
        if(ge != null)
        {
            return ge.proxy;
        }
        else
        {
            return null;
        }
    }

    public Tims getTims()
    {
        return tims;
    }

    //
    // end public interface
    //
    
    protected void initMbx() throws TimsException
    {
        int inst = 0;

        while (true)
        {
            try
            {
                getStatusReplyMbx = tims.mbxInit(RackName.create(RackName.GUI, inst, 0));
                break;
            }
            catch (TimsException e)
            {
                if (++inst == GUI_MAX_INSTANCES) {
                    JOptionPane.showMessageDialog(mainFrameContent,
                            "Can't create Mailbox\n" + e.getMessage(),
                            "Tims Exception", JOptionPane.ERROR_MESSAGE);
                    throw e;
                }
            }
        }
        try
        {
            for (int i = 0; i < elements.size(); i++)
                elements.get(i).replyMbx = tims.mbxInit(RackName.create(RackName.GUI, inst, i+1));

            mapViewReplyMbx = tims.mbxInit(RackName.create(RackName.GUI, inst, elements.size()+1));
            if (inst == 0)
                gdosMbx = tims.mbxInit(RackName.create(RackName.GDOS, inst));
        }
        catch (TimsException e)
        {
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't create Mailbox\n" + e.getMessage(),
                    "Tims Exception", JOptionPane.ERROR_MESSAGE);
            throw e;
        }
    }

    protected void initProxies() throws Exception
    {
        for (int i = 0; i < elements.size(); i++)
        {
            GuiElementDescriptor ge = elements.get(i);
            
            if(ge.proxyClass.length() > 0)
            {
                initProxy(elements.get(i));
            }
        }
    }

    protected void initProxy(GuiElementDescriptor ge) throws Exception
    {
        Class<?> rackProxyClass;

        try
        {
            rackProxyClass = guiCL.loadClass(ge.proxyClass);
        }
        catch (Exception e)
        {
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't load RackProxy.\n" +
                    ge.proxyClass,
                    "RACK GUI", JOptionPane.ERROR_MESSAGE);
            throw e;
        }
        
        Class<?>[] proxyConstrArgsTypes = new Class<?>[2];
        Object[] proxyConstrArgs = new Object[2];

        proxyConstrArgsTypes[0] = int.class;
        proxyConstrArgs[0] = new Integer(ge.instance);
        proxyConstrArgsTypes[1] = TimsMbx.class;
        proxyConstrArgs[1] = ge.replyMbx;

        try
        {
            ge.proxy = (RackProxy) rackProxyClass.getConstructor(proxyConstrArgsTypes)
                                                 .newInstance(proxyConstrArgs);
        }
        catch (Exception e)
        {
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't create RackProxy instance.\n" +
                    ge.cfg,
                    "RACK GUI", JOptionPane.ERROR_MESSAGE);
            throw e;
        }
    }

    protected void initGui() throws Exception
    {
        try
        {
            UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
            // UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        }
        catch (Exception e)
        {
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't set LookAndFeel",
                    "RACK GUI", JOptionPane.ERROR_MESSAGE);
            throw e;
        }

        // create navigation panel (Module Monitor on the left border)

        navPanel = new JPanel();
        navInterPanel = new JPanel(new BorderLayout());
        navInterPanel.add(BorderLayout.NORTH, navPanel);

        navScrollPanel = new JScrollPane(navInterPanel);
        navScrollPanel.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        navScrollPanel.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
        navScrollPanel.setPreferredSize(new Dimension(160, 10));

        // create groupsPanel
        for (int i = 0; i < groups.size(); i++)
        {
            GuiGroupDescriptor gg = groups.get(i);
            
            gg.button        = new JButton(gg.name);
            gg.panel         = new JPanel();
            gg.interPanel    = new JPanel();

            gg.button.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent ae)
                {
                    Object o = ae.getSource();
                    GuiGroupDescriptor g = null;

                    for (int i = 0; i < groups.size(); i++)
                    {
                        if (o == groups.get(i).button)
                        {
                            g = groups.get(i);
                        }
                    }
                    if (g.panel.isShowing())
                    {
                        g.panel.setVisible(false);
                    }
                    else
                    {
                        g.panel.setVisible(true);
                    }
                }
            });

            gg.button.setHorizontalAlignment(SwingConstants.LEFT);

            gg.panel = new JPanel(new GridLayout(0, 1));
            gg.panel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 0));

            gg.interPanel = new JPanel(new BorderLayout());
            gg.interPanel.add(gg.button, BorderLayout.NORTH);
            gg.interPanel.add(gg.panel, BorderLayout.CENTER);

            navPanel.setLayout(new BoxLayout(navPanel, BoxLayout.Y_AXIS));
            navPanel.add(gg.interPanel);
        }

        JButton allOffButton = new JButton("all off");
        allOffButton.setHorizontalAlignment(SwingConstants.LEFT);
        allOffButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent ae)
            {
                for (int i = elements.size() - 1; i >= 0; i--)
                {
                    elements.get(i).proxy.off();

                    try
                    {
                        Thread.sleep(100);
                    }
                    catch (InterruptedException e)
                    {
                    }
                }
            }
        });

        JPanel allOffInterPanel = new JPanel(new BorderLayout());
        allOffInterPanel.add(allOffButton, BorderLayout.NORTH);
        navPanel.add(allOffInterPanel);

        mainFrameContent.add(navScrollPanel, BorderLayout.WEST);

        // create work panel (Tabbed Panel in the center)
        jtp = new JTabbedPane();
        for (int i = 0; i < workspaces.size(); i++)
        {
            GuiWorkspaceDescriptor gw = workspaces.get(i);
            
        	gw.jdp = new JDesktopPane();
         	jtp.add(gw.name, gw.jdp);
        }
        mainFrameContent.add(jtp, BorderLayout.CENTER);

        // create message frame as an internal frame
        gdosFrame = new JInternalFrame("GDOS Message", true, false, true, true);
        if (gdosMbx != null) {
            gdosGui = new GDOSGui(gdosMbx);
            gdosFrame.getContentPane().add(gdosGui.getComponent());
            gdosFrame.pack();
            gdosFrame.setVisible(true);
            workspaces.get(0).jdp.add(gdosFrame);
            gdosGui.start();
        }

        if (showMapView)
        {
            // create mapView panel as a tab
        	mapViewFrame = new JInternalFrame("MapView", true, false, true, true);
            mapViewGui = new MapViewGui(elements, mapViewReplyMbx);
        	mapViewFrame.getContentPane().add(mapViewGui.getComponent());
        	mapViewFrame.pack();        	
        	mapViewFrame.setVisible(true);
            workspaces.get(mapViewWorkSpace).jdp.add(mapViewFrame);
        	mapViewFrame.setLocation(0, 0);
        	mapViewFrame.setSize(600, 400);
        }
        
        mainFrameContent.setVisible(true);
    }

    protected void restoreGuiElements()
    {
        for (int i = 0; i < elements.size(); i++)
        {
            GuiElementDescriptor ge = elements.get(i);
            
            if ((ge.size.width != 0) && (ge.size.height != 0))
            {
                openGuiElement(ge);
            }
        }
    }

    protected void initGuiElement(GuiElementDescriptor ge)
    {
        Class<?> guiElementClass;

        try
        {
            guiElementClass = guiCL.loadClass(ge.guiClass);
        }
        catch (ClassNotFoundException e)
        {
            e.printStackTrace();
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't load GuiElement\n" +
                    ge.guiClass,
                    "RACK GUI", JOptionPane.ERROR_MESSAGE);
            return;
        }

        // fuer den konstruktor ...(GuiElementDescriptor guiElement)
        Class<?>[] guiConstrArgsTypes = new Class<?>[1];
        Object[] guiConstrArgs = new Object[1];

        guiConstrArgsTypes[0] = GuiElementDescriptor.class;
        guiConstrArgs[0] = ge;

        try
        {
            ge.gui = (GuiElement) guiElementClass.getConstructor(guiConstrArgsTypes)
                                                 .newInstance(guiConstrArgs);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't create GuiElement instance\n" +
                    ge.cfg,
                    "RACK GUI", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        ge.gui.start();

        ge.frame = new JInternalFrame(ge.name, true, true, true, true);
        ge.frame.getContentPane().add(ge.gui.getComponent());

        Action action = new AbstractAction()
        {
            private static final long serialVersionUID = 1L;

            public void actionPerformed(ActionEvent e)
            {
                if (fullScreenModule == -1)
                {
                    JInternalFrame frame = workspaces.get(jtp.getSelectedIndex()).jdp.getSelectedFrame();
                    int module;
                    for (module = 0; module < elements.size(); module++)
                    {
                        if (elements.get(module).frame == frame)
                            break;
                    }
                    if (module == elements.size())
                        return;

                    fullScreenModule = module;
                    elements.get(module).gui.toggleFullScreen();
                }
                else
                {
                    elements.get(fullScreenModule).gui.toggleFullScreen();
                    fullScreenModule = -1;
                }
            }
        };

        ge.gui.getComponent().getInputMap().
            put(KeyStroke.getKeyStroke("F11"), "fullScreen");
        ge.gui.getComponent().getInputMap(JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT).
            put(KeyStroke.getKeyStroke("F11"), "fullScreen");
        ge.gui.getComponent().getActionMap().put("fullScreen", action);

        ge.frame.addInternalFrameListener(new InternalFrameAdapter()
        {
            public void internalFrameClosing(InternalFrameEvent e)
            {
                Object sourceFrame = e.getSource();
                GuiElementDescriptor ge = null;

                for (int i = 0; i < elements.size(); i++)
                {
                    ge = elements.get(i);
                    if (sourceFrame == ge.frame)
                    {
                        break;
                    }
                }
                ge.gui.terminate();
                ge.gui = null;
                ge.frame = null;
            }
        });
        ge.frame.pack();
        ge.frame.setVisible(true);

        if ((ge.size.width == 0) && (ge.size.height == 0))
        {
            int id = elements.indexOf(ge);
            ge.frame.setLocation(100 * (id / 10) + 20 * (id % 10),
                    25 + 25 * (id % 10));
        }
        else
        {
            ge.frame.setLocation(ge.location);
            ge.frame.setSize(ge.size);
        }
        ge.workspace.jdp.add(ge.frame);
    }

    protected void addNavPanel(GuiElementDescriptor ge)
    {
        ge.navPanel = new JPanel(new BorderLayout());

        ge.navStatusButton = new JRadioButton();
        ge.navStatusButton.setActionCommand(Integer.toString(elements.indexOf(ge)));
        ge.navStatusButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent ae)
            {
                try
                {
                    int module = Integer.parseInt(ae.getActionCommand());
                    GuiElementDescriptor ge = elements.get(module);
                    
                    if (ge.navStatusButton.isSelected() == true)
                    {
                        ge.proxy.on();
                    }
                    else
                    {
                        ge.proxy.off();
                    }
                }
                catch (NumberFormatException e)
                {
                }
            }
        });

        // if (moduleGui[id] == null)
        ge.navButton = new JButton(ge.name);
        ge.navButton.setToolTipText(ge.name);
        // else
        // moduleButton[id] = new JButton(moduleGui[id].getModuleName());
        ge.navButton.setHorizontalAlignment(SwingConstants.LEFT);
        ge.navButton.setActionCommand(Integer.toString(elements.indexOf(ge)));
        ge.navButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent ae)
            {
                try
                {
                    int module = Integer.parseInt(ae.getActionCommand());
                    GuiElementDescriptor ge = elements.get(module);

                    if ((ae.getModifiers() & ActionEvent.CTRL_MASK) == 0)
                    {
                        openGuiElement(ge);
                        relocateGuiElement(ge);
                    }
                    else
                    {
                        if (ge.status == RackProxy.MSG_DISABLED)
                        {
                            ge.proxy.on();
                        }
                        else
                        {
                            ge.proxy.off();
                        }
                    }
                }
                catch (NumberFormatException e)
                {
                }
            }
        });

        ge.navPanel.add(BorderLayout.WEST, ge.navStatusButton);
        ge.navPanel.add(BorderLayout.CENTER, ge.navButton);
        // System.out.println("modulelePanel[" + id + "] is ok");

        ge.group.panel.add(ge.navPanel);
    }

    protected void openGuiElement(GuiElementDescriptor ge)
    {
        if (ge.gui == null)
        {
            initGuiElement(ge);
        }

        if(ge.gui != null)
        {
            try
            {
                ge.frame.moveToFront();
                ge.frame.setSelected(true);
                jtp.setSelectedIndex(workspaces.indexOf(ge.workspace));
            }
            catch (java.beans.PropertyVetoException pe)
            {
            }
        }
    }

    protected void relocateGuiElement(GuiElementDescriptor ge)
    {
        Point     pos       = ge.frame.getLocation();
        Dimension size      = ge.frame.getSize();
        Dimension paneSize  = ge.frame.getDesktopPane().getSize();

        if (pos.x < 0)
            pos.x = 0;
        else if (pos.x + size.width > paneSize.width)
            pos.x = paneSize.width - size.width;

        if (pos.y < 0)
            pos.y = 0;
        else if (pos.y + size.height > paneSize.height)
            pos.y = paneSize.height - size.height;

        ge.frame.setLocation(pos);
    }

    protected void updateAllStatus()
    {
        for (int i = 0; i < groups.size(); i++)
        {
            GuiGroupDescriptor gg = groups.get(i);
            
            gg.on     = 0;
            gg.error  = 0;
            gg.sum    = 0;
        }

        for (int i = 0; i < elements.size(); i++)
        {
            GuiElementDescriptor ge = elements.get(i); 

            ge.status = RackProxy.MSG_TIMEOUT;
        }

        try
        {
            //System.out.println("Get all status");
            //int getAllStatusTime = (int)(System.nanoTime() / 1000000L);

            getStatusSeqNr++;
            if (getStatusSeqNr > 100)
                getStatusSeqNr = 0;

            for (int i = 0; i < elements.size(); i++)
            {
                GuiElementDescriptor ge = elements.get(i);

                // alle moduleProxy sind bei initModuleProxy schon vorhanden

                getStatusReplyMbx.send0(RackProxy.MSG_GET_STATUS, 
                        ge.proxy.getCommandMbx(),
                        (byte) 0,
                        (byte) getStatusSeqNr);
                // ein bischen warten, um nicht stossweise last zu erzeugen.
                try
                {
                    Thread.sleep(10);
                }
                catch (InterruptedException e)
                {
                }
            }

            TimsRawMsg reply;
            boolean notAllReplies = true;

            // wait for all replies or timeout
            while (notAllReplies)
            {
                // receive reply
                reply = getStatusReplyMbx.receive(1000);
                if (reply.seqNr == getStatusSeqNr)
                {
                    //int replyTime = (int)(System.nanoTime() / 1000000L);
                    //System.out.println("Status reply: " + reply.toString() + " time " + (replyTime - getAllStatusTime));
                    
                    // update module status array
                    for (int i = 0; i < elements.size(); i++)
                    {
                        GuiElementDescriptor ge = elements.get(i);

                        if (ge.proxy.getCommandMbx() == reply.src)
                        {
                            ge.status = reply.type;
                        }
                    }
                }

                // test if all replies are received
                notAllReplies = false;
                for (int i = 0; i < elements.size(); i++)
                {
                    GuiElementDescriptor ge = elements.get(i);

                    if (ge.status == RackProxy.MSG_TIMEOUT)
                    {
                        notAllReplies = true;
                    }
                }
            }
        }
        catch (TimsException e)
        {
            // System.out.println("Java Gui getStatus error: " +
            // e.getMessage());
        }

        if(navStatusButtonBG == null)
        {
            navStatusButtonBG = groups.get(0).button.getBackground();
        }
            
        for (int i = 0; i < elements.size(); i++)
        {
            GuiElementDescriptor ge = elements.get(i);
            
            // bei paramentern -show und -start soll trotzdem
            // der button angezeigt werden.
            if (ge.show && ge.status == RackProxy.MSG_NOT_AVAILABLE)
            {
                ge.status = RackProxy.MSG_DISABLED;
            }
            if (ge.start && ge.status == RackProxy.MSG_NOT_AVAILABLE)
            {
                ge.status = RackProxy.MSG_DISABLED;
            }

            switch (ge.status)
            {
                case RackProxy.MSG_ERROR:
                    if (ge.navPanel == null)
                    {
                        addNavPanel(ge);
                    }
                    ge.navStatusButton.setEnabled(true);
                    ge.navStatusButton.setSelected(true);
                    ge.navStatusButton.setBackground(Color.RED);
                    ge.navButton.setEnabled(true);

                    ge.group.sum++;
                    ge.group.error++;
                    break;

                case RackProxy.MSG_ENABLED:
                    if (ge.navPanel == null)
                    {
                        addNavPanel(ge);
                    }
                    ge.navStatusButton.setEnabled(true);
                    ge.navStatusButton.setSelected(true);
                    ge.navStatusButton.setBackground(Color.GREEN);
                    ge.navButton.setEnabled(true);

                    ge.group.sum++;
                    ge.group.on++;
                    break;

                case RackProxy.MSG_DISABLED:
                    if (ge.navPanel == null)
                    {
                        addNavPanel(ge);
                    }
                    ge.navStatusButton.setEnabled(true);
                    ge.navStatusButton.setSelected(false);
                    ge.navStatusButton.setBackground(navStatusButtonBG);
                    ge.navButton.setEnabled(true);

                    ge.group.sum++;
                    break;

                case RackProxy.MSG_NOT_AVAILABLE:
                    if (ge.navPanel != null)
                    {
                        ge.navStatusButton.setEnabled(false);
                        ge.navStatusButton.setSelected(false);
                        ge.navStatusButton.setBackground(navStatusButtonBG);
                        ge.navButton.setEnabled(false);
                    }
                    break;

                case RackProxy.MSG_TIMEOUT:
                default:
                    if (ge.navPanel != null)
                    {
                        ge.navStatusButton.setEnabled(true);
                        ge.navStatusButton.setSelected(true);
                        ge.navStatusButton.setBackground(Color.ORANGE);
                        ge.navButton.setEnabled(false);
                    }
            }
        }
    }

    public void run()
    {
        while (terminate == false)
        {
            updateAllStatus();
            
            if((navGroupButtonFG == null) & (groups.get(0) != null))
            {
                navGroupButtonFG = groups.get(0).button.getForeground();
            }
            
            for (int i = 0; i < groups.size(); i++)
            {
                GuiGroupDescriptor gg = groups.get(i);
                
                if (gg.error > 0)
                {
                    gg.button.setForeground(Color.RED);
                }
                else
                {
                    gg.button.setForeground(navGroupButtonFG);
                }
                gg.button.setText(gg.name +
                        " (" + gg.error + " , " + gg.on + " , " + gg.sum + ")");
            }
            navPanel.revalidate();
            try
            {
                Thread.sleep(2000);
            }
            catch (InterruptedException e) {}
        }
    }

    public void terminate()
    {
        System.out.println("Terminating ...");

        terminate = true;
        mainFrameContent.setVisible(false);
        
        // terminate gui thread
        try
        {
            this.interrupt();
            this.join(1000);
        }
        catch (Exception e) {}
        
        // terminate MapViewGui
        if(mapViewGui != null)
        {
            mapViewGui.terminate();
        }

        // terminate module guis
        for (int i = 0; i < elements.size(); i++)
        {
            GuiElementDescriptor ge = elements.get(i);

            if(ge.gui != null)
            {
                ge.gui.terminate();
            }
        }

        // terminate GDOSGui
        if(gdosGui != null)
        {
            gdosGui.terminate();
        }

        // terminate connection to TimsRouter
        if(tims != null)
        {
            tims.terminate();
        }

        // JOptionPane.showMessageDialog(frame,
        // "GUI Terminated",
        // "RACK GUI", JOptionPane.INFORMATION_MESSAGE);
        System.out.println("GUI terminated");
    }

    //
    // Starting RACK GUI as an application
    //
    
    protected static File mainSaveConfigFile;
    protected static Gui mainGui;

    public static void main(String[] args)
    {
        System.out.println("RACK GUI");
        
        mainSaveConfigFile = null;
        mainGui = null;

        try
        {
            // create main frame
            JFrame frame = new JFrame("Robot Gui");
            frame.addWindowListener(new WindowAdapter()
            {
                public void windowClosing(WindowEvent event)
                {
                    // write GUI configuration
                    if((mainSaveConfigFile != null) &&
                       (mainGui != null))
                    {
                        try
                        {
                            mainGui.cfg.writeConfig(new BufferedWriter(new FileWriter(mainSaveConfigFile)));
                        }
                        catch (IOException e)
                        {
                            JOptionPane.showMessageDialog(null,
                                    "Can't write config file",
                                    "RACK GUI", JOptionPane.ERROR_MESSAGE);
                            System.out.println("Can't write config file");
                        }
                    }

                    mainSaveConfigFile = null;
                    mainGui = null;
                    
                    System.exit(0);
                }
            });

            // reading config file

            String      fileName    = "";
            String      timsParam   = "";

            switch (args.length)
            {
                case 0:
                    FileDialog fd = new FileDialog(frame, "RACK GUI config", FileDialog.LOAD);
                    fd.setFile("*.cfg");
                    fd.setDirectory(".");
                    fd.setLocation(150, 150);
                    fd.setVisible(true);
                    if (fd.getFile() == null)
                    	System.exit(0);
                    fileName = fd.getDirectory() + System.getProperty("file.separator").charAt(0) + fd.getFile();
                    break;
                case 1: // only config file
                    fileName = args[0];
                    break;
                case 2: // config file and ip
                    fileName = args[0];
                    timsParam = args[1];
                    break;
                default:
                    JOptionPane.showMessageDialog(frame,
                            "Invalid argument count \"" + args.length + "\". Start Gui like:\n" +
                            "'java -jar rack.jar <config-file> <ip>' or\n" +
                            "'java -classpath ... rack.gui.Gui <config-file> <ip>",
                            "RACK GUI", JOptionPane.ERROR_MESSAGE);
                    throw new Exception("Invalid argument count " + args.length);
            }

            try
            {
                System.out.println("Load config file \"" + fileName + "\"");

                BufferedReader cfgReader = new BufferedReader(new FileReader(fileName));
                mainSaveConfigFile  = new File(fileName);

                mainGui = new Gui(frame, frame.getContentPane(), cfgReader, null, timsParam);
            }
            catch(IOException e)
            {
                JOptionPane.showMessageDialog(frame,
                        "Can't load config file \"" + fileName + "\"",
                        "RACK GUI", JOptionPane.ERROR_MESSAGE);
                throw e;
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
            JOptionPane.showMessageDialog(null,
                    "Can't start RACK GUI", "RACK GUI", JOptionPane.ERROR_MESSAGE);

            mainSaveConfigFile = null;
            mainGui = null;

            System.exit(-1);
        }
    }
}
