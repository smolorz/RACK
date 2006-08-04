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
 *      Artur Wiebe
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.gui;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.*;
import java.net.*;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.net.InetAddress;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JDesktopPane;
import javax.swing.JFrame;
import javax.swing.JInternalFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.SwingConstants;
import javax.swing.UIManager;
import javax.swing.event.InternalFrameAdapter;
import javax.swing.event.InternalFrameEvent;

import rack.gui.main.RackModuleGui;
import rack.main.naming.*;
import rack.main.proxy.*;
import rack.main.tims.Tims;
import rack.main.tims.TimsTcp;
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;


/**
 * Java Gui zum Rack
 *
 * @version $Id: Gui.java,v 1.72 2005/10/24 16:19:43 wiebe Exp $
 */

public class Gui extends Thread
{
    public static int MODULE_NUM;
    public static int GROUP_NUM;
    public static int JARFILE_NUM;
    public static int DRIVERS_NUM;
    public static int WORKSPACE_NUM;

    /** enable / disable module state polling */
    private boolean prefPollingOn = true;

    Vector configZeilen = new Vector();
    Vector moduleName = new Vector();
    Vector groupName = new Vector();
    Vector groupSize = new Vector();
    Vector jarFiles = new Vector();
    Vector workSpaceName = new Vector();
    Vector workSpaceSize = new Vector();
    Vector moduleWorkSpace = new Vector();    
    int[] groupSizeInt;

    ClassLoader guiCL = this.getContextClassLoader();
    Class moduleGuiClass;
    Class moduleProxyClass;

    // menu-stuff
    JMenuBar menuBar;
    JMenu preferencesMenu;
    JCheckBoxMenuItem prefPollingOnCbMenuItem;

    boolean showMapView = false;
    int mapViewWorkSpace;

    // main frame
    JFrame frame;

    // navigation panel
    JPanel navigationPanel, navigationInterPanel;
    JScrollPane navigationScrollPanel;
    // Groupe Panel
    JPanel[] groupPanel, groupInterPanel;
    JButton[] groupButton;
    // work panel
    JTabbedPane jtp;
//    JDesktopPane jdp;
//    JDesktopPane jdp2;
    JDesktopPane [] jdp;
    // JDesktopPane jdp3;
    // message internal frame
    JInternalFrame messageFrame;
    // mapViewFrame
    JInternalFrame mapViewFrame;
    // moduleFrame fr alle Module
    JInternalFrame[] moduleFrame;
    Point[] moduleLocation;
    int[] mainFrameLocationSize;
    int[][] moduleLocationSize;
    JButton[] moduleButton;
    JRadioButton[] statusButton;
    JPanel[] modulePanel;

    int[] replyMbx;
    int GDOSMbx;

    int[] moduleStatus;
    RackProxy[] moduleProxy;
    RackModuleGui[] moduleGui;

    byte getStatusSeqNo = 100;
    int getStatusReplyMbx;

    int[] groupError, groupOn, groupSum;
    public static boolean terminate = false;

    String fileName = "";

    InetAddress gwAddress;
    int gwPort;
    int  i;

