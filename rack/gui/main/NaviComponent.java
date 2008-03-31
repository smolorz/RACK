package rack.gui.main;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.font.FontRenderContext;
import java.awt.geom.Rectangle2D;

import javax.swing.JComponent;

import rack.main.defines.Position3d;

public class NaviComponent extends JComponent
{
    protected Position3d position = new Position3d();
    protected Position3d destination = null;
    protected boolean northArrowVisible = false;
    protected Color color = Color.RED;
    
    private static final long serialVersionUID = 1L;

    public NaviComponent()
    {
    }
    
    public NaviComponent(Color color, boolean northArrowVisible)
    {
        this.color = color;
        this.northArrowVisible = northArrowVisible;
    }
    
    public void setPosition(Position3d position)
    {
        this.position = position;
        this.repaint();
    }

    public void setDestination(Position3d destination)
    {
        this.destination = destination;
        this.repaint();
    }

    public void setNorthArrowVisible(boolean visible)
    {
        this.northArrowVisible = visible;
    }
    
    protected void drawArrow(Graphics g, double direction, boolean filled)
    {
        int cX = this.getWidth() / 2;
        int cY = this.getHeight() / 2;
        
        int radius = cX;
        if(radius > cY)
            radius = cY;
        
        int pX[] = new int[4];
        int pY[] = new int[4];
        
        pX[0] = cX + (int)(-0.5*(double)radius * Math.sin(direction));
        pY[0] = cY + (int)(-0.5*(double)radius * -Math.cos(direction));

        pX[1] = cX + (int)((double)radius * Math.sin(direction + 4.0*Math.PI/5.0));
        pY[1] = cY + (int)((double)radius * -Math.cos(direction + 4.0*Math.PI/5.0));

        pX[2] = cX + (int)((double)radius * Math.sin(direction));
        pY[2] = cY + (int)((double)radius * -Math.cos(direction));

        pX[3] = cX + (int)((double)radius * Math.sin(direction - 4.0*Math.PI/5.0));
        pY[3] = cY + (int)((double)radius * -Math.cos(direction - 4.0*Math.PI/5.0));

        if(filled)
        {
            g.fillPolygon(pX, pY, 4);
        }
        else
        {
            g.drawPolygon(pX, pY, 4);
        }
    }
    
    public void paint(Graphics g)
    {
        Graphics2D g2 = (Graphics2D)g;
        
        int cX = this.getWidth() / 2;
        int cY = this.getHeight() / 2;
        
        int radius = cX;
        if(radius > cY)
            radius = cY;
        
        g2.setColor(Color.BLACK);
        g2.drawArc(cX - radius, cY - radius, 2 * radius - 1, 2 * radius - 1, 0, 360);

        if(northArrowVisible)
        {
            g2.setColor(Color.BLACK);
            drawArrow(g2, -position.rho, false);
        }

        if(destination != null)
        {
            double dx = destination.x - position.x;
            double dy = destination.y - position.y;
            
            double direction = Math.atan2(dy, dx) - position.rho;
            int distance = (int)Math.sqrt(dx * dx + dy * dy);

            g2.setColor(color);
            drawArrow(g2, direction, true);

            String str;
            if(distance > 1000000)
            {
                str = (distance / 1000000) + "km";
            }
            else
            {
                str = (distance / 1000) + "m";
            }

            Font f = g2.getFont();
            f = f.deriveFont(Font.PLAIN, radius / 5);
            g2.setFont(f);

            g2.setColor(Color.BLACK);
            FontRenderContext frc = g2.getFontRenderContext();
            Rectangle2D bounds = f.getStringBounds(str, frc);

            g2.drawString(str, cX - (int)bounds.getWidth() / 2,
                          cY + f.getSize() / 2);
        }
    }
    
    public Dimension getPreferredSize()
    {
        return new Dimension(200, 200);
    }
}
