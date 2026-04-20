using System;
using Luna.Runtime.Internal;

namespace Luna.Runtime;

public enum LogVerbosity : uint
{
    FatalError = 0,
    Error = 1,
    Warning = 2,
    Info = 3,
    Debug = 4,
    Verbose = 5
}

public static class RuntimeLog
{
    public static void Log(LogVerbosity verbosity, string tag, string message)
    {
        ArgumentNullException.ThrowIfNull(tag);
        ArgumentNullException.ThrowIfNull(message);
        RuntimeNative.Log((uint)verbosity, tag, message);
    }

    public static void Verbose(string tag, string message)
    {
        Log(LogVerbosity.Verbose, tag, message);
    }

    public static void Debug(string tag, string message)
    {
        Log(LogVerbosity.Debug, tag, message);
    }

    public static void Info(string tag, string message)
    {
        Log(LogVerbosity.Info, tag, message);
    }

    public static void Warning(string tag, string message)
    {
        Log(LogVerbosity.Warning, tag, message);
    }

    public static void Error(string tag, string message)
    {
        Log(LogVerbosity.Error, tag, message);
    }

    public static bool PlatformOutputEnabled
    {
        set => RuntimeNative.LogSetPlatformEnabled(value ? 1 : 0);
    }

    public static LogVerbosity PlatformVerbosity
    {
        set => RuntimeNative.LogSetPlatformVerbosity((uint)value);
    }

    public static bool FileOutputEnabled
    {
        set => RuntimeNative.LogSetFileEnabled(value ? 1 : 0);
    }

    public static string File
    {
        set
        {
            ArgumentNullException.ThrowIfNull(value);
            RuntimeNative.LogSetFile(value);
        }
    }

    public static LogVerbosity FileVerbosity
    {
        set => RuntimeNative.LogSetFileVerbosity((uint)value);
    }

    public static void FlushFile()
    {
        RuntimeNative.LogFlushFile();
    }
}
