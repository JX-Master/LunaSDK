using System;

namespace Luna.Runtime;

public sealed class ErrorException : Exception
{
    public ErrorException(UIntPtr category, UIntPtr code, string message, string? categoryName = null, string? codeName = null)
        : base(message)
    {
        Category = category;
        Code = code;
        CategoryName = categoryName;
        CodeName = codeName;
    }

    public UIntPtr Category { get; }

    public UIntPtr Code { get; }

    public string? CategoryName { get; }

    public string? CodeName { get; }
}
