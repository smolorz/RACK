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
    
    protected static final int MAX_INST_TRIES = 250;

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

    private ClassLoader             guiCL;

    Tims                    tims[] = new Tims[1];
    String                  timsClass[] = new String[1];
    String                  timsParam[] = new String[1];
    String                  rackName = "";

    byte                    getStatusSeqNr = 100;
    TimsMbx                 getStatusReplyMbx[] = new TimsMbx[1];

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

    JPopupMenu              popup;
    MouseListener           popupListener;
    ActionListener          popupAction;
    GuiElementDescriptor    popupGe;
    JMenuItem[]             popupParameterItems;
    RackParamMsg            popupParameterMsg;
    
    // workspace
    JTabbedPane             jtp;

    //
    // constructor
    //
    
    public Gui(JFrame mainFrame, Container mainFrameContent, BufferedReader cfgReader, String timsClass[],
               String timsParam[]) throws Exception
    {
        this.setName("Gui");
        this.mainFrame = mainFrame;
        this.mainFrameContent = mainFrameContent;

        if (timsParam.length > 1)
        {
            this.timsParam = new String[timsParam.length];
            this.timsClass = new String[timsParam.length];
            this.tims = new Tims[timsParam.length];
            this.getStatusReplyMbx = new TimsMbx[timsParam.length];
        }
        this.timsParam[0] = "";
        this.timsClass[0] = null;
        for (int i = 1; i < this.timsParam.length; i++)
        {
        	this.timsParam[i] = "";
            this.timsClass[i] = null;
        }

        try
        {
            // reading config file
            cfg = new GuiCfg(this);
            cfg.readConfig(cfgReader);

            System.out.println("Initializing GuiClassLoader ...");
            initClassLoader();

            System.out.println("Initializing RackName extension ...");
            initRackName();

            for (int i = 0; i < timsParam.length; i++)
            {
                System.out.println("Initializing Tims " + i + " ...");
                initTims(timsClass[i], timsParam[i], i);
            }

            System.out.println("Initializing Proxies ...");
            initMbx();
            initProxies();

            System.out.println("Initializing GUI ...");
            initGui();

            System.out.println("Restoring GuiElements ...");
            restoreGuiElements();

            if (mainFrame != null)
            {
            	String titleString = "RACK GUI";
            	for (int j = 0; j < tims.length; j++)
            	{
            	    titleString += " " + timsParam[j];
            	}
                mainFrame.setTitle(titleString);
                mainFrame.setLocation(mainFrameLocation);
                mainFrame.setSize(mainFrameSize);
                if ((mainFrameState == FRAME_STATE_MAX) || (mainFrameState == FRAME_STATE_FIXED_MAX))
                {
                    mainFrame.setExtendedState(Frame.MAXIMIZED_BOTH);
                }
                if (mainFrameState == FRAME_STATE_ICON)
                {
                    mainFrame.setExtendedState(Frame.ICONIFIED);
                }

                if (mainFrameState == FRAME_STATE_FIXED)
                {
                    mainFrame.setResizable(false);
                }

                if (mainFrameState == FRAME_STATE_FIXED_MAX)
                {
                    mainFrame.setUndecorated(true);
                }
                mainFrame.setVisible(true);
            }
            System.out.println("RACK GUI started ...");

            // set GuiElements frame state (needs to be done after mainFrame.setVisible(true))
            for (int i = 0; i < elements.size(); i++)
            {
                GuiElementDescriptor ge = elements.get(i);
                if (ge.frame != null)
                {
                    try
                    {
                        if ((ge.frameState == FRAME_STATE_MAX) || (ge.frameState == FRAME_STATE_FIXED_MAX))
                        {
                            ge.frame.setMaximum(true);
                        }
                        if (ge.frameState == FRAME_STATE_ICON)
                        {
                            ge.frame.setIcon(true);
                        }
                        if ((ge.frameState == FRAME_STATE_FIXED) || (ge.frameState == FRAME_STATE_FIXED_MAX))
                        {
                            ge.frame.setClosable(false);
                            ge.frame.setResizable(false);
                            ge.frame.setMaximizable(false);
                            ge.frame.setIconifiable(false);
                        }
                    }
                    catch (PropertyVetoException e)
                    {}
                }
                for(int j = 0; j < ge.cfgSplit.length; j++)
                {
                    if(ge.cfgSplit[j].startsWith("-F"))
                    {
                        try
                        {
                            int fx = Integer.parseInt(ge.cfgSplit[j].substring(2));
                            
                            if((fx >= 1) && (fx <= 8))
                            {
                                ge.fx = fx;

                                jtp.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).
                                    put(KeyStroke.getKeyStroke("F" + fx), "F" + fx);
                                jtp.getActionMap().put("F" + fx, new FxOpenGuiElementAction(ge, this));
                            }
                        }
                        catch(NumberFormatException e)
                        {}
                    }
                }
            }
            start();
        }
        catch (Exception e)
        {
            terminate();
            throw e;
        }
    }

    //
    // public interface
    //
    public GuiElementDescriptor getGuiElement(int moduleName, int system, int instance)
    {
        for(int i = 0; i < elements.size(); i++)
        {
            GuiElementDescriptor ge = elements.get(i);
            
            if((ge.proxy != null) && (ge.proxy.getCommandMbx() == RackName.create(system, moduleName, instance, 0)))
            {
                return ge;
            }
        }
        return null;
    }
    
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

    public RackProxy getProxy(int moduleName, int system, int instance)
    {
        GuiElementDescriptor ge = getGuiElement(moduleName, system, instance);
        
        if(ge != null)
        {
            return ge.proxy;
        }
        else
        {
            return null;
        }
    }
    
    public RackProxy getProxy(int moduleName, int instance)
    {
        GuiElementDescriptor ge = getGuiElement(moduleName, 0, instance);
        
        if(ge != null)
        {
            return ge.proxy;
        }
        else
        {
            return null;
        }
    }

    public Tims getTims(int timsId)
    {
        if ((timsId >= 0) && (timsId < tims.length))
        {
            return tims[timsId];
        }
        else
        {
            return tims[0];
        }
    }

    //
    // end public interface
    //

    protected void initClassLoader() throws Exception
    {
        setGuiCL(this.getContextClassLoader());
        
        // load additional jar files
        for (int i = 0; i < jarfiles.size(); i++)
        {
            try
            {
                File jarfile = new File(jarfiles.get(i));
                
                URL urls[] = new URL[] { jarfile.toURI().toURL() };
                setGuiCL(new URLClassLoader(urls, getGuiCL()));
    
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
                getGuiCL().loadClass(rackName)
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

    protected void initTims(String timsClass, String timsParam, int timsId) throws Exception
    {
        // constructor parameter overwrites config file
        if(timsClass != null)
        {
            this.timsClass[timsId] = timsClass;
        }
        
        if (this.timsClass[timsId] == null)
        {
            this.timsClass[timsId] = "rack.main.tims.TimsTcp";
        }
    
        if(timsParam != "")
        {
            this.timsParam[timsId] = timsParam;
        }
    
        System.out.println("Connect to TimsRouter \"" + this.timsClass[timsId] + "\" param \"" + this.timsParam[timsId] + "\"");
    
        Class<?>[] timsConstrArgType = new Class<?>[] {String.class};
        Object[] timsConstrArg = new Object[1];
        timsConstrArg[0] = this.timsParam[timsId];
    
        try
        {
           tims[timsId] = (Tims) getGuiCL().loadClass(this.timsClass[timsId])
                                 .getConstructor(timsConstrArgType)
                                 .newInstance(timsConstrArg);
        }
        catch (Exception e)
        {
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't connect to TimsRouter\n" +
                    "\"" + this.timsClass + "\" param \"" + this.timsParam[timsId] + "\"\n",
                    "RACK GUI",
                    JOptionPane.ERROR_MESSAGE);
            throw e;
        }
    }

    protected void initMbx() throws TimsException
    {
        int inst = 0;

        // randomize timer

        for (int i = 0; i < tims.length; i++)
        {
            for(int instTries = 0; instTries < MAX_INST_TRIES; instTries++)
            {
        	    try
        	    {
        		    inst = inst + 1;// (int)(Math.random() * 255.0);
        		    System.out.println("Init MBX " + String.format("%08X", RackName.create(RackName.GUI, inst)));
        		    getStatusReplyMbx[i] = tims[i].mbxInit(RackName.create(RackName.GUI, inst));
        		    break;
        	    }
        	    catch (TimsException e)
        	    {
                    System.out.println("Caught exception: "+e);
                    if (instTries == MAX_INST_TRIES - 1)
                    {
        			    JOptionPane.showMessageDialog(mainFrameContent, "Can't create Mailbox\n" + e.getMessage(),
                                                      "Tims Exception", JOptionPane.ERROR_MESSAGE);
                        throw e;
                    }
                }
            }
        }
        
        try
        {
        	System.out.println("Init sub MBX " + String.format("%08X", RackName.create(RackName.GUI, inst, 1)) +
        	                   "-" + String.format("%08X", RackName.create(RackName.GUI, inst, elements.size())));
            int j = 0;
            for (int i = 0; i < elements.size(); i++)
            {
                try
                {
                    j++;
                    //System.out.println("try to init(sub):"+RackName.create(RackName.GUI, inst,j) );
                    elements.get(i).replyMbx = tims[elements.get(i).getTimsId()].mbxInit(RackName.create(RackName.GUI, inst, j));
                }
                catch (TimsException e)
                {
                	try 
                	{
						Thread.sleep(1000);
					} catch (InterruptedException e1) {}
                    j++;
                    //System.out.println("try to init(ssub):"+RackName.create(RackName.GUI, inst,j) );
                    elements.get(i).replyMbx = tims[elements.get(i).getTimsId()].mbxInit(RackName.create(RackName.GUI, inst, j));
                }
            }
        }
        catch (TimsException e)
        {
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't create (Sub) Mailbox\n" + e.getMessage(),
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
            rackProxyClass = getGuiCL().loadClass(ge.getProxyClass());
        }
        catch (Exception e)
        {
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't load RackProxy.\n" +
                    ge.getProxyClass(),
                    "RACK GUI", JOptionPane.ERROR_MESSAGE);
            throw e;
        }
        
        Class<?>[] proxyConstrArgsTypes = new Class<?>[3];
        Object[] proxyConstrArgs = new Object[3];

        proxyConstrArgsTypes[0] = int.class;
        proxyConstrArgs[0] = new Integer(ge.getSystemId());
        proxyConstrArgsTypes[1] = int.class;
        proxyConstrArgs[1] = new Integer(ge.instance);
        proxyConstrArgsTypes[2] = TimsMbx.class;
        proxyConstrArgs[2] = ge.replyMbx;
        
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

        popup = new JPopupMenu();

        popupListener = new MouseAdapter() {

            public void mousePressed(MouseEvent e) {
                maybeShowPopup(e);
            }
    
            public void mouseReleased(MouseEvent e) {
                maybeShowPopup(e);
            }
    
            private void maybeShowPopup(MouseEvent e)
            {
                if (e.isPopupTrigger())
                {
                    for(int i = 0; i < elements.size(); i++)
                    {
                        popupGe = (GuiElementDescriptor)elements.get(i);
                        if((popupGe.navButton == e.getSource()) ||
                           (popupGe.navStatusButton == e.getSource()))
                        {
                            break;
                        }
                    }
                    
                    popup.removeAll();

                    popupParameterMsg = popupGe.proxy.getParameter();

                    if(popupParameterMsg != null)
                    {
                        if(popupParameterMsg.parameterNum > 0)
                        {
                            popupParameterItems = new JMenuItem[popupParameterMsg.parameterNum];
                            
                            for(int i = 0; i < popupParameterMsg.parameterNum; i++)
                            {
                                popupParameterItems[i] = new JMenuItem(popupParameterMsg.parameter[i].toString());
                                popupParameterItems[i].addActionListener(popupAction);
                                popup.add(popupParameterItems[i]);
                            }
                        }
                        else
                        {
                            popupParameterItems = new JMenuItem[1];
                            popupParameterItems[0] = new JMenuItem("no parameter");
                            popupParameterItems[0].setEnabled(false);
                            popup.add(popupParameterItems[0]);
                        }
                    }
                    else
                    {
                        popupParameterItems = new JMenuItem[1];
                        popupParameterItems[0] = new JMenuItem("no reply");
                        popupParameterItems[0].setEnabled(false);
                        popup.add(popupParameterItems[0]);
                    }

                    popup.show(e.getComponent(),
                               e.getX(), e.getY());
                }
            }
        };
        popupAction = new ActionListener() {

            public void actionPerformed(ActionEvent e)
            {
                RackParam parameter = null;
                for(int i = 0; i < popupParameterItems.length; i++)
                {
                    if(popupParameterItems[i] == e.getSource())
                    {
                        parameter = popupParameterMsg.parameter[i];
                    }
                }
                
                String s = (String)JOptionPane.showInputDialog(popupGe.navButton,
                        "Set parameter:\n" + parameter.toString(), popupGe.name, JOptionPane.PLAIN_MESSAGE);
                
                if(s != null)
                {
                    try
                    {
                        switch(parameter.type)
                        {
                        case RackParam.STRING:
                            parameter.valueString = s;
                            break;
                        case RackParam.FLOAT:
                            parameter.valueFloat = Float.parseFloat(s);
                            break;
                        default: // INT32
                            parameter.valueInt32 = Integer.parseInt(s);
                        }
                        
                        popupGe.proxy.setParameter(new RackParamMsg(parameter));
                    }
                    catch(NumberFormatException nfe)
                    {
                        System.out.println(nfe);
                        JOptionPane.showMessageDialog(popupGe.navButton, "Can't set parameter:\nNumberFormatException", popupGe.name, JOptionPane.ERROR_MESSAGE);
                    }
                }
            }
        };

        jtp.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke("F12"), "F12");
        jtp.getActionMap().put("F12", new AbstractAction()
        {
            private static final long serialVersionUID = 1L;

            public void actionPerformed(ActionEvent ignored)
            {
                System.out.println("F12 pressed");
            }
        });
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
            guiElementClass = getGuiCL().loadClass(ge.guiClass);
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
                ge.frame.setVisible(false);
                ge.workspace.jdp.remove(ge.frame);
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
                    25 * (id % 10));
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
        if(ge.fx > 0)
        {
            ge.navButton.setToolTipText(ge.name + " [F" + ge.fx + "]");
        }
        else
        {
            ge.navButton.setToolTipText(ge.name);
        }
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

        if(ge.proxy != null)
        {
            ge.navButton.addMouseListener(popupListener);
            ge.navStatusButton.addMouseListener(popupListener);
        }

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
        String titleString;
        
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
                if(mainFrame != null)
                {
                	titleString = "RACK GUI";
                	for (int j = 0; j < tims.length; j++)
                	{
                	    titleString += " " + timsParam[j] + " " + tims[j].getDataRate();
                	}
                    mainFrame.setTitle(titleString);
                }
                
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
                    getStatusReplyMbx[ge.getTimsId()].send0(RackProxy.MSG_GET_STATUS, 
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

            for (int k = 0; k < getStatusReplyMbx.length; k++)
            {
                try
                {
                    while (true)
                    {
                        TimsMsg reply = getStatusReplyMbx[k].receiveIf();

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
            }

            navPanel.revalidate();

            i++;
            if(i >= elements.size())
            {
                i = 0;

                if(mainFrame != null)
                {
                	titleString = "RACK GUI";
                	for (int j = 0; j < tims.length; j++)
                	{
                	    titleString += " " + timsParam[j] + " " + tims[j].getDataRate();
                	}
                    mainFrame.setTitle(titleString);
                }
                
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
        for (int i = 0; i < tims.length; i++)
        {
        	if (tims[i] != null)
            {
                tims[i].terminate();
            }
        }

        // JOptionPane.showMessageDialog(frame,
        // "GUI Terminated",
        // "RACK GUI", JOptionPane.INFORMATION_MESSAGE);
        System.out.println("RACK GUI terminated");
    }

    //
    // Starting RACK GUI as an application
    //
    
    protected static File mainSaveConfigFile;
    protected static Gui mainGui;

    public static void main(String[] args)
    {
        System.out.println("Starting RACK GUI ...");
        
        mainSaveConfigFile = null;
        mainGui = null;

        try
        {
            // create main frame
            JFrame frame = new JFrame("Rack Gui");
            frame.addWindowListener(new WindowAdapter()
            {
                public void windowClosing(WindowEvent event)
                {
                    System.out.println("RACK GUI terminating ...");
                    
                    // write GUI configuration
                    if((mainSaveConfigFile != null) &&
                       (mainGui != null))
                    {
                        try
                        {
                            System.out.println("Writing config file ...");
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
            String      timsParam[] = new String[1];
            timsParam[0] = "";
            String      timsClass[] = new String[1];
            timsClass[0] = null;

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
                default: // config file and one or more ip/ports
                	fileName = args[0];
                    timsParam = new String[args.length-1];
                    timsClass = new String[args.length-1];
                    for (int i = 1; i < args.length; i++)
                    {
                        timsParam[i-1] = args[i];
                        timsClass[i-1] = null;
                    }
                /*case 2: // config file and ip
                    fileName = args[0];
                    timsParam = args[1];
                    break;
                default:
                    JOptionPane.showMessageDialog(frame,
                            "Invalid argument count \"" + args.length + "\". Start Gui like:\n" +
                            "'java -jar rack.jar <config-file> <ip>' or\n" +
                            "'java -classpath ... rack.gui.Gui <config-file> <ip>",
                            "RACK GUI", JOptionPane.ERROR_MESSAGE);
                    throw new Exception("Invalid argument count " + args.length);*/
            }

            try
            {
                System.out.println("Loading config file \"" + fileName + "\" ...");

                BufferedReader cfgReader = new BufferedReader(new FileReader(fileName));
                mainSaveConfigFile  = new File(fileName);

                mainGui = new Gui(frame, frame.getContentPane(), cfgReader, timsClass, timsParam);
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

	public void setGuiCL(ClassLoader guiCL) {
		this.guiCL = guiCL;
	}

	public ClassLoader getGuiCL() {
		return guiCL;
	}
}

class FxOpenGuiElementAction extends AbstractAction
{
    private static final long serialVersionUID = 1L;

    protected GuiElementDescriptor ge;
    protected Gui gui;

    FxOpenGuiElementAction(GuiElementDescriptor ge, Gui gui)
    {
        this.ge = ge;
        this.gui = gui;
    }
    
    public void actionPerformed(ActionEvent ignored)
    {
        gui.openGuiElement(ge);
        gui.relocateGuiElement(ge);
    }
};
