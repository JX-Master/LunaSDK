using System;
using Luna.Font.Internal;
using Luna.Runtime;
using System.Runtime.InteropServices;

namespace Luna.Font;

public static class FontModule
{
    public const int InvalidGlyph = -1;
    public const short CommandMoveTo = 1;
    public const short CommandLineTo = 2;
    public const short CommandCurveTo = 3;

    public static void Init()
    {
        if (!global::Luna.Runtime.Runtime.IsInitialized)
        {
            throw new InvalidOperationException("Luna runtime must be initialized before initializing the Font module.");
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNativeGenerated.InitModule()));
    }

    public static IFontFile LoadTtfFontFile(byte[] data)
    {
        ArgumentNullException.ThrowIfNull(data);
        using var pinnedData = PinnedByteArray.Create(data);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNativeGenerated.LoadTtfFontFile(pinnedData.Pointer, (ulong)data.Length, out var fontFile)));
        return new NativeFontFile(fontFile, retain: false);
    }

    public static IFontFile LoadTtfFontFile(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        return LoadTtfFontFile(RuntimeFile.LoadData(path));
    }

    public static IFontFile LoadTtfFontFile(IFile file)
    {
        ArgumentNullException.ThrowIfNull(file);
        return LoadTtfFontFile(RuntimeFile.LoadData(file));
    }

    public static IFontFile GetDefaultFont()
    {
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNativeGenerated.GetDefaultFont(out var fontFile)));
        return new NativeFontFile(fontFile, retain: false);
    }

    private readonly struct PinnedByteArray : IDisposable
    {
        private readonly GCHandle m_handle;

        public IntPtr Pointer { get; }

        private PinnedByteArray(GCHandle handle, IntPtr pointer)
        {
            m_handle = handle;
            Pointer = pointer;
        }

        public static PinnedByteArray Create(byte[] data)
        {
            if (data.Length == 0)
            {
                return new PinnedByteArray(default, IntPtr.Zero);
            }
            var handle = GCHandle.Alloc(data, GCHandleType.Pinned);
            return new PinnedByteArray(handle, handle.AddrOfPinnedObject());
        }

        public void Dispose()
        {
            if (m_handle.IsAllocated)
            {
                m_handle.Free();
            }
        }
    }
}
