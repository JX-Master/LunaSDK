using System;
using Luna.Runtime;
using Luna.RHI.Internal;

namespace Luna.RHI;

internal sealed class RhiTexture : RhiResource, ITexture
{
    private readonly IntPtr _itexture;

    internal RhiTexture(IntPtr nativeObject, IntPtr nativeTexture, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeTexture == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeTexture));
        }
        _itexture = nativeTexture;
    }

    public TextureDesc Desc
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.TextureGetDesc(_itexture, out var desc)));
            return desc.ToPublic();
        }
    }

    internal static IntPtr GetNativeTexturePointer(ITexture texture)
    {
        ArgumentNullException.ThrowIfNull(texture);
        if (texture is not RhiTexture nativeTexture)
        {
            throw new ArgumentException("The texture must be created by Luna.RHI.", nameof(texture));
        }
        nativeTexture.EnsureNotDisposed();
        return nativeTexture._itexture;
    }
}
