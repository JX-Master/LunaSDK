using System;

namespace Luna.Runtime;

public readonly struct Type : IEquatable<Type>
{
    internal Type(IntPtr handle)
    {
        Handle = handle;
    }

    internal IntPtr Handle { get; }

    public bool IsValid => Handle != IntPtr.Zero;

    public Guid Guid => RuntimeTypes.GetGuid(this);

    public string Name => RuntimeTypes.GetName(this);

    public string Alias => RuntimeTypes.GetAlias(this);

    public ulong Size => RuntimeTypes.GetSize(this);

    public ulong Alignment => RuntimeTypes.GetAlignment(this);

    public Type BaseType => RuntimeTypes.GetBase(this);

    public bool IsA(Type targetType)
    {
        return RuntimeTypes.IsType(this, targetType);
    }

    public bool Equals(Type other)
    {
        return Handle == other.Handle;
    }

    public override bool Equals(object? obj)
    {
        return obj is Type other && Equals(other);
    }

    public override int GetHashCode()
    {
        return Handle.GetHashCode();
    }

    public static bool operator ==(Type left, Type right)
    {
        return left.Equals(right);
    }

    public static bool operator !=(Type left, Type right)
    {
        return !left.Equals(right);
    }

    public override string ToString()
    {
        return IsValid ? Name : "None";
    }
}
