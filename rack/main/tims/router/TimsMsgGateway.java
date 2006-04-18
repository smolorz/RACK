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
package rack.main.tims.router;

import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ListSelectionModel;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import rack.main.naming.*;

public class TimsMsgGateway extends Thread
{

  public static final byte LOGIN = 10;
  public static final byte MBX_INIT = 11;
  public static final byte MBX_DELETE = 12;
  public static final byte MBX_INIT_WITH_REPLY = 13;
  public static final byte MBX_DELETE_WITH_REPLY = 14;
  public static final byte MBX_PURGE = 15;
  public static final byte GET_STATUS = 17;

  public static final byte ENABLE_DEBUG_OUTPUT = 30;
  public static final byte DISABLE_DEBUG_OUTPUT = 31;
  /** setzt die Zeit auf dem Gateway in [ms].
   * diese wird bei den debug-ausgaben genutzt.
   * --- nicht echtzeitfaehig!!! ----
   */
  public static final byte SET_TIME = 50;
  /** gibt die Zeit des Gateways in [ms].
   * diese wird bei den debug-ausgaben genutzt.
   * --- nicht echtzeitfaehig!!! ----
   */
  public static final byte GET_TIME = 51;
  public static final byte TIME_DATA = 52;

  protected static int       connectionCount = 0;
  protected static Vector    connectionVec = new Vector();
  protected static Hashtable mbxList = new Hashtable();
  private   static boolean   debugOutputOn = false;
  protected static boolean   guiMode = false;
  private   static long      timeOffsetMillis; // zeit-offset zur systemzeit in ms

  protected ServerSocket serverSocket;

  protected static JFrame frame;
  protected static JPanel panel;
  protected static JCheckBox debugCheckBox;
  protected static DefaultListModel connectionListModel;
  protected static JList connectionList;

  /** fuegt der Liste einen (von TimsMsgGatewayServer genrierten) Socket hinzu. */
  public static synchronized void insertSocket(Socket newSocket)
  {
    try {
      Thread newThread = new TimsMsgGatewayConnection(newSocket, mbxList);
      newThread.start();
    } catch (IOException e) {
      System.out.println("TimsMsgGateway error. " + e.toString());
    }
  }

  public static void main(String[] args)
  {
    try {
      // nachsehen, ob es einen parameter "-gui" gibt. diesen dann aus args entfernen
      ArrayList a = new ArrayList(Arrays.asList(args));
      int i = a.indexOf("-gui");
      if (i == -1)
        i = a.indexOf("-GUI");
      if (i > -1) {
        // GUI !!!
        a.remove(i);
        args = (String[]) a.toArray(new String[a.size()]);
        System.out.println("Starte Gateway im GUI");
        guiMode = true;
      } else {
        System.out.println("Tip: Use option \"-gui\" to start gw with a gui!");
      }

      switch (args.length) {
        case 1 :
          TimsMsgGatewayServer gatewayServer =
            new TimsMsgGatewayServer(
              InetAddress.getLocalHost(),
              Integer.parseInt(args[0]));
          gatewayServer.start();
          break;
        case 2 :
          TimsMsgGatewayServer gatewayServer2 =
            new TimsMsgGatewayServer(
              InetAddress.getByName(args[0]),
              Integer.parseInt(args[1]));
          gatewayServer2.start();
          break;
        case 4 : //Lauschen auf 2 IPs und 2 Ports
          TimsMsgGatewayServer gatewayServer3 =
            new TimsMsgGatewayServer(
              InetAddress.getByName(args[0]),
              Integer.parseInt(args[1]));
          TimsMsgGatewayServer gatewayServer4 =
            new TimsMsgGatewayServer(
              InetAddress.getByName(args[2]),
              Integer.parseInt(args[3]));
          gatewayServer3.start();
          gatewayServer4.start();
          break;
        default :
          System.out.println(
            "Can't start TimsMsgGateway. Wrong parameter");
          System.out.println(
            "use TimsMsgGateway [ip] port [ip2 port2]");
          waitOnError();
      }
    } catch (NumberFormatException e) {
      System.out.println("Can't start TimsMsgGateway. " + e.toString());
      System.out.println("use TimsMsgGateway [ip] port [ip2 port2]");
      waitOnError();
    } catch (IOException e) {
      System.out.println("Can't start TimsMsgGateway. " + e.toString());
      System.out.println("Another Gateway running ?");
      waitOnError();
    }
    // zum start die zeit erstmal auf null setzen
    timeOffsetMillis =
    Calendar.getInstance().getTimeInMillis();
    if (guiMode) initGui();

  }

