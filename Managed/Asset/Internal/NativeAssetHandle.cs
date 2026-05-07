using System;
using System.Runtime.InteropServices;

namespace Luna.Asset.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeAssetHandle
{
    public readonly IntPtr Handle;

    public AssetHandle ToManaged()
    {
        return new AssetHandle(Handle);
    }

    public static NativeAssetHandle FromManaged(AssetHandle asset)
    {
        return new NativeAssetHandle(asset.Handle);
    }

    private NativeAssetHandle(IntPtr handle)
    {
        Handle = handle;
    }
}
