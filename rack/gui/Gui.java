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
import java.beans.PropertyVetoException;
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

    public static int FRAME_STATE_NORMAL    = 0;
    public static int FRAME_STATE_MAX       = 1;
    public static int FRAME_STATE_ICON      = 2;
    public static int FRAME_STATE_FIXED     = 3;
    public static int FRAME_STATE_FIXED_MAX = 4;

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
    int                     mainFrameState = FRAME_STATE_NORMAL;
    int                     fullScreenElement = -1;
    // navigation panel
    JPanel                  navPanel, navInterPanel;
    JScrollPane             navScrollPanel;
    Color                   navStatusButtonBG;
    int                     navUpdate = 2000;
    // workspace
    JTabbedPane             jtp;

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

            System.out.println("Initializing GuiClassLoader ...");
            initClassLoader();

            System.out.println("Initializing RackName extension ...");
            initRackName();
    
            System.out.println("Initializing Tims  ...");
            initTims(timsClass, timsParam);
    
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
                if((mainFrameState == FRAME_STATE_MAX) ||
                   (mainFrameState == FRAME_STATE_FIXED_MAX))
                {
                    mainFrame.setExtendedState(Frame.MAXIMIZED_BOTH);
                }
                if(mainFrameState == FRAME_STATE_ICON)
                {
                    mainFrame.setExtendedState(Frame.ICONIFIED);
                }
                
                if(mainFrameState == FRAME_STATE_FIXED)
                {
                    mainFrame.setResizable(false);
                }
                
                if(mainFrameState == FRAME_STATE_FIXED_MAX)
                {
                    mainFrame.setUndecorated(true);
                }
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

    public Vector<GuiElementDescriptor> getGuiElements()
    {
        return elements;
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

    protected void initClassLoader() throws Exception
    {
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
    }
    
    protected void initRackName() throws Exception
    {
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
    }

    protected void initTims(String timsClass, String timsParam) throws Exception
    {
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
    }

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
                System.out.println(e);
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
            
            if(ge.instance >= 0)
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
                    GuiElementDescriptor ge = elements.get(i);
                    if(ge.proxy != null)
                    {
                        ge.proxy.off();
    
                        try
                        {
                            Thread.sleep(100);
                        }
                        catch (InterruptedException e)
                        {
                        }
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
        ge.frame.setResizable(true);
        ge.frame.setMaximizable(true);
        ge.frame.setIconifiable(true);

        ge.frame.getContentPane().add(ge.gui.getComponent());

        Action action = new AbstractAction()
        {
            private static final long serialVersionUID = 1L;

            public void actionPerformed(ActionEvent e)
            {
                if (fullScreenElement == -1)
                {
                    JInternalFrame frame = workspaces.get(jtp.getSelectedIndex()).jdp.getSelectedFrame();
                    int i;
                    for (i = 0; i < elements.size(); i++)
                    {
                        if (elements.get(i).frame == frame)
                            break;
                    }
                    if (i == elements.size())
                        return;

                    fullScreenElement = i;
                    elements.get(i).gui.toggleFullScreen();
                }
                else
                {
                    elements.get(fullScreenElement).gui.toggleFullScreen();
                    fullScreenElement = -1;
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
                
                ge.location = new Point();
                ge.size = new Dimension();
                ge.frameState = FRAME_STATE_NORMAL;
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

        try
        {
            if((ge.frameState == FRAME_STATE_MAX) ||
               (ge.frameState == FRAME_STATE_FIXED_MAX))
            {
                ge.frame.setMaximum(true);
            }
            if(ge.frameState == FRAME_STATE_ICON)
            {
                ge.frame.setIcon(true);
            }
            if((ge.frameState == FRAME_STATE_FIXED) ||
               (ge.frameState == FRAME_STATE_FIXED_MAX))
            {
                ge.frame.setClosable(false);
                ge.frame.setResizable(false);
                ge.frame.setMaximizable(false);
                ge.frame.setIconifiable(false);
            }
        }
        catch (PropertyVetoException e)
        {
        }
    }

    protected void addNavPanel(GuiElementDescriptor ge)
    {
        ge.navPanel = new JPanel(new BorderLayout());

        if(ge.proxy != null)
        {
            ge.navStatusButton = new JRadioButton();
            ge.navStatusButton.setActionCommand(Integer.toString(elements.indexOf(ge)));
            ge.navStatusButton.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent ae)
                {
                    try
                    {
                        int i = Integer.parseInt(ae.getActionCommand());
                        GuiElementDescriptor ge = elements.get(i);
                        
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
        }
        else
        {
            ge.navStatusButton = new JRadioButton("");
        }

        ge.navButton = new JButton(ge.name);
        ge.navButton.setToolTipText(ge.name);
        ge.navButton.setHorizontalAlignment(SwingConstants.LEFT);
        ge.navButton.setActionCommand(Integer.toString(elements.indexOf(ge)));
        ge.navButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent ae)
            {
                try
                {
                    int i = Integer.parseInt(ae.getActionCommand());
                    GuiElementDescriptor ge = elements.get(i);

                    openGuiElement(ge);
                    relocateGuiElement(ge);
                }
                catch (NumberFormatException e)
                {
                }
            }
        });

        ge.navPanel.add(BorderLayout.WEST, ge.navStatusButton);
        ge.navPanel.add(BorderLayout.CENTER, ge.navButton);

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
                if(ge.frame.isIcon())
                {
                    ge.frame.setIcon(false);
                }
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

        if (pos.x + size.width > paneSize.width)
            pos.x = paneSize.width - size.width;

        if (pos.y + size.height > paneSize.height)
            pos.y = paneSize.height - size.height;

        ge.frame.setLocation(pos);

        if (pos.x < 0)
            pos.x = 0;

        if (pos.y < 0)
            pos.y = 0;

        ge.frame.setLocation(pos);
    }

    public void run()
    {
        GuiElementDescriptor ge; 
        int i;
        
        navStatusButtonBG = groups.get(0).button.getBackground();

        if(navUpdate <= 0)  // no nav update
        {
            for (int j = 0; j < elements.size(); j++)
            {
                ge = elements.get(j); 
                ge.status = RackProxy.MSG_DISABLED;
                
                addNavPanel(ge);
                ge.navStatusButton.setEnabled(false);
                ge.navStatusButton.setSelected(false);
                ge.navStatusButton.setBackground(navStatusButtonBG);
                ge.navButton.setEnabled(true);
            }
            
            navPanel.revalidate();

            while (terminate == false)
            {
                try
                {
                    Thread.sleep(1000);
                }
                catch (InterruptedException e) {}
            }
        } // no nav update

        for (int j = 0; j < elements.size(); j++)
        {
            ge = elements.get(j); 
            ge.status = RackProxy.MSG_NOT_AVAILABLE;
            
            if(ge.proxy == null)
            {
                addNavPanel(ge);
                ge.navStatusButton.setEnabled(false);
                ge.navStatusButton.setSelected(false);
                ge.navStatusButton.setBackground(navStatusButtonBG);
                ge.navButton.setEnabled(true);
            }
        }

        getStatusSeqNr = 1;
        i = 0;
        
        while (terminate == false)
        {
            ge = elements.get(i); 

            if(ge.proxy != null)
            {
                if(ge.status == RackProxy.MSG_TIMEOUT)
                {
                    // no reply since last get status
                    if (ge.navPanel != null)
                    {
                        ge.navStatusButton.setEnabled(true);
                        ge.navStatusButton.setSelected(false);
                        ge.navStatusButton.setBackground(Color.ORANGE);
                        ge.navButton.setEnabled(false);
                    }
                }
                
                try
                {
                    getStatusReplyMbx.send0(RackProxy.MSG_GET_STATUS, 
                            ge.proxy.getCommandMbx(),
                            (byte) 0,
                            (byte) getStatusSeqNr);
                }
                catch (TimsException e) {}
                
                ge.status = RackProxy.MSG_TIMEOUT;
            }

            try
            {
                Thread.sleep(navUpdate/elements.size());
            }
            catch (InterruptedException e) {}
            
            try
            {
                while (true)
                {
                    TimsMsg reply = getStatusReplyMbx.receiveIf();
                    
                    // update module status array
                    for (int j = 0; j < elements.size(); j++)
                    {
                        ge = elements.get(j);

                        if(ge.proxy != null)
                        {
                            if (ge.proxy.getCommandMbx() == reply.src)
                            {
                                ge.status = reply.type;

                                // always show navButton of GuiElements with parameter -show and -start
                                if (ge.hasParameter("show") && ge.status == RackProxy.MSG_NOT_AVAILABLE)
                                {
                                    ge.status = RackProxy.MSG_DISABLED;
                                }
                                if (ge.hasParameter("start") && ge.status == RackProxy.MSG_NOT_AVAILABLE)
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
                                            ge.navStatusButton.setSelected(false);
                                            ge.navStatusButton.setBackground(Color.ORANGE);
                                            ge.navButton.setEnabled(false);
                                        }
                                }
                            }
                        }
                    }
                }
            }
            catch (TimsException e)
            {}
            
            navPanel.revalidate();

            i++;
            if(i >= elements.size())
            {
                i = 0;
                getStatusSeqNr++;
                if (getStatusSeqNr > 100)
                    getStatusSeqNr = 1;
            }
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
        
        // terminate gui elements
        for (int i = 0; i < elements.size(); i++)
        {
            GuiElementDescriptor ge = elements.get(i);

            if(ge.gui != null)
            {
                ge.gui.terminate();
            }
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
