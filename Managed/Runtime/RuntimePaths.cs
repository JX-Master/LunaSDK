using System;
using System.Runtime.InteropServices;
using Luna.Runtime.Internal;

namespace Luna.Runtime;

public static class RuntimePaths
{
    public static string CurrentDirectory
    {
        get => GetAndRelease(RuntimeNative.GetCurrentDir, RuntimeNative.ReleaseCurrentDir);
        set
        {
            ArgumentNullException.ThrowIfNull(value);
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNative.SetCurrentDir(value)));
        }
    }

    public static string ProcessPath => GetAndRelease(RuntimeNative.GetProcessPath, RuntimeNative.ReleaseProcessPath);

    private static string GetAndRelease(Func<IntPtr> get, Action<IntPtr> release)
    {
        var path = get();
        if (path == IntPtr.Zero)
        {
            return string.Empty;
        }

        try
        {
            return Marshal.PtrToStringUTF8(path) ?? string.Empty;
        }
        finally
        {
            release(path);
        }
    }
}
