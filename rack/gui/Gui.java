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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
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

import rack.gui.main.RackModuleGui;
import rack.main.*;
import rack.main.tims.*;


public final class Gui extends Thread
{
    private int moduleNum;
    private int groupNum;
    private int jarfileNum;
    private int workspaceNum;

    private Vector cfgLines         = new Vector();
    private Vector moduleName       = new Vector();
    private Vector groupName        = new Vector();
    private Vector groupSize        = new Vector();
    private Vector jarFiles         = new Vector();
    private Vector workSpaceName    = new Vector();
    private Vector moduleWorkSpace  = new Vector();
    private int[]  groupSizeInt;

    // main frame
    private JFrame          mainFrame;
    private Container       mainFrameContent;
    private int[]           mainFrameLocationSize;

    // navigation panel
    private JPanel          navigationPanel, navigationInterPanel;
    private JScrollPane     navigationScrollPanel;
    // Groupe Panel
    private JPanel[]        groupPanel, groupInterPanel;
    private JButton[]       groupButton;
    // work panel
    private JTabbedPane     jtp;
    private JDesktopPane [] jdp;

    private JInternalFrame[] moduleFrame;
    private int[][]         moduleLocationSize;
    private JButton[]       moduleButton;
    private JRadioButton[]  statusButton;
    private JPanel[]        modulePanel;

    private ClassLoader     guiCL = this.getContextClassLoader();
    private Class           moduleGuiClass;

    private GDOSGui         gdosGui = null;
    private TimsMbx         gdosMbx;
    private JInternalFrame  messageFrame;
    
    private boolean         showMapView = false;
    private MapViewGui      myMapViewGui = null;
    private int             mapViewWorkSpace;
    private JInternalFrame  mapViewFrame;
    private TimsMbx         mapViewReplyMbx;

    private int[]           moduleStatus;
    private RackProxy[]     moduleProxy;
    private RackModuleGui[] moduleGui;
    private TimsMbx[]       moduleReplyMbx;

    private byte            getStatusSeqNo = 100;
    private TimsMbx         getStatusReplyMbx;

    private int[]           groupError, groupOn, groupSum;

    private TimsTcp         tims;
    private InetAddress     routerAdr = null;
    private int             routerPort = 0;
    private String          rackName = "";

    private boolean         terminate = false;

