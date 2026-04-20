using System;
using Luna.Runtime;
using Luna.RHI.Internal;

namespace Luna.RHI;

public static class Module
{
    public static void Init()
    {
        if (!global::Luna.Runtime.Runtime.IsInitialized)
        {
            throw new InvalidOperationException("Luna runtime must be initialized before initializing the RHI module.");
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.InitModule()));
    }

    public static BackendType BackendType => (BackendType)RhiNative.GetBackendType();

    public static IDevice GetMainDevice()
    {
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.GetMainDevice(out var device)));
        return new RhiDevice(device.Object, device.IDevice, retain: true);
    }

    public static IDevice CreateDevice(IAdapter adapter)
    {
        ArgumentNullException.ThrowIfNull(adapter);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.NewDevice(RhiAdapter.GetNativeAdapterPointer(adapter), out var device)));
        return new RhiDevice(device.Object, device.IDevice, retain: false);
    }

    public static IAdapter[] GetAdapters()
    {
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.GetNumAdapters(out var count)));
        var adapters = new IAdapter[count];
        try
        {
            for (uint i = 0; i < count; ++i)
            {
                RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.GetAdapter(i, out var adapter)));
                adapters[i] = new RhiAdapter(adapter.Object, adapter.IAdapter, retain: false);
            }
            return adapters;
        }
        catch
        {
            foreach (var adapter in adapters)
            {
                adapter?.Dispose();
            }
            throw;
        }
    }
}
