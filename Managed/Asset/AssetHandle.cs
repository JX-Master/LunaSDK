using System;
using System.Runtime.InteropServices;

namespace Luna.Asset;

[StructLayout(LayoutKind.Sequential)]
public readonly struct AssetHandle : IEquatable<AssetHandle>
{
    internal AssetHandle(IntPtr handle)
    {
        Handle = handle;
    }

    internal IntPtr Handle { get; }

    public bool IsValid => Handle != IntPtr.Zero;

    public bool Equals(AssetHandle other)
    {
        return Handle == other.Handle;
    }

    public override bool Equals(object? obj)
    {
        return obj is AssetHandle other && Equals(other);
    }

    public override int GetHashCode()
    {
        return Handle.GetHashCode();
    }

    public static bool operator ==(AssetHandle left, AssetHandle right)
    {
        return left.Equals(right);
    }

    public static bool operator !=(AssetHandle left, AssetHandle right)
    {
        return !left.Equals(right);
    }
}
