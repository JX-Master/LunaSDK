using System;
using Luna.Font.Internal;
using Luna.Runtime;

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
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNative.InitModule()));
    }

    public static IFontFile LoadTtfFontFile(byte[] data)
    {
        ArgumentNullException.ThrowIfNull(data);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNative.LoadTtfFontFile(data, (ulong)data.Length, out var fontFile)));
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
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNative.GetDefaultFont(out var fontFile)));
        return new NativeFontFile(fontFile, retain: false);
    }
}
