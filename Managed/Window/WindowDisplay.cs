#if LUNA_PLATFORM_DESKTOP
using System;

namespace Luna.Window;

public readonly struct WindowDisplay : IEquatable<WindowDisplay>
{
    internal WindowDisplay(IntPtr handle)
    {
        Handle = handle;
    }

    internal IntPtr Handle { get; }

    public bool IsValid => Handle != IntPtr.Zero;

    public bool Equals(WindowDisplay other)
    {
        return Handle == other.Handle;
    }

    public override bool Equals(object? obj)
    {
        return obj is WindowDisplay other && Equals(other);
    }

    public override int GetHashCode()
    {
        return Handle.GetHashCode();
    }

    public static bool operator ==(WindowDisplay left, WindowDisplay right)
    {
        return left.Equals(right);
    }

    public static bool operator !=(WindowDisplay left, WindowDisplay right)
    {
        return !left.Equals(right);
    }
}
#endif
