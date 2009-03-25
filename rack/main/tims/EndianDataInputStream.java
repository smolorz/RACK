package rack.main.tims;

import java.io.*;

public abstract class EndianDataInputStream implements DataInput
{
    public abstract short readShort() throws IOException;

    public abstract int readUnsignedShort() throws IOException;

    public abstract char readChar() throws IOException;

    public abstract int readInt() throws IOException;

    public abstract long readLong() throws IOException;

    public final float readFloat() throws IOException
    {
        return Float.intBitsToFloat(readInt());
    }

    public final double readDouble() throws IOException
    {
        return Double.longBitsToDouble(readLong());
    }

    public final void readFully(byte[] b) throws IOException
    {
        in.readFully(b);
    }

    public final void readFully(byte[] b, int off, int len) throws IOException
    {
        in.readFully(b, off, len);
    }

    public final int skipBytes(int n) throws IOException
    {
        return (in.skipBytes(n));
    }

    public final boolean readBoolean() throws IOException
    {
        return (in.readBoolean());
    }

    public final byte readByte() throws IOException
    {
        return (in.readByte());
    }

    public final int readUnsignedByte() throws IOException
    {
        return (in.readUnsignedByte());
    }

    public final String readLine() throws IOException
    {
        throw (new IOException("readLine() is not implemented"));
    }

    public final String readUTF() throws IOException
    {
        throw (new IOException("readUTF() is not implemented"));
    }

    protected DataInputStream in;
}
