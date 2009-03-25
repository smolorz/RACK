package rack.main.tims;

import java.io.*;

public class BigEndianDataInputStream extends EndianDataInputStream
{
    public BigEndianDataInputStream(InputStream in)
    {
        this.in = new DataInputStream(in);
    }

    public final short readShort() throws IOException
    {
        return in.readShort();
    }

    public final int readUnsignedShort() throws IOException
    {
        return in.readUnsignedShort();
    }

    public final char readChar() throws IOException
    {
        return in.readChar();
    }

    public final int readInt() throws IOException
    {
        return in.readInt();
    }

    public final long readLong() throws IOException
    {
        return in.readLong();
    }
}
