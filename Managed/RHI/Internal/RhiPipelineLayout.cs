using System;
using Luna.Runtime;

namespace Luna.RHI;

internal sealed class RhiPipelineLayout : RhiDeviceChild, IPipelineLayout
{
    private readonly IntPtr _ipipelineLayout;

    internal RhiPipelineLayout(IntPtr nativeObject, IntPtr nativePipelineLayout, bool retain)
        : base(nativeObject, retain)
    {
        if (nativePipelineLayout == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativePipelineLayout));
        }
        _ipipelineLayout = nativePipelineLayout;
    }

    internal static IntPtr GetNativePipelineLayoutPointer(IPipelineLayout pipelineLayout)
    {
        ArgumentNullException.ThrowIfNull(pipelineLayout);
        if (pipelineLayout is not RhiPipelineLayout nativePipelineLayout)
        {
            throw new ArgumentException("The pipeline layout must be created by Luna.RHI.", nameof(pipelineLayout));
        }
        nativePipelineLayout.EnsureNotDisposed();
        return nativePipelineLayout._ipipelineLayout;
    }
}
