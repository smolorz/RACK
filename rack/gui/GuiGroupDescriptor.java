package rack.gui;

import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JPanel;

public class GuiGroupDescriptor
{
    GuiGroupDescriptor(String name)
    {
        this.name = name;
    }
    
    String  name;

    JPanel  panel;
    JPanel  interPanel;
    JButton button;

    int     error;
    int     on;
    int     sum;

    Vector<GuiElementDescriptor> element = new Vector<GuiElementDescriptor>();

    public boolean equals(Object o)
    {
        try
        {
            return name == ((GuiGroupDescriptor) o).name;
        }
        catch (Exception e)
        {
            return false;
        }
    }
}
