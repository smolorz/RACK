package rack.gui;

import rack.main.RackProxy;
import rack.main.tims.Tims;

public interface GuiInterface
{
    public RackProxy getProxy(int moduleName, int instance);

    public Tims getTims();
}
