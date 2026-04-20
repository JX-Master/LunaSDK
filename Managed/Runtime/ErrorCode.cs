using System;

namespace Luna.Runtime;

public readonly struct ErrorCode : IEquatable<ErrorCode>
{
    public ErrorCode(UIntPtr value)
    {
        Value = value;
    }

    internal UIntPtr Value { get; }

    public bool Succeeded => Value == UIntPtr.Zero;

    public bool Failed => Value != UIntPtr.Zero;

    public bool Equals(ErrorCode other)
    {
        return Value == other.Value;
    }

    public override bool Equals(object? obj)
    {
        return obj is ErrorCode other && Equals(other);
    }

    public override int GetHashCode()
    {
        return Value.GetHashCode();
    }

    public static bool operator ==(ErrorCode left, ErrorCode right)
    {
        return left.Equals(right);
    }

    public static bool operator !=(ErrorCode left, ErrorCode right)
    {
        return !left.Equals(right);
    }

    public override string ToString()
    {
        return Succeeded ? "Ok" : $"0x{Value.ToUInt64():x}";
    }
}
