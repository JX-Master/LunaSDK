using System;
using System.Runtime.InteropServices;

namespace Luna.Runtime;

[StructLayout(LayoutKind.Sequential)]
public readonly struct Guid : IEquatable<Guid>
{
    public Guid(ulong high, ulong low)
    {
        High = high;
        Low = low;
    }

    public ulong High { get; }

    public ulong Low { get; }

    public bool Equals(Guid other)
    {
        return High == other.High && Low == other.Low;
    }

    public override bool Equals(object? obj)
    {
        return obj is Guid other && Equals(other);
    }

    public override int GetHashCode()
    {
        return HashCode.Combine(High, Low);
    }

    public static bool operator ==(Guid left, Guid right)
    {
        return left.Equals(right);
    }

    public static bool operator !=(Guid left, Guid right)
    {
        return !left.Equals(right);
    }
}
