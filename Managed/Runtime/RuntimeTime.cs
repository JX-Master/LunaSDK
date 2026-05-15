using Luna.Runtime.Internal;

namespace Luna.Runtime;

public static class RuntimeTime
{
    public static ulong Ticks => RuntimeNativeGenerated.TimeGetTicks();

    public static double TicksPerSecond => RuntimeNativeGenerated.TimeGetTicksPerSecond();

    public static long UtcTimestamp => RuntimeNativeGenerated.TimeGetUtcTimestamp();

    public static long LocalTimestamp => RuntimeNativeGenerated.TimeGetLocalTimestamp();

    public static long LocalTimestampToUtcTimestamp(long localTimestamp)
    {
        return RuntimeNativeGenerated.TimeLocalToUtc(localTimestamp);
    }

    public static long UtcTimestampToLocalTimestamp(long utcTimestamp)
    {
        return RuntimeNativeGenerated.TimeUtcToLocal(utcTimestamp);
    }

    public static DateTime TimestampToDateTime(long timestamp)
    {
        RuntimeNativeGenerated.TimeTimestampToDatetime(timestamp, out var dateTime);
        return dateTime;
    }

    public static long DateTimeToTimestamp(DateTime dateTime)
    {
        return RuntimeNativeGenerated.TimeDatetimeToTimestamp(dateTime);
    }
}