    public Gui(JFrame mainFrame, Container mainFrameContent, BufferedReader cfgReader,
               InetAddress routerAdr, int routerPort) throws Exception
    {
        this.mainFrame = mainFrame;
        this.mainFrameContent = mainFrameContent;

        try
        {
            // reading config file
            readConfig(cfgReader);

            if (moduleNum <= 0)
            {
                JOptionPane.showMessageDialog(mainFrameContent,
                        "Config file empty.\n" + 
                        "GUI config file has to contain at least one module",
                        "RACK GUI", JOptionPane.ERROR_MESSAGE);
                throw new Exception("Config file empty");
            }
    
            moduleReplyMbx     = new TimsMbx[moduleNum];
            moduleStatus       = new int[moduleNum];
            moduleProxy        = new RackProxy[moduleNum];
            moduleGui          = new RackModuleGui[moduleNum];
            moduleFrame        = new JInternalFrame[moduleNum];
            moduleButton       = new JButton[moduleNum];
            statusButton       = new JRadioButton[moduleNum];
            modulePanel        = new JPanel[moduleNum];
            moduleLocationSize = new int[moduleNum][4];
    
            // load additional jar files
            for (int i = 0; i < jarfileNum; i++)
            {
                File jarFile = new File((String)jarFiles.get(i));
                
                try {
                    URL urls[] = new URL[ ] { jarFile.toURL() };
//                    ClassLoader aCL = Thread.currentThread().getContextClassLoader();
//                    URLClassLoader aUrlCL = new URLClassLoader(urls, guiCL);
                    guiCL = new URLClassLoader(urls, guiCL);
//                    Thread.currentThread().setContextClassLoader(aUrlCL);

                     System.out.println("File " + jarFile + " has been loaded");
                }
                catch (Exception e)
                {
                    JOptionPane.showMessageDialog(mainFrameContent,
                                    "Can't load jar file \"" + jarFile + "\"",
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
                        .getMethod("initClassStringTable", (Class[])null)
                        .invoke((Object)null, (Object[])null);
                }
                catch (Exception e)
                {
                    JOptionPane.showMessageDialog(mainFrameContent,
                                    "Can't load RackName extension \"" + rackName + "\"",
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
            if(routerAdr != null)
            {
                this.routerAdr = routerAdr;
            }
            
            // constructor parameter overwrites config file
            if(routerPort > 0)
            {
                this.routerPort = routerPort;
            }

            // set default port
            if(this.routerPort <= 0)
            {
                this.routerPort = 2000;
            }

            if (this.routerAdr == null)
            {
                JOptionPane.showMessageDialog(mainFrameContent,
                        "No TimsRouterTcp address\n" + 
                        "Add param 'ROUTER_IP x.x.x.x' to your GUI config file",
                        "RACK GUI", JOptionPane.ERROR_MESSAGE);
                throw new Exception("No TimsRouterTcp address");
            }
    
            try
            {
                System.out.println("Connect to TimsRouterTcp "
                        + this.routerAdr.getHostAddress() + " port " + this.routerPort);
    
                tims = new TimsTcp(this.routerAdr, this.routerPort);
            }
            catch (TimsException e)
            {
                JOptionPane.showMessageDialog(mainFrameContent,
                        "Can't connect to TimsRouterTcp.\n" +
                        "Check service and connection to " +
                        this.routerAdr.getHostAddress() + " port " + this.routerPort,
                        "RACK GUI",
                        JOptionPane.ERROR_MESSAGE);
                throw e;
            }
    
            System.out.println("Initializing mailboxes ...");
            initMbx();
    
            System.out.println("Initializing GUI ...");
            initGui();
    
            System.out.println("Initializing Proxies ...");
            initModuleProxy();
    
            System.out.println("Initializing last stand ...");
            lastStand();
    
            System.out.println("Starting ...");
            start();

            if(mainFrame != null)
            {
                mainFrame.setTitle("RACK GUI (" + this.routerAdr.getHostAddress() + ")");
                mainFrame.setLocation(mainFrameLocationSize[0], mainFrameLocationSize[1]);
                mainFrame.setSize(mainFrameLocationSize[2], mainFrameLocationSize[3]);
                mainFrame.setVisible(true);
            }
        }
        catch(Exception e)
        {
            terminate();
            throw e;
        }
    }

    private void lastStand()
    {
        for (int i = 0; i < moduleNum; i++)
        {
            moduleLocationSize[i] = getModuleLocationSize(i);
            if (moduleLocationSize[i] != null)
            {
                openModule(i, moduleLocationSize[i]);
                // System.out.println("module[" + i + "] is open");
            }
            else
            {
                // System.out.println("module[" + i + "] is not open");
            }

        }
    }

    private void readConfig(BufferedReader configReader) throws IOException
    {
        String string;
        try
        {
            int grpElement = 0;
            int readMode = 0; // init
            String currWorkSpace = "workspace";
            workSpaceName.add(currWorkSpace);
            
            string = configReader.readLine();

            while (string != null)
            {
                string.trim();
                cfgLines.add(string);

                // Debug output
                //System.out.println(string);

                if (!string.startsWith("//") && string.length() != 0)
                {
                    if (string.startsWith("GROUP"))
                    {
                        readMode = 1;
                        groupName.add(string);
                        groupSize.add(new Integer(grpElement));
                        // the first element of this vector is always = 0;
                        grpElement = 0;
                    }
                    else if (string.startsWith("ROUTER_IP"))
                    {
                        readMode = 2;
                        string = string.substring(10);  // cut ROUTER_IP
                        if (string.length() > 0)
                        {
                            routerAdr = InetAddress.getByName(string.trim());
                        }
                    }
                    else if (string.startsWith("ROUTER_PORT"))
                    {
                        readMode = 3;
                        string = string.substring(12);  // cut ROUTER_PORT
                        if (string.length() > 0)
                        {
                            routerPort = Integer.parseInt(string.trim());
                        }
                    }
                    else if (string.startsWith("JAR_FILES"))
                    {
                        readMode = 4;
                    }
                    else if (string.startsWith("RACK_NAME"))
                    {
                        string = string.substring(10);  // cut RACK_NAME
                        if (string.length() > 0)
                        {
                            rackName = string.trim();
                        }
                    }
                    else if (string.startsWith("MAPVIEW"))
                    {
                        showMapView = true;
                        mapViewWorkSpace = workSpaceName.indexOf(currWorkSpace);
                    }
                    else if (string.startsWith("WORKSPACE"))
                    {
                    	if (moduleName.size() == 0)
                        {
                        	workSpaceName.remove(0);
                        }
                        
                    	string = string.substring(9).toLowerCase();
                        if (string.length() > 0)
                    	{
                        	if (workSpaceName.indexOf(string) < 0)
                        	{
                        		workSpaceName.add(string);
                        	}
                        	currWorkSpace = string;
                    	}
                    }
                    else // line starts without keyword
                    {
                        if (readMode == 1) // module
                        {
                            moduleName.add(string);
                            moduleWorkSpace.add(new Integer(
                            			workSpaceName.indexOf(currWorkSpace)));                            
                            grpElement++;
                        }
                        if (readMode == 4) // jar files
                        {
                            jarFiles.add(string);
                        }

                    }
                }

                // read next line
                string = configReader.readLine();
            }
            groupSize.add(new Integer(grpElement));
            groupSize.remove(0); // just to remove the first useless element
            configReader.close();
        }
        catch (IOException ioe)
        {
            JOptionPane.showMessageDialog(mainFrameContent,
                            "Error reading config file",
                            "RACK GUI", JOptionPane.ERROR_MESSAGE);
            throw ioe;
        }
        moduleNum    = moduleName.size();
        groupNum     = groupName.size();
        jarfileNum   = jarFiles.size();
        workspaceNum = workSpaceName.size();        
        // System.out.println("module_num=" + MODULE_NUM);
        groupSizeInt = new int[groupNum];
       	
        for (int i = 0; i < groupNum; i++)
        {
            groupSizeInt[i] = ((Integer) groupSize.get(i)).intValue();
        }

        System.out.println("Found " + moduleNum + " modules");
        System.out.println("Found " + groupNum + " groups");
        System.out.println("Found " + jarfileNum + " jar files");
    }

    private String getModuleName(int id)
    {
        // erstmal sehen, ob der parameter "-name=..." in der gui.cfg benutzt
        // wurde
        String name = getModuleParameter(id, "-name=");
        if (name == "")
        {
            // nein! also den Standard Namen verwenden
            name = RackName.nameString(moduleProxy[id].getCommandMbx());
        }
        return name;
    }

    private int[] getModuleLocationSize(int id)
    {
        int[] locationSize = new int[4];
        String path = (String) moduleName.elementAt(id);
        // System.out.println("module["+id+"] :"+path);
        int kAuf   = path.indexOf('(');
        int komma1 = path.indexOf(',');
        int b      = path.indexOf(';');
        int komma2 = path.indexOf(',', b);
        // int komma2 = path.lastIndexOf(',');
        int kZu = path.indexOf(')');
        // die meistens falle sind so! Deshalb ist es so effizient!
        if (path.substring(kAuf, kZu + 1).equals("(,;,)"))
        {
            return null;
        }
        else
        {

            // System.out.println(path.substring(kAuf,kZu+1)+" ;
            // "+Integer.parseInt(path.substring(b+1,komma2).trim()));
            try
            {
                // for the location
                locationSize[0] = Integer.parseInt((path.substring(kAuf + 1,
                        komma1)).trim());
                locationSize[1] = Integer.parseInt((path.substring(komma1 + 1,
                        b)).trim());

                // for the size
                locationSize[2] = Integer.parseInt((path.substring(b + 1,
                        komma2)).trim());
                locationSize[3] = Integer.parseInt((path.substring(komma2 + 1,
                        kZu)).trim());

                return locationSize;

            }
            catch (NumberFormatException nfe)
            {
                JOptionPane.showMessageDialog(mainFrameContent,
                        "Error reading module location and size.\n" + 
                        "\"" + path + "\"",
                        "RACK GUI", JOptionPane.ERROR_MESSAGE);
                return null;
            }
        }
    }

    private String getModuleParameter(int id, String param)
    {
        String path = (String) moduleName.elementAt(id);
        int a = path.indexOf(param);
        if (a < 0)
            return ""; // param not found
        a += param.length();
        char stop = ' ';
        if (path.charAt(a) == '\"')
        { // der name steht in anfuehrungszeichen
            a += 1;
            stop = '\"';
        }
        int e = path.indexOf(stop, a);
        if (e <= a)
            e = path.length();
        // System.out.println("Proxy: " + path.substring(a, e));
        return (path.substring(a, e));
    }

    /*
     * private String getModuleParameterProxy(int id) { String path = (String)
     * moduleName.elementAt(id); int a = path.indexOf("-proxy="); if (a < 0)
     * return ""; a += 7; int e = path.indexOf(" ", a); if (e <= a) e =
     * path.length(); //System.out.println("Proxy: " + path.substring(a, e));
     * return (path.substring(a, e)); } private String getModuleParameterName(int
     * id) { String path = (String) moduleName.elementAt(id); int a =
     * path.indexOf("-name="); if (a < 0) return ""; a += 6; char stop = ' '; if
     * (path.charAt(a) == '\"') { // der name steht in anfuehrungszeichen a +=
     * 1; stop = '\"'; } int e = path.indexOf(stop, a); if (e <= a) e =
     * path.length(); //System.out.println("Proxy: " + path.substring(a, e));
     * return (path.substring(a, e)); }
     */
    private boolean getModuleParameterShow(int id)
    {
        String path = (String) moduleName.elementAt(id);
        return (path.indexOf("-show") > -1);
    }

    private boolean getModuleParameterStart(int id)
    {
        String path = (String) moduleName.elementAt(id);
        return (path.indexOf("-start") > -1);
    }

    private String getGroupName(int id)
    {
        String name = null;
        name = (String) groupName.get(id);
        int blank = name.lastIndexOf(' ');
        name = name.substring(blank).toLowerCase();
        return name;
    }

    private boolean isInGroup(int groupId, int moduleId)
    {
        int count = 0;
        for (int i = 0; i < groupId; i++)
        {
            count = count + groupSizeInt[i];
        }
        if ((moduleId >= count) && (moduleId < count + groupSizeInt[groupId]))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    // alle ModuleFrame Positionen in ConfigurationFile schreiben!
    private void writeConfig(BufferedWriter cfgWriter) throws IOException
    {
        System.out.println("Write config");
        StringBuffer sb;
        for (int i = 0; i < moduleNum; i++)
        {
            sb = new StringBuffer(moduleName.get(i).toString());
            int kAuf = sb.indexOf("(");
            int kZu = sb.indexOf(")");

            if (moduleFrame[i] != null)
            {
                if (moduleFrame[i].isIcon())
                {
                    try
                    {
                        moduleFrame[i].setIcon(false);
                    }
                    catch (PropertyVetoException e) {}
                }

                if (moduleFrame[i].isMaximum())
                {
                    try
                    {
                        moduleFrame[i].setMaximum(false);
                    }
                    catch (PropertyVetoException e) {}
                }

                if (moduleLocationSize[i] == null)
                {
                    moduleLocationSize[i] = new int[4];
                }

                System.out.println("moduleFrame[" + i + "] ! = null");
                System.out.println("moduleLocationSize[" + i + "][0]"
                        + moduleFrame[i].getLocation().x);
                // System.out.println("moduleLocationSize["+i+"].toString="+moduleLocationSize[i].toString());
                // hier moduleLocationSize[i][0] NullPointException !
                moduleLocationSize[i][0] = moduleFrame[i].getLocation().x;
                moduleLocationSize[i][1] = moduleFrame[i].getLocation().y;
                moduleLocationSize[i][2] = moduleFrame[i].getSize().width;
                moduleLocationSize[i][3] = moduleFrame[i].getSize().height;

                sb = sb.replace(kAuf, kZu, "(" + moduleLocationSize[i][0]
                        + "," + moduleLocationSize[i][1] + ";"
                        + moduleLocationSize[i][2] + ","
                        + moduleLocationSize[i][3]);
            }
            else
            {
                System.out.println("moduleFrame[" + i + "] = null");
                System.out.println("test " + i);
                moduleLocationSize[i] = null;
                sb = sb.replace(kAuf, kZu, "(,;,");
            }
            for (int z = 0; z < cfgLines.size(); z++)
            {
                if (moduleName.get(i).equals(cfgLines.get(z)))
                {
                    cfgLines.set(z, sb);
                    System.out.println("moduleName:" + moduleName.get(i));
                    System.out.println("configZeilen:"
                            + cfgLines.get(z));

                }
            }

        }
        // um die mainFrameLocationSize abzuspeichen.
        if(mainFrame != null)
        {
            mainFrameLocationSize[0] = mainFrame.getLocation().x;
            mainFrameLocationSize[1] = mainFrame.getLocation().y;
            mainFrameLocationSize[2] = mainFrame.getSize().width;
            mainFrameLocationSize[3] = mainFrame.getSize().height;
        }

        String str = "//mainFrameLocationSize(" + mainFrame.getLocation().x
                     + "," + mainFrame.getLocation().y + ";" + mainFrame.getSize().width
                     + "," + mainFrame.getSize().height + ")";

        if (cfgLines.get(0).toString().startsWith(
                "//mainFrameLocationSize"))
        {
            cfgLines.set(0, str);
        }
        else
        {
            cfgLines.insertElementAt(str, 0);
        }

        for (int z = 0; z < cfgLines.size(); z++)
        {
            System.out.println(cfgLines.get(z));
            cfgWriter.write(cfgLines.get(z).toString());
            cfgWriter.newLine();
        }

        cfgWriter.close();
    }

    private RackModuleGui createModuleGui(int module)
    {
        // moduleGui werden erzeugt. Bevor moduleGui erzeugt werden, wurde
        // moduleProxy erzeugt.
        String str = null;
        String moduleGuiName = null;
        String moduleProxyName = null;
        str = (String) moduleName.get(module); // str enthaelt jetzt die
                                                // entspr. zeile der gui.cfg
                                                // -datei.
        // fuer den konstruktor ...(Proxy proxy)
        Class[] guiConstrArgsTypes = new Class[1];
        Object[] guiConstrArgs = new Object[1];
        // fuer den konstruktor ...(Integer moduleIndex, RackProxy[] proxyList, RackModuleGui[] guiList, Tims tims)
        Class[] guiConstrArgsTypes2 = new Class[4];
        Object[] guiConstrArgs2 = new Object[4];

        int blank = str.indexOf(' ');
        System.out.println("str " + str);
        moduleGuiName = str.substring(0, blank);
        System.out.println("Name 1 " + moduleGuiName);
        if (!getModuleParameterStart(module))
        {
            int dot = str.indexOf('.');
            moduleGuiName = moduleGuiName.substring(0, dot)
                            + ".gui."
                            + moduleGuiName.substring(dot + 1, moduleGuiName.length())
                            + "Gui";
        }
        System.out.println("Name 2 " + moduleGuiName);

        moduleProxyName = getModuleParameter(module, "-proxy=");
        // -proxy=??? im cfg-file?
        if (moduleProxyName == "")
            moduleProxyName = str.substring(0, blank).concat("Proxy");
        // System.out.println(moduleProxyClass.getConstructor(proxyParametersClass).newInstance(proxyParameters).toString());

        try
        {
            moduleGuiClass = guiCL.loadClass(moduleGuiName);
        }
        catch (ClassNotFoundException e1)
        {
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't load module gui.\n" +
                    "\"" + moduleGuiName + "\"",
                    "RACK GUI", JOptionPane.ERROR_MESSAGE);
            return null;
        }

        try
        {
            guiConstrArgsTypes[0] = guiCL.loadClass(moduleProxyName);
        }
        catch (ClassNotFoundException e1)
        {
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't load module proxy.\n" + 
                    "\"" + moduleProxyName + "\" class not found! \n" +
                    "Tip: Use parameter \"-proxy=...\" in config-File.",
                    "RACK GUI", JOptionPane.ERROR_MESSAGE);
            return null;
        }
        guiConstrArgs[0] = moduleProxy[module];
        // try to create RackModuleGui instance with constructor:
        // (Integer moduleIndex, RackProxy[] proxyList, RackModuleGui[] guiList, Tims tims)
        guiConstrArgs2[0] = new Integer(module);
        guiConstrArgsTypes2[0] = Integer.class;
        guiConstrArgs2[1] = moduleProxy;
        guiConstrArgsTypes2[1] = RackProxy[].class;
        guiConstrArgs2[2] = moduleGui;
        guiConstrArgsTypes2[2] = RackModuleGui[].class;
        guiConstrArgs2[3] = tims;
        guiConstrArgsTypes2[3] = Tims.class;

        try
        {
            moduleGui[module] = (RackModuleGui) guiCL.loadClass(moduleGuiName)
                                                     .getConstructor(guiConstrArgsTypes2)
                                                     .newInstance(guiConstrArgs2);
        }
        catch (Exception e)
        {
        }

        // wenn das nicht klappt, dann den aelteren standart-konstuktor nehmen
        if (moduleGui[module] == null)
        {
            try
            {
                moduleGui[module] = (RackModuleGui) moduleGuiClass.getConstructor(guiConstrArgsTypes)
                                                                  .newInstance(guiConstrArgs);
            }
            catch (IllegalArgumentException e)
            {
                e.printStackTrace();
                JOptionPane.showMessageDialog(mainFrameContent,
                        "\"" + moduleGuiName + "\" Illegal Argument!", 
                        "RACK GUI", JOptionPane.ERROR_MESSAGE);
                return null;
            }
            catch (InstantiationException e)
            {
                e.printStackTrace();
                JOptionPane.showMessageDialog(mainFrameContent,
                        "\"" + moduleGuiName + "\" Instantiation Exception!", 
                        "RACK GUI", JOptionPane.ERROR_MESSAGE);
                return null;
            }
            catch (NoSuchMethodException e)
            {
                e.printStackTrace();
                JOptionPane.showMessageDialog(mainFrameContent,
                        "\"" + moduleGuiName + "\" No Such Method Exception!", 
                        "RACK GUI", JOptionPane.ERROR_MESSAGE);
                return null;
            }
            catch (Exception e)
            {
                e.printStackTrace();
                JOptionPane.showMessageDialog(mainFrameContent,
                        "\"" + moduleGuiName + "\" getConstructor/newInstance Exception!", 
                        "RACK GUI", JOptionPane.ERROR_MESSAGE);
                return null;
            }
        }
        // dem modul den parameterstring (bzw. die ganze zeile) aus der gui.cfg
        // uebergeben
        return moduleGui[module];
    }

    private void initMbx() throws TimsException
    {
        try
        {
            // Debug output
            //System.out.println("Creating mailbox " + Integer.toHexString(getStatusReplyMbx) + " ...");
            getStatusReplyMbx = tims.mbxInit(RackName.create(RackName.GUI, 0, 0));

            for (int i = 0; i < moduleNum; i++)
            {
                // Debug output
                //System.out.println("Creating mailbox " + Integer.toHexString(replyMbx[i]) + " ...");
                moduleReplyMbx[i] = tims.mbxInit(RackName.create(RackName.GUI, 0, i+1));
            }

            // Debug output
            //System.out.println("Creating mailbox " + Integer.toHexString(GDOSMbx) + " ...");
            mapViewReplyMbx = tims.mbxInit(RackName.create(RackName.GUI, 0, moduleNum+1));

            // Debug output
            //System.out.println("Creating mailbox " + Integer.toHexString(GDOSMbx) + " ...");
            gdosMbx = tims.mbxInit(RackName.create(RackName.GDOS, 0));
        }
        catch (TimsException e)
        {
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't create Mailbox\n" + e.getMessage(),
                    "Tims Exception", JOptionPane.ERROR_MESSAGE);
            throw e;
        }
    }

    private void initModuleProxy() throws Exception
    {
        for (int i = 0; i < moduleNum; i++)
        {
            moduleProxy[i] = createModuleProxy(i);
        }
    }

    private RackProxy createModuleProxy(int module) throws Exception
    {
        // moduleProxy werden erzeugt.
        String str = null;
        String moduleProxyName = null;
        str = (String) moduleName.get(module);
        int blank = str.indexOf(' ');
        int k = str.indexOf('(');
        int id = Integer.parseInt(str.substring(blank + 1, k).trim());
        moduleProxyName = getModuleParameter(module, "-proxy=");
        // -proxy=??? im cfg-file?
        if (moduleProxyName == "")
            moduleProxyName = str.substring(0, blank).concat("Proxy");

        Class[] proxyConstrArgsTypes = new Class[]
        { int.class, TimsMbx.class };
        Object[] proxyConstrArgs = new Object[2];
        proxyConstrArgs[0] = new Integer(id);
        proxyConstrArgs[1] = moduleReplyMbx[module];

        try
        {
            moduleProxy[module] = (RackProxy) guiCL.loadClass(moduleProxyName)
                                                   .getConstructor(proxyConstrArgsTypes)
                                                   .newInstance(proxyConstrArgs);
        }
        catch (Exception e)
        {
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't load module proxy.\n" +
                    "\"" + str.substring(0, blank) + "\"",
                    "RACK GUI", JOptionPane.ERROR_MESSAGE);
            throw e;
        }
        return moduleProxy[module];
    }

    private void initGui() throws Exception
    {
        try
        {
            UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
            //UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        }
        catch (Exception e)
        {
            JOptionPane.showMessageDialog(mainFrameContent,
                    "Can't set LookAndFeel",
                    "RACK GUI", JOptionPane.ERROR_MESSAGE);
            throw e;
        }

        // create navigation panel (Module Monitor on the left border)

        navigationPanel = new JPanel();
        navigationInterPanel = new JPanel(new BorderLayout());
        navigationInterPanel.add(BorderLayout.NORTH, navigationPanel);

        navigationScrollPanel = new JScrollPane(navigationInterPanel);
        navigationScrollPanel
                .setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        navigationScrollPanel
                .setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
        navigationScrollPanel.setPreferredSize(new Dimension(160, 10));

        // create groupsPanel
        groupButton = new JButton[groupNum];
        groupPanel = new JPanel[groupNum];
        groupInterPanel = new JPanel[groupNum];
        for (int i = 0; i < groupNum; i++)
        {
            groupButton[i] = new JButton(getGroupName(i));
            groupPanel[i] = new JPanel();
            groupInterPanel[i] = new JPanel();

            groupButton[i].addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent ae)
                {
                    Object o = ae.getSource();
                    int groupIndex = -1;
                    for (int j = 0; j < groupNum; j++)
                    {
                        if (o == groupButton[j])
                        {
                            groupIndex = j;
                        }
                    }
                    if (groupPanel[groupIndex].isShowing())
                    {
                        groupPanel[groupIndex].setVisible(false);
                    }
                    else
                    {
                        groupPanel[groupIndex].setVisible(true);
                    }
                }
            });
            groupButton[i].setHorizontalAlignment(SwingConstants.LEFT);

            groupPanel[i] = new JPanel(new GridLayout(0, 1));
            groupPanel[i].setBorder(BorderFactory
                    .createEmptyBorder(0, 10, 0, 0));

            groupInterPanel[i] = new JPanel(new BorderLayout());
            groupInterPanel[i].add(groupButton[i], BorderLayout.NORTH);
            groupInterPanel[i].add(groupPanel[i], BorderLayout.CENTER);

            navigationPanel.setLayout(new BoxLayout(navigationPanel,
                    BoxLayout.Y_AXIS));
            navigationPanel.add(groupInterPanel[i]);
        }

