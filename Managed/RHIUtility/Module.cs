using System;
using Luna.RHI;
using Luna.RHIUtility.Internal;
using Luna.Runtime;

namespace Luna.RHIUtility;

public static class Module
{
    public static void Init()
    {
        if (!global::Luna.Runtime.Runtime.IsInitialized)
        {
            throw new InvalidOperationException("Luna runtime must be initialized before initializing the RHIUtility module.");
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.InitModule()));
    }

    public static IResourceWriteContext CreateResourceWriteContext(IDevice device)
    {
        ArgumentNullException.ThrowIfNull(device);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.NewResourceWriteContext(device.GetNativeHandle(), out var context)));
        return new ResourceWriteContext(context.Object, context.IResourceWriteContext, retain: false);
    }

    public static IResourceReadContext CreateResourceReadContext(IDevice device)
    {
        ArgumentNullException.ThrowIfNull(device);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.NewResourceReadContext(device.GetNativeHandle(), out var context)));
        return new ResourceReadContext(context.Object, context.IResourceReadContext, retain: false);
    }

    public static IBlitContext CreateBlitContext(IDevice device, Format destinationFormat)
    {
        ArgumentNullException.ThrowIfNull(device);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.NewBlitContext(device.GetNativeHandle(), (uint)destinationFormat, out var context)));
        return new BlitContext(context.Object, context.IBlitContext, retain: false);
    }

    public static IMipmapGenerationContext CreateMipmapGenerationContext(IDevice device)
    {
        ArgumentNullException.ThrowIfNull(device);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.NewMipmapGenerationContext(device.GetNativeHandle(), out var context)));
        return new MipmapGenerationContext(context.Object, context.IMipmapGenerationContext, retain: false);
    }
}
