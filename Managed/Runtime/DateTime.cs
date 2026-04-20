using System.Runtime.InteropServices;

namespace Luna.Runtime;

[StructLayout(LayoutKind.Sequential)]
public readonly struct DateTime
{
    public DateTime(short year, byte month, byte day, byte hour, byte minute, byte second, byte dayOfWeek)
    {
        Year = year;
        Month = month;
        Day = day;
        Hour = hour;
        Minute = minute;
        Second = second;
        DayOfWeek = dayOfWeek;
    }

    public readonly short Year;

    public readonly byte Month;

    public readonly byte Day;

    public readonly byte Hour;

    public readonly byte Minute;

    public readonly byte Second;

    public readonly byte DayOfWeek;

    public override string ToString()
    {
        return $"{Year:D4}-{Month:D2}-{Day:D2} {Hour:D2}:{Minute:D2}:{Second:D2}";
    }
}
