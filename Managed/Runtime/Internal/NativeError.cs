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

        var code = RuntimeNativeGenerated.ErrorUnwrap(result);
        var category = RuntimeNativeGenerated.ErrorGetCodeCategory(code);
        var message = Marshal.PtrToStringUTF8(RuntimeNativeGenerated.ErrorExplain(result))
            ?? $"Luna native call failed with error code 0x{code.ToUInt64():x}.";
        var codeName = Marshal.PtrToStringUTF8(RuntimeNativeGenerated.ErrorGetCodeName(code));
        var categoryName = Marshal.PtrToStringUTF8(RuntimeNativeGenerated.ErrorGetCategoryName(category));

        throw new ErrorException(category, code, message, categoryName, codeName);
    }
}
