using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Luna.Runtime;
using Luna.Window.Internal;

namespace Luna.Window;

#if LUNA_PLATFORM_WINDOWS || LUNA_PLATFORM_MACOS
public static class WindowFileDialogs
{
    public static string[] OpenFiles(
        string? title = null,
        IReadOnlyList<FileDialogFilter>? filters = null,
        string? initialDirectory = null,
        FileDialogFlags flags = FileDialogFlags.None)
    {
        using var nativeFilters = new NativeFilterBuffer(filters);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.OpenFileDialog(
            title,
            nativeFilters.Filters,
            (ulong)nativeFilters.Filters.Length,
            initialDirectory,
            (uint)flags,
            out var paths)));
        try
        {
            return ReadStringList(paths);
        }
        finally
        {
            WindowNative.FreeStringList(paths.Items, paths.Count);
        }
    }

    public static bool TryOpenFiles(
        out string[] paths,
        string? title = null,
        IReadOnlyList<FileDialogFilter>? filters = null,
        string? initialDirectory = null,
        FileDialogFlags flags = FileDialogFlags.None)
    {
        try
        {
            paths = OpenFiles(title, filters, initialDirectory, flags);
            return true;
        }
        catch (ErrorException ex) when (IsInterrupted(ex))
        {
            paths = Array.Empty<string>();
            return false;
        }
    }

    public static string SaveFile(
        string? title = null,
        IReadOnlyList<FileDialogFilter>? filters = null,
        string? initialFilePath = null,
        FileDialogFlags flags = FileDialogFlags.None)
    {
        var path = IntPtr.Zero;
        using var nativeFilters = new NativeFilterBuffer(filters);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.SaveFileDialog(
            title,
            nativeFilters.Filters,
            (ulong)nativeFilters.Filters.Length,
            initialFilePath,
            (uint)flags,
            out path)));
        try
        {
            return Marshal.PtrToStringUTF8(path) ?? string.Empty;
        }
        finally
        {
            WindowNative.FreeString(path);
        }
    }

    public static bool TrySaveFile(
        out string? path,
        string? title = null,
        IReadOnlyList<FileDialogFilter>? filters = null,
        string? initialFilePath = null,
        FileDialogFlags flags = FileDialogFlags.None)
    {
        try
        {
            path = SaveFile(title, filters, initialFilePath, flags);
            return true;
        }
        catch (ErrorException ex) when (IsInterrupted(ex))
        {
            path = null;
            return false;
        }
    }

    public static string OpenDirectory(string? title = null, string? initialDirectory = null)
    {
        var path = IntPtr.Zero;
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.OpenDirDialog(title, initialDirectory, out path)));
        try
        {
            return Marshal.PtrToStringUTF8(path) ?? string.Empty;
        }
        finally
        {
            WindowNative.FreeString(path);
        }
    }

    public static bool TryOpenDirectory(out string? path, string? title = null, string? initialDirectory = null)
    {
        try
        {
            path = OpenDirectory(title, initialDirectory);
            return true;
        }
        catch (ErrorException ex) when (IsInterrupted(ex))
        {
            path = null;
            return false;
        }
    }

    private static string[] ReadStringList(WindowNative.StringList list)
    {
        if (list.Count == 0)
        {
            return Array.Empty<string>();
        }

        var count = checked((int)list.Count);
        var result = new string[count];
        for (var i = 0; i < count; ++i)
        {
            var text = Marshal.ReadIntPtr(list.Items, i * IntPtr.Size);
            result[i] = Marshal.PtrToStringUTF8(text) ?? string.Empty;
        }
        return result;
    }

    private static bool IsInterrupted(ErrorException ex)
    {
        return ex.CategoryName == "BasicError" && ex.CodeName == "interrupted";
    }

    private sealed class NativeFilterBuffer : IDisposable
    {
        private readonly List<IntPtr> _strings = new();
        private readonly List<IntPtr> _extensionArrays = new();

        public NativeFilterBuffer(IReadOnlyList<FileDialogFilter>? filters)
        {
            try
            {
                if (filters is null || filters.Count == 0)
                {
                    Filters = Array.Empty<WindowNative.FileDialogFilter>();
                    return;
                }

                Filters = new WindowNative.FileDialogFilter[filters.Count];
                for (var i = 0; i < filters.Count; ++i)
                {
                    var filter = filters[i] ?? throw new ArgumentException("Filter cannot be null.", nameof(filters));
                    var extensions = filter.Extensions;
                    var extensionArray = Marshal.AllocCoTaskMem(IntPtr.Size * extensions.Count);
                    _extensionArrays.Add(extensionArray);
                    for (var j = 0; j < extensions.Count; ++j)
                    {
                        var extension = AllocString(extensions[j]);
                        Marshal.WriteIntPtr(extensionArray, j * IntPtr.Size, extension);
                    }
                    Filters[i] = new WindowNative.FileDialogFilter(
                        AllocString(filter.Name),
                        extensionArray,
                        (ulong)extensions.Count);
                }
            }
            catch
            {
                Dispose();
                throw;
            }
        }

        public WindowNative.FileDialogFilter[] Filters { get; private set; } = Array.Empty<WindowNative.FileDialogFilter>();

        public void Dispose()
        {
            foreach (var extensionArray in _extensionArrays)
            {
                Marshal.FreeCoTaskMem(extensionArray);
            }
            _extensionArrays.Clear();

            foreach (var text in _strings)
            {
                Marshal.FreeCoTaskMem(text);
            }
            _strings.Clear();
        }

        private IntPtr AllocString(string text)
        {
            var pointer = Marshal.StringToCoTaskMemUTF8(text);
            _strings.Add(pointer);
            return pointer;
        }
    }
}
#endif
