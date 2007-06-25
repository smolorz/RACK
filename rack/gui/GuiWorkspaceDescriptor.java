package rack.gui;

import java.util.Vector;
import javax.swing.JDesktopPane;

public class GuiWorkspaceDescriptor
{
    GuiWorkspaceDescriptor(String name)
    {
        this.name = name;
    }
    
    String              name;
    JDesktopPane        jdp;

    Vector<GuiElementDescriptor>  element = new Vector<GuiElementDescriptor>();

    public boolean equals(Object o)
    {
        try
        {
            return name == ((GuiWorkspaceDescriptor) o).name;
        }
        catch (Exception e)
        {
            return false;
        }
    }
}
