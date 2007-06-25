package rack.gui;

import java.awt.Dimension;
import java.awt.Point;

import javax.swing.JButton;
import javax.swing.JInternalFrame;
import javax.swing.JPanel;
import javax.swing.JRadioButton;

import rack.gui.main.RackModuleGui;
import rack.main.RackProxy;
import rack.main.tims.TimsMbx;

public class GuiElementDescriptor
{
    // general
    String          name;
    RackModuleGui   gui;

    RackProxy       proxy;
    int             instance = -1;
    int             status;
    TimsMbx         replyMbx;

    // gui.cfg
    String          cfg;
    String[]        cfgSplit;
    String          guiClass;
    String          proxyClass;
    boolean         start;
    boolean         show;

    // swing
    JInternalFrame  frame;
    Point           location    = new Point();
    Dimension       size        = new Dimension();
    JPanel          navPanel;
    JButton         navButton;
    JRadioButton    navStatusButton;

    // references
    GuiInterface            mainGui;
    GuiGroupDescriptor      group;
    GuiWorkspaceDescriptor  workspace;

    public String getName()
    {
        return name;
    }
    
    public RackProxy getProxy()
    {
        return proxy;
    }
    
    public int getInstance()
    {
        return instance;
    }
    
    public GuiInterface getMainGui()
    {
        return mainGui;
    }
    
    public boolean equals(Object o)
    {
        try
        {
            return cfg == ((GuiElementDescriptor) o).cfg;
        }
        catch (Exception e)
        {
            return false;
        }
    }
    
    public String toString()
    {
        return cfg;
    }
}
