using System;
using Luna.Runtime;
using Luna.RHI.Internal;

namespace Luna.RHI;

internal sealed class RhiAdapter : ObjectBase, IAdapter
{
    private readonly IntPtr _iadapter;

    internal RhiAdapter(IntPtr nativeObject, IntPtr nativeAdapter, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeAdapter == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeAdapter));
        }
        _iadapter = nativeAdapter;
    }

    public string Name
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.AdapterGetName(_iadapter, out var name)));
            return System.Runtime.InteropServices.Marshal.PtrToStringUTF8(name) ?? string.Empty;
        }
    }

    internal static IntPtr GetNativeAdapterPointer(IAdapter adapter)
    {
        ArgumentNullException.ThrowIfNull(adapter);
        if (adapter is not RhiAdapter nativeAdapter)
        {
            throw new ArgumentException("The adapter must be created by Luna.RHI.", nameof(adapter));
        }
        nativeAdapter.EnsureNotDisposed();
        return nativeAdapter._iadapter;
    }
}
