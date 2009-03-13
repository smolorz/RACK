package rack.main;

public class AngleTool
{
    protected static final float F_2PI = (float)(2.0 * Math.PI);
    protected static final float F_PI = (float)Math.PI;
    
    public static float normalise(float angle)
    {
        while(angle < 0.0f)
        {
            angle += F_2PI;
        }

        while(angle >= F_2PI)
        {
            angle -= F_2PI;
        }
        return angle;
    }

    public static float normaliseSym0(float angle)
    {
        while(angle <= -F_PI)
        {
            angle += F_2PI;
        }

        while(angle > F_PI)
        {
            angle -= F_2PI;
        }
        return angle;
    }
}
