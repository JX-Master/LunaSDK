using Luna.Runtime.Internal;

namespace Luna.Runtime;

public static class RuntimeTime
{
    public static ulong Ticks => RuntimeNative.TimeGetTicks();

    public static double TicksPerSecond => RuntimeNative.TimeGetTicksPerSecond();

    public static long UtcTimestamp => RuntimeNative.TimeGetUtcTimestamp();

    public static long LocalTimestamp => RuntimeNative.TimeGetLocalTimestamp();

    public static long LocalTimestampToUtcTimestamp(long localTimestamp)
    {
        return RuntimeNative.TimeLocalToUtc(localTimestamp);
    }

    public static long UtcTimestampToLocalTimestamp(long utcTimestamp)
    {
        return RuntimeNative.TimeUtcToLocal(utcTimestamp);
    }

    public static DateTime TimestampToDateTime(long timestamp)
    {
        RuntimeNative.TimeTimestampToDateTime(timestamp, out var dateTime);
        return dateTime;
    }

    public static long DateTimeToTimestamp(DateTime dateTime)
    {
        return RuntimeNative.TimeDateTimeToTimestamp(dateTime);
    }
}
