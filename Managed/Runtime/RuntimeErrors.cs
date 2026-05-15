using System;
using System.Runtime.InteropServices;
using Luna.Runtime.Internal;

namespace Luna.Runtime;

public static class RuntimeErrors
{
    public static void ThrowIfFailed(ErrorCode code)
    {
        NativeError.ThrowIfFailed(code.Value);
    }

    public static ErrorCode GetCodeByName(string categoryName, string codeName)
    {
        ArgumentException.ThrowIfNullOrEmpty(categoryName);
        ArgumentException.ThrowIfNullOrEmpty(codeName);
        return new ErrorCode(RuntimeNativeGenerated.ErrorGetCodeByName(categoryName, codeName));
    }

    public static ErrorCategory GetCategoryByName(string categoryName)
    {
        ArgumentException.ThrowIfNullOrEmpty(categoryName);
        return new ErrorCategory(RuntimeNativeGenerated.ErrorGetCategoryByName(categoryName));
    }

    public static string GetCodeName(ErrorCode code)
    {
        return PtrToString(RuntimeNativeGenerated.ErrorGetCodeName(code.Value));
    }

    public static string GetCategoryName(ErrorCategory category)
    {
        return PtrToString(RuntimeNativeGenerated.ErrorGetCategoryName(category.Value));
    }

    public static ErrorCategory GetCodeCategory(ErrorCode code)
    {
        return new ErrorCategory(RuntimeNativeGenerated.ErrorGetCodeCategory(code.Value));
    }

    public static string Explain(ErrorCode code)
    {
        return PtrToString(RuntimeNativeGenerated.ErrorExplain(code.Value));
    }

    public static ErrorCode Unwrap(ErrorCode code)
    {
        return new ErrorCode(RuntimeNativeGenerated.ErrorUnwrap(code.Value));
    }

    public static ErrorCode CurrentCode => new(RuntimeNativeGenerated.ErrorGetCurrentCode());

    public static string CurrentMessage => PtrToString(RuntimeNativeGenerated.ErrorGetCurrentMessage());

    private static string PtrToString(IntPtr value)
    {
        return Marshal.PtrToStringUTF8(value) ?? string.Empty;
    }
}
