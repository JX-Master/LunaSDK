using System;

namespace Luna.Runtime;

public readonly struct ErrorCategory : IEquatable<ErrorCategory>
{
    internal ErrorCategory(UIntPtr value)
    {
        Value = value;
    }

    internal UIntPtr Value { get; }

    public bool IsValid => Value != UIntPtr.Zero;

    public bool Equals(ErrorCategory other)
    {
        return Value == other.Value;
    }

    public override bool Equals(object? obj)
    {
        return obj is ErrorCategory other && Equals(other);
    }

    public override int GetHashCode()
    {
        return Value.GetHashCode();
    }

    public static bool operator ==(ErrorCategory left, ErrorCategory right)
    {
        return left.Equals(right);
    }

    public static bool operator !=(ErrorCategory left, ErrorCategory right)
    {
        return !left.Equals(right);
    }

    public override string ToString()
    {
        return IsValid ? $"0x{Value.ToUInt64():x}" : "None";
    }
}
