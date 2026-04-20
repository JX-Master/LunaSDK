using System;
using System.Runtime.InteropServices;

namespace Luna.Runtime.Internal;

internal static class NativeError
{
    public static void ThrowIfFailed(UIntPtr result)
    {
        if (result == UIntPtr.Zero)
        {
            return;
        }

        var code = RuntimeNative.ErrorUnwrap(result);
        var category = RuntimeNative.ErrorGetCodeCategory(code);
        var message = Marshal.PtrToStringUTF8(RuntimeNative.ErrorExplain(result))
            ?? $"Luna native call failed with error code 0x{code.ToUInt64():x}.";
        var codeName = Marshal.PtrToStringUTF8(RuntimeNative.ErrorGetCodeName(code));
        var categoryName = Marshal.PtrToStringUTF8(RuntimeNative.ErrorGetCategoryName(category));

        throw new ErrorException(category, code, message, categoryName, codeName);
    }
}
