using System;
using System.Runtime.InteropServices;
using Luna.Runtime.Internal;

namespace Luna.Runtime;

public static class RuntimePaths
{
    public static string CurrentDirectory
    {
        get => GetAndRelease(RuntimeNativeGenerated.GetCurrentDir, RuntimeNativeGenerated.ReleaseCurrentDir);
        set
        {
            ArgumentNullException.ThrowIfNull(value);
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RuntimeNativeGenerated.SetCurrentDir(value)));
        }
    }

    public static string ProcessPath => GetAndRelease(RuntimeNativeGenerated.GetProcessPath, RuntimeNativeGenerated.ReleaseProcessPath);

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
