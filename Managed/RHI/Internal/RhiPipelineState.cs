using System;
using Luna.Runtime;

namespace Luna.RHI;

internal sealed class RhiPipelineState : RhiDeviceChild, IPipelineState
{
    private readonly IntPtr _ipipelineState;

    internal RhiPipelineState(IntPtr nativeObject, IntPtr nativePipelineState, bool retain)
        : base(nativeObject, retain)
    {
        if (nativePipelineState == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativePipelineState));
        }
        _ipipelineState = nativePipelineState;
    }

    internal static IntPtr GetNativePipelineStatePointer(IPipelineState pipelineState)
    {
        ArgumentNullException.ThrowIfNull(pipelineState);
        if (pipelineState is not RhiPipelineState nativePipelineState)
        {
            throw new ArgumentException("The pipeline state must be created by Luna.RHI.", nameof(pipelineState));
        }
        nativePipelineState.EnsureNotDisposed();
        return nativePipelineState._ipipelineState;
    }
}
