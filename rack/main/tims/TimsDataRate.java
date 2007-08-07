package rack.main.tims;

public class TimsDataRate
{
    public int  sendBytes        = 0;
    public int  sendMessages     = 0;
    public int  receivedBytes    = 0;
    public int  receivedMessages = 0;

    public long startTime        = 0;
    public long endTime          = 0;

    public TimsDataRate()
    {
    }

    public TimsDataRate(TimsDataRate dataRate)
    {
        sendBytes = dataRate.sendBytes;
        sendMessages = dataRate.sendMessages;
        receivedBytes = dataRate.receivedBytes;
        receivedMessages = dataRate.receivedMessages;

        startTime = dataRate.startTime;
        endTime = dataRate.endTime;
    }
    
    public String toString()
    {
        double deltaT = (float)(endTime - startTime) / 1000.0;
        double dataRate = Math.rint((float)(sendBytes + receivedBytes) / deltaT / 102.4)/10.0;
        long snd = Math.round((double)sendMessages / deltaT);
        long rcv = Math.round((double)receivedMessages / deltaT);
        return "( " + dataRate + " kbyte/s / " + snd + " / " + rcv + " )";
    }
}