        JButton allOffButton = new JButton("all off");
        allOffButton.setHorizontalAlignment(SwingConstants.LEFT);
        allOffButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent ae)
            {
                for (int i = moduleNum - 1; i >= 0; i--)
                {

                    moduleProxy[i].off();

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
        navigationPanel.add(allOffInterPanel);

        mainFrameContent.add(navigationScrollPanel, BorderLayout.WEST);

        // create work panel (Tabbed Panel in the center)
        jtp = new JTabbedPane();
        jdp = new JDesktopPane[workspaceNum];
        for (int i = 0; i < workspaceNum; i++)
        {
        	jdp[i] = new JDesktopPane();
         	jtp.add((String)workSpaceName.get(i),jdp[i]);
        }
        mainFrameContent.add(jtp, BorderLayout.CENTER);

        // create message frame as an internal frame
        messageFrame = new JInternalFrame("GDOS Message", true, false, true, true);
        gdosGui = new GDOSGui(gdosMbx);
        messageFrame.getContentPane().add(gdosGui.getComponent());
        messageFrame.pack();
        messageFrame.setVisible(true);
        jdp[0].add(messageFrame);
        gdosGui.start();

        if (showMapView)
        {
            //create mapView panel as a tab
        	mapViewFrame = new JInternalFrame("MapView", true, false, true, true);
            myMapViewGui = new MapViewGui(moduleGui, mapViewReplyMbx);
        	mapViewFrame.getContentPane().add(myMapViewGui.getComponent());
        	mapViewFrame.pack();        	
        	mapViewFrame.setVisible(true);
        	jdp[mapViewWorkSpace].add(mapViewFrame);
        	mapViewFrame.setLocation(0, 0);
        	mapViewFrame.setSize(600, 400);
        }
        
        mainFrameLocationSize = getMainFrameLocationSize(cfgLines);
        if (mainFrameLocationSize == null)
        {
            mainFrameLocationSize = new int[4];
            mainFrameLocationSize[0] = 0;
            mainFrameLocationSize[1] = 0;
            mainFrameLocationSize[2] = 800;
            mainFrameLocationSize[3] = 600;
        }

        mainFrameContent.setVisible(true);
    }

    private int[] getMainFrameLocationSize(Vector configZeilen)
    {
        int[] locationSize = new int[4];
        String path = (String) configZeilen.get(0);
        int kAuf = path.indexOf('(');
        int komma1 = path.indexOf(',');
        int b = path.indexOf(';');
        int komma2 = path.indexOf(',', b);
        // int komma2 = path.lastIndexOf(',');
        int kZu = path.indexOf(')');

        if (kAuf < 0 && komma1 < 0 && b < 0 && komma2 < 0)
        {
            return null;
        }
        else if (path.substring(kAuf, kZu + 1).equals("(,;,)"))
        {
            return null;
        }
        else
        {
            // System.out.println(path.substring(kAuf,kZu+1)+" ;
            // "+Integer.parseInt(path.substring(b+1,komma2).trim()));
            try
            {
                // for the location
                locationSize[0] = Integer.parseInt((path.substring(kAuf + 1,
                        komma1)).trim());
                locationSize[1] = Integer.parseInt((path.substring(komma1 + 1,
                        b)).trim());

                // for the size
                locationSize[2] = Integer.parseInt((path.substring(b + 1,
                        komma2)).trim());
                locationSize[3] = Integer.parseInt((path.substring(komma2 + 1,
                        kZu)).trim());
                if (locationSize[2] < 0 || locationSize[3] < 0)
                {
                    return null;
                }
                else
                {
                    return locationSize;
                }
            }
            catch (Exception e)
            {
                e.toString();
                return null;
            }
        }
    }

    private void initModule(int id)
    {
        modulePanel[id] = new JPanel(new BorderLayout());

        statusButton[id] = new JRadioButton();
        statusButton[id].setActionCommand(Integer.toString(id));
        statusButton[id].addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent ae)
            {
                try
                {
                    int module = Integer.parseInt(ae.getActionCommand());

                    if (statusButton[module].isSelected() == true)
                    {
                        moduleProxy[module].on();
                    }
                    else
                    {
                        moduleProxy[module].off();
                    }
                }
                catch (NumberFormatException e)
                {
                }
            }
        });

        // if (moduleGui[id] == null)
        moduleButton[id] = new JButton(getModuleName(id));
        moduleButton[id].setToolTipText(getModuleName(id));
        // else
        // moduleButton[id] = new JButton(moduleGui[id].getModuleName());
        moduleButton[id].setHorizontalAlignment(SwingConstants.LEFT);
        moduleButton[id].setActionCommand(Integer.toString(id));
        moduleButton[id].addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent ae)
            {
                try
                {
                    int module = Integer.parseInt(ae.getActionCommand());

                    if ((ae.getModifiers() & ActionEvent.CTRL_MASK) == 0)
                    {
                        openModule(module, null);
                        relocateModule(module);
                    }
                    else
                    {
                        if (moduleStatus[module] == RackProxy.MSG_DISABLED)
                        {
                            moduleProxy[module].on();
                        }
                        else
                        {
                            moduleProxy[module].off();
                        }
                    }
                }
                catch (NumberFormatException e)
                {
                }
            }
        });

        modulePanel[id].add(BorderLayout.WEST, statusButton[id]);
        modulePanel[id].add(BorderLayout.CENTER, moduleButton[id]);
        // System.out.println("modulelePanel[" + id + "] is ok");

        for (int i = 0; i < groupNum; i++)
        {
            if (isInGroup(i, id))
            {
                groupPanel[i].add(modulePanel[id]);
                // System.out.println(
                // "modulelePanel[" + i + "] is in group[" + i + "]");
                break;
            }
        }

    }

    private void openModule(int id, int[] moduleLocationSize)
    {
        if (moduleGui[id] == null)
        {

            moduleGui[id] = createModuleGui(id); // gleichzeitig wird
                                                    // moduleProxy[id] erzeugt .
            moduleGui[id].start();
            moduleFrame[id] = new JInternalFrame(getModuleName(id), true, true,
                    true, true);

            moduleFrame[id].getContentPane().add(
                    ((RackModuleGui) moduleGui[id]).getComponent());
            moduleFrame[id].addInternalFrameListener(new InternalFrameAdapter()
            {
                public void internalFrameClosing(InternalFrameEvent e)
                {
                    Object o = e.getSource();
                    int module = -1;
                    for (int j = 0; j < moduleNum; j++)
                    {
                        if (o == moduleFrame[j])
                        {
                            module = j;
                        }
                    }
                    if (moduleGui[module] != null)
                    {
                        ((RackModuleGui) moduleGui[module]).terminate();
                        moduleGui[module] = null;
                    }
                    moduleFrame[module] = null;

                    // System.out.println("Internal frame is closing");
                }
            });
            moduleFrame[id].pack();
            moduleFrame[id].setVisible(true);
            if (moduleLocationSize == null)
            {
                moduleFrame[id].setLocation(100 * (id / 10) + 20 * (id % 10),
                        25 + 25 * (id % 10));
            }
            else
            {
                moduleFrame[id].setLocation(moduleLocationSize[0],
                        moduleLocationSize[1]);
                moduleFrame[id].setSize(moduleLocationSize[2],
                        moduleLocationSize[3]);
            }
            jdp[((Integer)moduleWorkSpace.get(id)).intValue()].add(moduleFrame[id]);
        }

        try
        {
            moduleFrame[id].moveToFront();
            moduleFrame[id].setSelected(true);
            jtp.setSelectedIndex(((Integer)moduleWorkSpace.get(id)).intValue());
        }
        catch (java.beans.PropertyVetoException pe)
        {
            pe.printStackTrace();
        }
    }

    private void relocateModule(int id)
    {
        Point pos = moduleFrame[id].getLocation();
        Dimension size = moduleFrame[id].getSize();
        Dimension paneSize = moduleFrame[id].getDesktopPane().getSize();

        if (pos.x < 0)
            pos.x = 0;
        else if (pos.x + size.width > paneSize.width)
            pos.x = paneSize.width - size.width;

        if (pos.y < 0)
            pos.y = 0;
        else if (pos.y + size.height > paneSize.height)
            pos.y = paneSize.height - size.height;

        moduleFrame[id].setLocation(pos);
    }

    private void updateAllStatus()
    {
        groupError = new int[groupNum];
        groupOn = new int[groupNum];
        groupSum = new int[groupNum];

        for (int i = 0; i < groupNum; i++)
        {
            groupOn[i] = 0;
            groupError[i] = 0;
            groupSum[i] = 0;
        }
        for (int i = 0; i < moduleNum; i++)
        {
            moduleStatus[i] = RackProxy.MSG_TIMEOUT;
        }

        try
        {
            if (getStatusSeqNo < 127)
                getStatusSeqNo++;
            else
                getStatusSeqNo = -128;

            for (int i = 0; i < moduleNum; i++)
            {
                // alle moduleProxy sind bei initModuleProxy schon vorhand.

                getStatusReplyMbx.send0(RackProxy.MSG_GET_STATUS, 
                        moduleProxy[i].getCommandMbx(),
                        (byte) 0,
                        (byte) getStatusSeqNo);
                // ein bischen warten, um nicht stossweise last zu erzeugen.
                try
                {
                    Thread.sleep(10);
                }
                catch (InterruptedException e)
                {
                }
            }

            TimsDataMsg reply;
            boolean notAllReplies = true;

            // wait for all replies or timeout
            while (notAllReplies)
            {
                // receive reply
                reply = getStatusReplyMbx.receive(1000);
                if (reply.seqNr == getStatusSeqNo)
                {
                    // update module status array
                    for (int i = 0; i < moduleNum; i++)
                    {

                        if (moduleProxy[i].getCommandMbx() == reply.src)
                        {
                            moduleStatus[i] = reply.type;
                        }
                    }
                }

                // test if all replies are received
                notAllReplies = false;
                for (int i = 0; i < moduleNum; i++)
                {
                    if (moduleStatus[i] == RackProxy.MSG_TIMEOUT)
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

        for (int i = 0; i < moduleNum; i++)
        {
            // bei paramentern -show und -start soll trotzdem
            // der button angezeigt werden.
            if (getModuleParameterShow(i)
                    && moduleStatus[i] == RackProxy.MSG_NOT_AVAILABLE)
                moduleStatus[i] = RackProxy.MSG_DISABLED;
            if (getModuleParameterStart(i)
                    && moduleStatus[i] == RackProxy.MSG_NOT_AVAILABLE)
                moduleStatus[i] = RackProxy.MSG_DISABLED;

            switch (moduleStatus[i])
            {
                case RackProxy.MSG_ERROR:
                    if (modulePanel[i] == null)
                    {
                        initModule(i);
                    }
                    statusButton[i].setEnabled(true);
                    statusButton[i].setSelected(true);
                    statusButton[i].setForeground(Color.red);
                    moduleButton[i].setEnabled(true);

                    for (int id = 0; id < groupNum; id++)
                    {
                        if (isInGroup(id, i))
                        {
                            groupSum[id]++;
                            groupError[id]++;
                            // System.out.println(
                            // "groupSum[" + id + "]" + "=" + groupSum[id]);
                        }
                    }
                    break;

                case RackProxy.MSG_ENABLED:
                    if (modulePanel[i] == null)
                    {
                        initModule(i);
                    }
                    statusButton[i].setEnabled(true);
                    statusButton[i].setSelected(true);
                    statusButton[i].setForeground(Color.green);
                    moduleButton[i].setEnabled(true);

                    for (int id = 0; id < groupNum; id++)
                    {
                        if (isInGroup(id, i))
                        {
                            groupSum[id]++;
                            groupOn[id]++;
                            // System.out.println(
                            // "groupSum[" + id + "]" + "=" + groupSum[id]);
                        }
                    }

                    break;
                case RackProxy.MSG_DISABLED:
                    if (modulePanel[i] == null)
                    {
                        initModule(i);
                    }
                    statusButton[i].setEnabled(true);
                    statusButton[i].setSelected(false);
                    moduleButton[i].setEnabled(true);

                    for (int id = 0; id < groupNum; id++)
                    {
                        if (isInGroup(id, i))
                        {
                            groupSum[id]++;
                            // System.out.println(
                            // "groupSum[" + id + "]" + "=" + groupSum[id]);
                        }
                    }

                    break;

                case RackProxy.MSG_NOT_AVAILABLE:
                    if (modulePanel[i] != null)
                    {
                        statusButton[i].setEnabled(false);
                        statusButton[i].setSelected(false);
                        moduleButton[i].setEnabled(false);
                    }
                    break;
                case RackProxy.MSG_TIMEOUT:
                default:
                    if (modulePanel[i] != null)
                    {
                        statusButton[i].setEnabled(true);
                        statusButton[i].setSelected(true);
                        statusButton[i].setForeground(Color.orange);
                        moduleButton[i].setEnabled(false);
                    }
            }
        }
    }

    public void run()
    {
        while (terminate == false)
        {
            updateAllStatus();
            for (int i = 0; i < groupSize.size(); i++)
            {
                if (groupError[i] > 0)
                {
                    groupButton[i].setForeground(Color.red);
                }
                else
                {
                    groupButton[i].setForeground(groupButton[i].getParent()
                            .getForeground());
                }
                groupButton[i].setText(getGroupName(i) + " ("
                        + groupError[i] + " , " + groupOn[i] + " , "
                        + groupSum[i] + ")");
            }
            navigationPanel.revalidate();
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
            this.join(100);
        }
        catch (Exception e) {}
        
        // terminate MapViewGui
        if(myMapViewGui != null)
        {
            myMapViewGui.terminate();
        }

        // terminate module guis
        for (int i = 0; i < moduleNum; i++)
        {
            if(moduleGui[i] != null)
            {
                moduleGui[i].terminate();
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

//        JOptionPane.showMessageDialog(frame,
//                "GUI Terminated",
//                "RACK GUI", JOptionPane.INFORMATION_MESSAGE);
        System.out.println("GUI terminated");
    }

    //
    // Starting RACK GUI as an application
    //
    
    private static File mainSaveConfigFile;
    private static Gui mainGui;

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
                            mainGui.writeConfig(new BufferedWriter(new FileWriter(mainSaveConfigFile)));
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
            InetAddress routerAdr   = null;
            int         routerPort  = 0;

            try
            {
                switch (args.length)
                {
                    case 0:
                        FileDialog fd = new FileDialog(frame, "RACK GUI config", FileDialog.LOAD);
                        fd.setFile("*.cfg");
                        fd.setDirectory(".");
                        fd.setLocation(150, 150);
                        fd.setVisible(true);
                        fileName = fd.getDirectory() + System.getProperty("file.separator").charAt(0) + fd.getFile();
                        break;
                    case 1: // only config file
                        fileName = args[0];
                        break;
                    case 2: // config file and ip
                        fileName = args[0];
                        routerAdr = InetAddress.getByName(args[1]);
                        break;
                    case 3: // config file, ip and port
                        fileName = args[0];
                        routerAdr = InetAddress.getByName(args[1]);
                        routerPort = Integer.parseInt(args[2]);
                        break;
                    default:
                        JOptionPane.showMessageDialog(frame,
                                "Invalid argument count \"" + args.length + "\". Start Gui like:\n" +
                                "'javaw -jar rack.jar <config-file>' or\n" +
                                "'java -classpath ... rack.gui.Gui config-file ip <port>",
                                "RACK GUI", JOptionPane.ERROR_MESSAGE);
                        throw new Exception("Invalid argument count " + args.length);
                }
            }
            catch(UnknownHostException e)
            {
                JOptionPane.showMessageDialog(frame,
                        "Invalid router ip \"" + args[1] + "\". Start Gui like:\n" +
                        "'javaw -jar rack.jar <config-file>' or\n" +
                        "'java -classpath ... rack.gui.Gui config-file ip <port>",
                        "RACK GUI", JOptionPane.ERROR_MESSAGE);
                throw e;
            }
            catch(NumberFormatException e)
            {
                JOptionPane.showMessageDialog(frame,
                        "Invalid router port \"" + args[2] + "\". Start Gui like:\n" +
                        "'javaw -jar rack.jar <config-file>' or\n" +
                        "'java -classpath ... rack.gui.Gui config-file ip <port>",
                        "RACK GUI", JOptionPane.ERROR_MESSAGE);
                throw e;
            }

            try
            {
                System.out.println("Load config file \"" + fileName + "\"");

                BufferedReader cfgReader = new BufferedReader(new FileReader(fileName));
                mainSaveConfigFile  = new File(fileName);

                mainGui = new Gui(frame, frame.getContentPane(), cfgReader, routerAdr, routerPort);
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
