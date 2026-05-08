using System;
using Luna.Runtime;
using Luna.Runtime.Internal;

namespace Luna.AHI.Internal;

internal sealed class NativeAdapter : ObjectBase, IAdapter
{
    private readonly IntPtr _iadapter;

    internal NativeAdapter(NativeAdapterHandle handle, bool retain)
        : base(handle.Object, retain)
    {
        if (handle.IAdapter == IntPtr.Zero)
        {
            throw new ArgumentException("Native adapter handle is incomplete.", nameof(handle));
        }
        _iadapter = handle.IAdapter;
    }

    public string Name
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(AhiNative.IAdapterGetName(_iadapter, out var name)));
            try
            {
                return System.Runtime.InteropServices.Marshal.PtrToStringUTF8(name) ?? string.Empty;
            }
            finally
            {
                AhiNative.FreeString(name);
            }
        }
    }

    public bool IsPrimary
    {
        get
        {
            EnsureNotDisposed();
            return AhiNative.IAdapterIsPrimary(_iadapter) != 0;
        }
    }

    public WaveFormat[] GetNativeWaveFormats()
    {
        EnsureNotDisposed();
        var firstPassCode = new ErrorCode(AhiNative.IAdapterGetNativeWaveFormats(_iadapter, null, 0, out var count));
        var insufficientBuffer = RuntimeErrors.GetCodeByName("BasicError", "insufficient_user_buffer");
        if (firstPassCode.Failed && firstPassCode != insufficientBuffer)
        {
            RuntimeErrors.ThrowIfFailed(firstPassCode);
        }
        if (count == 0)
        {
            return Array.Empty<WaveFormat>();
        }
        var nativeFormats = new NativeWaveFormat[checked((int)count)];
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AhiNative.IAdapterGetNativeWaveFormats(_iadapter, nativeFormats, (ulong)nativeFormats.Length, out count)));
        var result = new WaveFormat[nativeFormats.Length];
        for (var i = 0; i < nativeFormats.Length; ++i)
        {
            result[i] = nativeFormats[i].ToPublic();
        }
        return result;
    }

    internal static IntPtr GetNativeAdapterPointer(IAdapter? adapter)
    {
        if (adapter is null)
        {
            return IntPtr.Zero;
        }
        if (adapter is not NativeAdapter nativeAdapter)
        {
            throw new ArgumentException("The adapter must be created by Luna.AHI.", nameof(adapter));
        }
        nativeAdapter.EnsureNotDisposed();
        return nativeAdapter._iadapter;
    }
}