  public static void waitOnError()
  {
    // bei windows auf bestatigung warten
    if (System.getProperty("os.name").toUpperCase().indexOf("WINDOWS")
      > -1);
    System.out.println("press <Enter>");
    try {
      new BufferedReader(new InputStreamReader(System.in)).readLine();
    } catch (IOException e2) { /* handle the error */
    }
  }

  public static void initGui()
  {
    try {
      UIManager.setLookAndFeel(
        UIManager.getCrossPlatformLookAndFeelClassName());
    } catch (Exception e) {
      System.out.println(e.toString());
    }

    connectionListModel = new DefaultListModel();
    //connectionListModel.addElement("TEST");
    //connectionList = new JList(connectionListModel);

    connectionList = new JList(connectionListModel) {
      public String getToolTipText(MouseEvent e) {
        int index = locationToIndex(e.getPoint());
        if (-1 < index) {
          String toolTip =
            "<html>"
              + connectionListModel.elementAt(index).toString()
              + ":<p>";
          toolTip += getMbxlistElementInfo(index)
            + "<p>Use right-mouse-click to kill connection.";
          return toolTip;
        } else {
          //return super.getToolTipText();
          return "Use right-mouse-click to kill connection.";
        }
      }
    };
    connectionList.setToolTipText("");
    connectionList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
    connectionList.setLayoutOrientation(JList.VERTICAL);
    connectionList.setVisibleRowCount(-1);

    JScrollPane listScroller = new JScrollPane(connectionList);
    listScroller.setPreferredSize(new Dimension(480, 80));
    //listScroller.setAlignmentX(Component.LEFT_ALIGNMENT);

    debugCheckBox = new JCheckBox("Debug output");
    debugCheckBox.setMnemonic(KeyEvent.VK_D);
    debugCheckBox.setSelected(false);

    GridBagConstraints c = new GridBagConstraints();
    panel = new JPanel(new GridBagLayout());
    panel.setBorder(BorderFactory.createEmptyBorder(4,12,4,12));
    JLabel label = new JLabel("Connections");
    c.gridx = 0;
    c.gridy = 0;
    //label.setAlignmentX(Component.LEFT_ALIGNMENT);
    panel.add(label, c);
    c.gridy = 2;
    //debugCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
    panel.add(debugCheckBox, c);
    JButton clearButton = new JButton("Clear");
    clearButton.setToolTipText("Clear debug log");
    c.gridx = 2;
    c.gridy = 4;
    c.anchor = GridBagConstraints.LAST_LINE_END;
    panel.add(clearButton, c);
    c.anchor = GridBagConstraints.CENTER;
    c.gridx = 0;
    c.gridy = 3;
    c.weightx = 1.0;
    c.weighty = 1.0;
    c.gridwidth = 3;
    //c.fill = GridBagConstraints.HORIZONTAL;
    c.fill = GridBagConstraints.BOTH;
    c.gridy = 1;
    panel.add(listScroller, c);

    //Make sure we have nice window decorations.
    // nicht mit java 1.3
    //JFrame.setDefaultLookAndFeelDecorated(true);

    frame = new JFrame("Tims Message Gateway");
    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

    frame.getContentPane().add(panel);
    frame.pack();
    frame.setVisible(true);
    frame.setSize(524, 400);
    frame.validate();

    debugCheckBox.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e) {
        debugOutputOn = debugCheckBox.isSelected();
      }
    });

    connectionList.addMouseListener(new MouseListener() {

      public void mouseClicked(MouseEvent e) {
        //String s = (String) connectionList.getSelectedValue();
        int i = connectionList.locationToIndex(e.getPoint());
        if (i >= 0) {
          connectionList.setSelectedIndex(i);
          String s = (String) connectionListModel.elementAt(i);

          // suchen, zu welchem connectionVec eintrag der connectionListModel-wert passt
          TimsMsgGatewayConnection con = null;
          int n;
          for (n = 0; n < connectionVec.size(); n++) {
            con =
              (TimsMsgGatewayConnection) connectionVec.elementAt(
                n);
            if (con.connectionListRepresentation.equals(s)) {
              break;
            }
          }

          if (con.connectionListRepresentation.equals(s)) {
            if (SwingUtilities.isRightMouseButton(e)) {
              if (JOptionPane
                .showConfirmDialog(
                  frame,
                  "Do you want to kill connection to "
                    + s
                    + "?",
                  "Kill connection",
                  JOptionPane.WARNING_MESSAGE,
                  JOptionPane.YES_NO_OPTION)
                == JOptionPane.OK_OPTION) {
                con.killConnection();
              }
            } else if (SwingUtilities.isLeftMouseButton(e)) {
              JOptionPane.showMessageDialog(
                frame,
                "<html>" + getMbxlistElementInfo(n),
                "Info for " + s,
                JOptionPane.INFORMATION_MESSAGE);
            }
          }
        }
      }
      public void mouseReleased(MouseEvent e) {
      }
      public void mousePressed(MouseEvent e) {
      }
      public void mouseEntered(MouseEvent e) {
      }
      public void mouseExited(MouseEvent e) {
        connectionList.clearSelection();
        //        connectionList.setToolTipText(
        //          "Use right-mouse-click to kill connection.");
      }
    });

    /*
    JMenuItem delete = new JMenuItem("Delete");
    final JPopupMenu popupMenu = new JPopupMenu();
    popupMenu.add(delete);
    popupMenu.add(new JMenuItem("PopupItem 2"));
    popupMenu.add(new JPopupMenu.Separator());
    popupMenu.add(new JMenuItem("PopupItem 3"));
    // showing popup
    jList.addMouseListener(new MouseAdapter() {
       public void mouseClicked(MouseEvent me) {
         if (SwingUtilities.isRightMouseButton(me)     // if right mouse button clicked

             && !jList.isSelectionEmpty()              // and

             && jList.locationToIndex(me.getPoint())   // and clicked list selection is not empty point

             == jList.getSelectedIndex()) {            // inside selected item bounds

                 popupMenu.show(jList, me.getX(), me.getY());
           }
       }
    }
    );
    */

  }

  public static String getMbxlistElementInfo(int index)
  {
    String info = "";
    synchronized (mbxList) {
      Integer key;

      for (Enumeration en = mbxList.keys(); en.hasMoreElements();) {
        key = (Integer) en.nextElement();

        if (mbxList.get(key)
          == ((TimsMsgGatewayConnection) connectionVec
            .elementAt(index))) {
          info += RackName.string(key.intValue()) + "<p>";
        }
      }
    }
    return info;
  }

  public static boolean isDebugOutputOn()
  {
    return debugOutputOn;
  }

  public static void setDebugOutputOn(boolean b)
  {
    debugOutputOn = b;
    if (guiMode)
      debugCheckBox.setSelected(b);
  }

  public static long getTimeMillis()
  {
    return (Calendar.getInstance().getTimeInMillis() - timeOffsetMillis);
  }

  public static void setTimeMillis(long time)
  {
    timeOffsetMillis = Calendar.getInstance().getTimeInMillis() - time;
  }

}
