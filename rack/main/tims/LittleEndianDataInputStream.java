package rack.main.tims;

import java.io.*;

public class LittleEndianDataInputStream extends EndianDataInputStream
{
    public LittleEndianDataInputStream(InputStream in)
    {
        this.in = new DataInputStream(in);
    }

    public final short readShort() throws IOException
    {
        byte[] b = new byte[2];

        in.readFully(b);

        return (short) ((b[1] << 8) | (b[0] & 0xff));
    }

    public final int readUnsignedShort() throws IOException
    {
        byte[] b = new byte[2];

        in.readFully(b);

        return (((b[1] & 0xff) << 8) | (b[0] & 0xff));
    }

    public final char readChar() throws IOException
    {
        byte[] b = new byte[2];

        in.readFully(b);

        return (char) ((b[1] << 8) | (b[0] & 0xff));
    }

    public final int readInt() throws IOException
    {
        byte[] b = new byte[4];

        in.readFully(b);

        return (((b[3] & 0xff) << 24) | ((b[2] & 0xff) << 16) |
                ((b[1] & 0xff) << 8)  | ((b[0] & 0xff)));
    }

    public final long readLong() throws IOException
    {
        byte[] b = new byte[8];

        in.readFully(b);

        return (((long) (b[7] & 0xff) << 56) | ((long) (b[6] & 0xff) << 48) |
                ((long) (b[5] & 0xff) << 40) | ((long) (b[4] & 0xff) << 32) |
                ((long) (b[3] & 0xff) << 24) | ((long) (b[2] & 0xff) << 16) |
                ((long) (b[1] & 0xff) << 8)  | ((long) (b[0] & 0xff)));
    }
}