    public Gui(String[] args)
    {
        gwAddress = null;
        gwPort = 0;

        switch (args.length)
        {
            case 0:
                fileName = loadFile(new Frame(), "Open ...", "./rack/gui", "*.cfg");
                break;
            case 1: // only config file
                fileName = args[0];
                break;
            default:
                JOptionPane.showMessageDialog(frame,
                        "Starting Gui like 'java -jar rack.jar <config-file>'.",
                        "Notice", JOptionPane.ERROR_MESSAGE);
                System.exit(0);
        }

        // reading config file

        System.out.println("Load config file \"" + fileName + "\"");
        readConfig(fileName);

        // checks

        if (gwAddress != null)
        {
            System.out.println("Gateway router address : " + gwAddress.toString());
        }
        else
        {
            JOptionPane.showMessageDialog(frame,
                    "Add param 'ROUTER_IP W.X.Y.Z' to your GUI config file " + fileName,
                    "Notice", JOptionPane.ERROR_MESSAGE);
            System.exit(-1);
        }

        if (gwPort != 0)
        {
            System.out.println("Gateway router port    : " + gwPort);
        }
        else
        {
            JOptionPane.showMessageDialog(frame,
                    "Add param 'ROUTER_PORT XYZ' to your GUI config file " + fileName,
                    "Notice", JOptionPane.ERROR_MESSAGE);
            System.exit(-1);
        }

        System.out.println("Found " + MODULE_NUM + " modules");
        System.out.println("Found " + GROUP_NUM + " groups");
        System.out.println("Found " + JARFILE_NUM + " jar files");

        // load additional jar files
          for (i=0; i< JARFILE_NUM; i++)
          {
               File jf = new File((String) jarFiles.get(i));
               if (jf.exists() == true)
               {
                   try {
                       addFile(jf);
                       System.out.println("File " + (String) jarFiles.get(i) + " has been loaded");
                   }
                   catch (IOException ioe)
                   {
                       JOptionPane.showMessageDialog(
                                       frame,
                                       "The given jar file " + jf.getName() + " could not be found.",
                                       "I/O error", JOptionPane.ERROR_MESSAGE);
                       System.exit(0);
                   }
               }
               else
             {
                   JOptionPane.showMessageDialog(
                           frame,
                           "The given jar file " + jf.getName() + " could not be found.",
                           "I/O error", JOptionPane.ERROR_MESSAGE);
                   System.exit(-1);
            }
        }

        // connect to router ...

        try
        {
            System.out.println("Connect to TcpTimsMsgGateway "
                    + gwAddress.getHostAddress() + ":" + gwPort);

            new TimsTcp(gwAddress, gwPort);
        }
        catch (MsgException e)
        {
            JOptionPane.showMessageDialog(frame,
                    "Cannot connect to TimsMsgGateway. \n"
                            + "Check service and connection to "
                            + gwAddress.getHostAddress()
                            + ".", "Error starting Gui",
                    JOptionPane.ERROR_MESSAGE);
            System.exit(-1);
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
    }

    public String loadFile(Frame f, String title, String defDir, String fileType)
    {
        FileDialog fd = new FileDialog(f, title, FileDialog.LOAD);
        fd.setFile(fileType);
        fd.setDirectory(defDir);
        fd.setLocation(150, 150);
        fd.setVisible(true);
        return fd.getDirectory() + System.getProperty("file.separator").charAt(0) +
               fd.getFile();
    }

    public void addFile(String s) throws IOException
    {
        File f = new File(s);
        addFile(f);
    }

    public void addFile(File f) throws IOException
    {
        addURL(f.toURL());
    }

    public void addURL(URL u) throws IOException
    {
        URL urls[] = new URL[ ]{ u };
//      ClassLoader aCL = Thread.currentThread().getContextClassLoader();
//      URLClassLoader aUrlCL = new URLClassLoader(urls, guiCL);
        guiCL = new URLClassLoader(urls, guiCL);
//      Thread.currentThread().setContextClassLoader(aUrlCL);
    }

    public void lastStand()
    {
        for (int i = 0; i < MODULE_NUM; i++)
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

    public void readConfig(String fileName)
    {
        BufferedReader bfr;
        String string;
        try
        {
            bfr = new BufferedReader(new FileReader(fileName));
            string = bfr.readLine().trim();
            int grpElement = 0;
            int readMode = 0; // init
            String currWorkSpace = "workspace";
            workSpaceName.add(currWorkSpace);
            
            
            while (string != null)
            {
                configZeilen.add(string);

                System.out.println(string);

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
                            gwAddress = InetAddress.getByName(string.trim());
                        }
                    }
                    else if (string.startsWith("ROUTER_PORT"))
                    {
                        readMode = 3;
                        string = string.substring(12);  // cut ROUTER_PORT
                        if (string.length() > 0)
                        {
                            gwPort = Integer.parseInt(string.trim());
                        }
                    }
                    else if (string.startsWith("JAR_FILES"))
                    {
                        readMode = 4;
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
                string = bfr.readLine();
            }
            groupSize.add(new Integer(grpElement));
            groupSize.remove(0); // just to remove the first useless element
            bfr.close();
        }
        catch (IOException ioe)
        {
            JOptionPane
                    .showMessageDialog(
                            frame,
                            "Die angegebene Konfigurationsdatei konnte nicht gelesen werden.",
                            "E/A-Fehler", JOptionPane.ERROR_MESSAGE);
            System.exit(0);
        }
        MODULE_NUM    = moduleName.size();
        GROUP_NUM     = groupName.size();
        JARFILE_NUM   = jarFiles.size();
        WORKSPACE_NUM = workSpaceName.size();        
        // System.out.println("module_num=" + MODULE_NUM);
        groupSizeInt = new int[GROUP_NUM];
       	
        for (int i = 0; i < GROUP_NUM; i++)
        {
            groupSizeInt[i] = ((Integer) groupSize.get(i)).intValue();
        }
    }

    public String getModuleName(int id)
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

    public int[] getModuleLocationSize(int id)
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
                // if: for the situation "( , ; ,)" whitespace in the clama;
                // System.out.println(nfe.getMessage()+"="+"For input
                // string:\"\"");
                if (!(nfe.getMessage().equals("For input string: \"\"")))
                {

                    System.out.println(nfe.getMessage());
                    JOptionPane.showMessageDialog(frame, path
                            .concat("NumberFormatException: ")
                            + nfe.getMessage(), "cfg-Fehler",
                            JOptionPane.ERROR_MESSAGE);
                }
                return null;

            }
            catch (Exception e)
            {

                return null;
            }
        }
    }

    public String getModuleParameter(int id, String param)
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
     * public String getModuleParameterProxy(int id) { String path = (String)
     * moduleName.elementAt(id); int a = path.indexOf("-proxy="); if (a < 0)
     * return ""; a += 7; int e = path.indexOf(" ", a); if (e <= a) e =
     * path.length(); //System.out.println("Proxy: " + path.substring(a, e));
     * return (path.substring(a, e)); } public String getModuleParameterName(int
     * id) { String path = (String) moduleName.elementAt(id); int a =
     * path.indexOf("-name="); if (a < 0) return ""; a += 6; char stop = ' '; if
     * (path.charAt(a) == '\"') { // der name steht in anfuehrungszeichen a +=
     * 1; stop = '\"'; } int e = path.indexOf(stop, a); if (e <= a) e =
     * path.length(); //System.out.println("Proxy: " + path.substring(a, e));
     * return (path.substring(a, e)); }
     */
    public boolean getModuleParameterShow(int id)
    {
        String path = (String) moduleName.elementAt(id);
        return (path.indexOf("-show") > -1);
    }

    public boolean getModuleParameterStart(int id)
    {
        String path = (String) moduleName.elementAt(id);
        return (path.indexOf("-start") > -1);
    }

    public String getGroupName(int id)
    {
        String name = null;
        name = (String) groupName.get(id);
        int blank = name.lastIndexOf(' ');
        name = name.substring(blank).toLowerCase();
        return name;
    }

    public boolean isInGroup(int groupId, int moduleId)
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
    void writeConfig(String cfg)
    {
        System.out.println("fange an config zu schreiben!");
        BufferedWriter bw;
        StringBuffer sb;
        try
        {
            bw = new BufferedWriter(new FileWriter(cfg));

            for (int i = 0; i < MODULE_NUM; i++)
            {
                sb = new StringBuffer(moduleName.get(i).toString());
                int kAuf = sb.indexOf("(");
                int kZu = sb.indexOf(")");

                if (moduleFrame[i] != null)
                {
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

                    if (moduleFrame[i].isIcon())
                    {
                        moduleLocationSize[i][2] = moduleFrame[i]
                                .getMinimumSize().width;
                        moduleLocationSize[i][3] = moduleFrame[i]
                                .getMinimumSize().height;
                    }
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
                for (int z = 0; z < configZeilen.size(); z++)
                {
                    if (moduleName.get(i).equals(configZeilen.get(z)))
                    {
                        configZeilen.set(z, sb);
                        System.out.println("moduleName:" + moduleName.get(i));
                        System.out.println("configZeilen:"
                                + configZeilen.get(z));

                    }
                }

            }
            // um die mainFrameLocationSize abzuspeichen.
            String str = "//mainFrameLocationSize(" + frame.getLocation().x
                    + "," + frame.getLocation().y + ";" + frame.getSize().width
                    + "," + frame.getSize().height + ")";

            if (configZeilen.get(0).toString().startsWith(
                    "//mainFrameLocationSize"))
            {
                configZeilen.set(0, str);
            }
            else
            {
                configZeilen.insertElementAt(str, 0);
            }

            for (int z = 0; z < configZeilen.size(); z++)
            {
                System.out.println(configZeilen.get(z));
                bw.write(configZeilen.get(z).toString());
                bw.newLine();
            }

            bw.close();
        }
        catch (Exception e)
        {
            System.out.println(e.getClass().toString() + "  bei writeConfig");
        }
    }

    public RackProxy createModuleProxy(int module)
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
        { int.class, int.class };
        Object[] proxyConstrArgs = new Object[2];
        proxyConstrArgs[0] = new Integer(id);
        proxyConstrArgs[1] = new Integer(replyMbx[module]);

        try
        {
            moduleProxy[module] = (RackProxy) guiCL.loadClass(moduleProxyName)
                                                   .getConstructor(proxyConstrArgsTypes)
                                                   .newInstance(proxyConstrArgs);
        }
        catch (Exception e)
        {
            JOptionPane.showMessageDialog(frame, "\"" + str.substring(0, blank)
                    + "\" ist fehlerhaft", "configuration-error",
                    JOptionPane.ERROR_MESSAGE);
            System.exit(-1);
        }
        return (moduleProxy[module]);
    }

    public RackModuleGui createModuleGui(int module)
    {
        // moduleGui werden erzeugt. Bevor moduleGui erzeugt werden, wurde
        // moduleProxy erzeugt.
        String str = null;
        String moduleGuiName = null;
        String moduleProxyName = null;
        str = (String) moduleName.get(module); // str enthaelt jetzt die
                                                // entspr. zeile der gui.cfg
                                                // -datei.
        // fuer den konstruktor ...(Proxy proxysowiso)
        Class[] guiConstrArgsTypes = new Class[1];
        Object[] guiConstrArgs = new Object[1];
        // fuer den konstruktor ...(Gui mainJavaGui, Integer module-id, Proxy
        // proxysowiso)
        Class[] guiConstrArgsTypes2 = new Class[3];
        Object[] guiConstrArgs2 = new Object[3];

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
            guiConstrArgsTypes[0] = guiCL.loadClass(moduleProxyName);
        }
        catch (ClassNotFoundException e1)
        {
            errorDialog("\""
                    + moduleProxyName
                    + "\" class not found! \n Tip: Use parameter \"-proxy=...\" in config-File.");
            return null;
        }
        guiConstrArgs[0] = moduleProxy[module];
        try
        {
            moduleGuiClass = guiCL.loadClass(moduleGuiName);
        }
        catch (ClassNotFoundException e1)
        {
            errorDialog("\"" + moduleGuiName + "\" class not found!");
            return null;
        }
        // try to create RackModuleGui instance with constructor:
        // (Integer moduleIndex, RackProxy[] proxyList, RackModuleGui[] guiList)
        guiConstrArgs2[0] = new Integer(module);
        guiConstrArgsTypes2[0] = Integer.class;
        guiConstrArgs2[1] = moduleProxy;
        guiConstrArgsTypes2[1] = RackProxy[].class;
        guiConstrArgs2[2] = moduleGui;
        guiConstrArgsTypes2[2] = RackModuleGui[].class;

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
                errorDialog("\"" + moduleGuiName + "\" Illegal Argument!");
                e.printStackTrace();
                return null;
            }
            catch (InstantiationException e)
            {
                errorDialog("\"" + moduleGuiName
                        + "\" Instantiation Exception!");
                e.printStackTrace();
                return null;
            }
            catch (NoSuchMethodException e)
            {
                errorDialog("\"" + moduleGuiName
                        + "\" No Such Method Exception!");
                e.printStackTrace();
                return null;
            }
            catch (Exception e)
            {
                errorDialog("\"" + moduleGuiName
                        + "\" getConstructor/newInstance Exception!");
                e.printStackTrace();
                return null;
            }
        }
        // dem modul den parameterstring (bzw. die ganze zeile) aus der gui.cfg
        // uebergeben
        return moduleGui[module];
    }

    protected void errorDialog(String msg)
    {
        JOptionPane.showMessageDialog(frame, msg, "configuration-error",
                JOptionPane.ERROR_MESSAGE);
    }

    public void initMbx()
    {
        replyMbx           = new int[MODULE_NUM];
        moduleStatus       = new int[MODULE_NUM];
        moduleProxy        = new RackProxy[MODULE_NUM];
        moduleGui          = new RackModuleGui[MODULE_NUM];
        moduleFrame        = new JInternalFrame[MODULE_NUM];
        moduleLocation     = new Point[MODULE_NUM];
        moduleButton       = new JButton[MODULE_NUM];
        statusButton       = new JRadioButton[MODULE_NUM];
        modulePanel        = new JPanel[MODULE_NUM];
        moduleLocationSize = new int[MODULE_NUM][4];

        try
        {
            for (int i = 0; i < MODULE_NUM; i++)
            {
                replyMbx[i] = RackName.create(RackName.GUI, 0, i);
                System.out.println("Creating mailbox " + replyMbx[i] + " ...");
                Tims.mbxInit(replyMbx[i]);
            }

            getStatusReplyMbx = RackName.create(RackName.GUI, 0, MODULE_NUM);
            System.out.println("Creating mailbox " + getStatusReplyMbx + " ...");
            Tims.mbxInit(getStatusReplyMbx);

            GDOSMbx = RackName.create(RackName.GDOS, 0);
            System.out.println("Creating mailbox " + GDOSMbx + " ...");
            Tims.mbxInit(GDOSMbx);

        }
        catch (NumberFormatException e)
        {

            JOptionPane
                    .showMessageDialog(
                            frame,
                            "Can't start Gui. Use \"Gui [gatewayIp] gatewayPort gui.cfg\"",
                            "configuration-error", JOptionPane.ERROR_MESSAGE);
            System.exit(-1);

        }
        catch (MsgException e)
        {

            JOptionPane.showMessageDialog(frame,
                    "Can't start Gui: MsgException " + e.toString()
                            + "\nConnection to Tims Message Gateway possible?",
                    "configuration-error", JOptionPane.ERROR_MESSAGE);
            System.out.println("Can't start Gui: MsgException " + e.toString());
            System.exit(-1);

        }

    }

    public void initModuleProxy()
    {
        for (int i = 0; i < MODULE_NUM; i++)
        {
            moduleProxy[i] = createModuleProxy(i);
        }
    }

    public void initGui()
    {
        try
        {
            UIManager.setLookAndFeel(UIManager
                    .getCrossPlatformLookAndFeelClassName());
            // UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        }
        catch (Exception e)
        {
            System.out.println(e.toString());
        }

        // create main frame
        frame = new JFrame("Robot Gui " + gwAddress.getHostAddress());
        frame.addWindowListener(new WindowAdapter()
        {
            public void windowClosing(WindowEvent e)
            {
                System.out.println(e.toString());

                terminate();
                System.out.println("Terminate");

                writeConfig(fileName);
                System.out.println("writeConfig");

                // System.exit(-2);
                System.exit(0);

            }

/*
            public void windowClosed()
            {
                System.out.println("frame is closed!");
                System.exit(-2);
            }
*/
        });

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
        groupButton = new JButton[GROUP_NUM];
        groupPanel = new JPanel[GROUP_NUM];
        groupInterPanel = new JPanel[GROUP_NUM];
        for (int i = 0; i < GROUP_NUM; i++)
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
                    for (int j = 0; j < GROUP_NUM; j++)
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
                for (int i = MODULE_NUM - 1; i >= 0; i--)
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

        frame.getContentPane().add(navigationScrollPanel, BorderLayout.WEST);

        // create work panel (Tabbed Panel in the center)
        jtp = new JTabbedPane();
        jdp = new JDesktopPane[WORKSPACE_NUM];
        for (int i = 0; i < WORKSPACE_NUM; i++)
        {
        	jdp[i] = new JDesktopPane();
         	jtp.add((String)workSpaceName.get(i),jdp[i]);
        }
        frame.getContentPane().add(jtp, BorderLayout.CENTER);

        // create message frame as an internal frame
        messageFrame = new JInternalFrame("Message", true, false, true, true);
        GDOSGui gdosGui = new GDOSGui(GDOSMbx);
        messageFrame.getContentPane().add(gdosGui.getComponent());
        messageFrame.pack();
        messageFrame.setVisible(true);
        jdp[0].add(messageFrame);
        gdosGui.start();

        if (showMapView)
        {
            //create mapView panel as a tab
        	mapViewFrame = new JInternalFrame("MapView", true, false, true, true);
            MapViewGui myMapViewGui = new MapViewGui(moduleGui);
        	mapViewFrame.getContentPane().add(myMapViewGui.getComponent());
        	mapViewFrame.pack();        	
        	mapViewFrame.setVisible(true);
        	jdp[mapViewWorkSpace].add(mapViewFrame);
        	mapViewFrame.setLocation(0, 0);
        	mapViewFrame.setSize(850, 
        						 400);
        	
        }

        frame.setJMenuBar(createMenu());
        if ((mainFrameLocationSize = getMainFrameLocationSize(configZeilen)) != null)
        {
            frame.setLocation(mainFrameLocationSize[0],
                    mainFrameLocationSize[1]);
            frame.setSize(mainFrameLocationSize[2], mainFrameLocationSize[3]);

        }
        else
        {
            frame.setSize(800, 600);
            frame.setLocation(0, 0);
        }

        frame.setVisible(true);
    }

    public int[] getMainFrameLocationSize(Vector configZeilen)
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

    public JMenuBar createMenu()
    {
        // Create the menu bar.
        menuBar = new JMenuBar();
        // Build the first menu.
        preferencesMenu = new JMenu("Preferences");
        preferencesMenu.setMnemonic(KeyEvent.VK_P);
        preferencesMenu.getAccessibleContext().setAccessibleDescription(
                "Setting Preferences");
        menuBar.add(preferencesMenu);

        prefPollingOnCbMenuItem = new JCheckBoxMenuItem("Enable module polling");
        prefPollingOnCbMenuItem.setMnemonic(KeyEvent.VK_E);
        prefPollingOnCbMenuItem.setSelected(prefPollingOn);
        prefPollingOnCbMenuItem.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                prefPollingOn = prefPollingOnCbMenuItem.isSelected();
            }
        });
        preferencesMenu.add(prefPollingOnCbMenuItem);

        return (menuBar);
    }

    public void initModule(int id)
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
                        openModule(module);
                    }
                    else
                    {
                        if (moduleStatus[module] == RackMsgType.MSG_DISABLED)
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

        for (int i = 0; i < GROUP_NUM; i++)
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

    public void openModule(int id)
    {
        openModule(id, null);
    }

    public void openModule(int id, int[] moduleLocationSize)
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
                    for (int j = 0; j < MODULE_NUM; j++)
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
        }
        catch (java.beans.PropertyVetoException pe)
        {
            pe.printStackTrace();
        }
    }

    protected void updateAllStatus()
    {
        groupError = new int[GROUP_NUM];
        groupOn = new int[GROUP_NUM];
        groupSum = new int[GROUP_NUM];

        for (int i = 0; i < GROUP_NUM; i++)
        {
            groupOn[i] = 0;
            groupError[i] = 0;
            groupSum[i] = 0;
        }
        for (int i = 0; i < MODULE_NUM; i++)
        {
            moduleStatus[i] = RackMsgType.MSG_TIMEOUT;
        }

        try
        {
            if (getStatusSeqNo < 127)
                getStatusSeqNo++;
            else
                getStatusSeqNo = -128;

            for (int i = 0; i < MODULE_NUM; i++)
            {
                // alle moduleProxy sind bei initModuleProxy schon vorhand.

                Tims.send0(RackMsgType.MSG_GET_STATUS, moduleProxy[i]
                        .getCommandMbx(), getStatusReplyMbx, (byte) 0,
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
                reply = Tims.receive(getStatusReplyMbx, 1000);
                if (reply.seq_nr == getStatusSeqNo)
                {
                    // update module status array
                    for (int i = 0; i < MODULE_NUM; i++)
                    {

                        if (moduleProxy[i].getCommandMbx() == reply.src)
                        {
                            moduleStatus[i] = reply.type;
                        }
                    }
                }

                // test if all replies are received
                notAllReplies = false;
                for (int i = 0; i < MODULE_NUM; i++)
                {
                    if (moduleStatus[i] == RackMsgType.MSG_TIMEOUT)
                    {
                        notAllReplies = true;
                    }
                }
            }
        }
        catch (MsgException e)
        {
            // System.out.println("Java Gui getStatus error: " +
            // e.getMessage());
        }

        for (int i = 0; i < MODULE_NUM; i++)
        {
            // bei paramentern -show und -start soll trotzdem
            // der button angezeigt werden.
            if (getModuleParameterShow(i)
                    && moduleStatus[i] == RackMsgType.MSG_NOT_AVAILABLE)
                moduleStatus[i] = RackMsgType.MSG_DISABLED;
            if (getModuleParameterStart(i)
                    && moduleStatus[i] == RackMsgType.MSG_NOT_AVAILABLE)
                moduleStatus[i] = RackMsgType.MSG_DISABLED;

            switch (moduleStatus[i])
            {
                case RackMsgType.MSG_ERROR:
                    if (modulePanel[i] == null)
                    {
                        initModule(i);
                    }
                    statusButton[i].setEnabled(true);
                    statusButton[i].setSelected(true);
                    statusButton[i].setForeground(Color.red);
                    moduleButton[i].setEnabled(true);

                    for (int id = 0; id < GROUP_NUM; id++)
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

                case RackMsgType.MSG_ENABLED:
                    if (modulePanel[i] == null)
                    {
                        initModule(i);
                    }
                    statusButton[i].setEnabled(true);
                    statusButton[i].setSelected(true);
                    statusButton[i].setForeground(Color.green);
                    moduleButton[i].setEnabled(true);

                    for (int id = 0; id < GROUP_NUM; id++)
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
                case RackMsgType.MSG_DISABLED:
                    if (modulePanel[i] == null)
                    {
                        initModule(i);
                    }
                    statusButton[i].setEnabled(true);
                    statusButton[i].setSelected(false);
                    moduleButton[i].setEnabled(true);

                    for (int id = 0; id < GROUP_NUM; id++)
                    {
                        if (isInGroup(id, i))
                        {
                            groupSum[id]++;
                            // System.out.println(
                            // "groupSum[" + id + "]" + "=" + groupSum[id]);
                        }
                    }

                    break;

                case RackMsgType.MSG_NOT_AVAILABLE:
                    if (modulePanel[i] != null)
                    {
                        statusButton[i].setEnabled(false);
                        statusButton[i].setSelected(false);
                        moduleButton[i].setEnabled(false);
                    }
                    break;
                case RackMsgType.MSG_TIMEOUT:
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

            if (prefPollingOn)
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
            }
            navigationPanel.revalidate();
            try
            {
                Thread.sleep(2000);
            }
            catch (InterruptedException e)
            {
            }

        }

    }

    public void terminate()
    {
        terminate = true;
    }

    public static void main(String[] args)
    {
        try
        {
            new Gui(args);
        }
        catch (Throwable t)
        {
            System.out.println(t);
            t.printStackTrace();
        }
    }
}
