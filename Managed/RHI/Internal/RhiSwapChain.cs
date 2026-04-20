using System;
using Luna.Runtime;
using Luna.RHI.Internal;
using Luna.Window;

namespace Luna.RHI;

internal sealed class RhiSwapChain : RhiDeviceChild, ISwapChain
{
    private readonly IntPtr _iswapChain;

    internal RhiSwapChain(IntPtr nativeObject, IntPtr nativeSwapChain, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeSwapChain == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeSwapChain));
        }
        _iswapChain = nativeSwapChain;
    }

    public IWindow Window
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.SwapChainGetWindow(_iswapChain, out var window)));
            return WindowModule.WrapNativeWindow(window.Object, window.IWindow, retain: false);
        }
    }

    public SwapChainDesc Desc
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.SwapChainGetDesc(_iswapChain, out var desc)));
            return desc.ToPublic();
        }
    }

    public SwapChainSurfaceTransform SurfaceTransform
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.SwapChainGetSurfaceTransform(_iswapChain, out var transform)));
            return (SwapChainSurfaceTransform)transform;
        }
    }

    public bool ResetSuggested
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.SwapChainResetSuggested(_iswapChain, out var suggested)));
            return suggested != 0;
        }
    }

    public ITexture GetCurrentBackBuffer()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.SwapChainGetCurrentBackBuffer(_iswapChain, out var texture)));
        return new RhiTexture(texture.Object, texture.ITexture, retain: true);
    }

    public void Present()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.SwapChainPresent(_iswapChain)));
    }

    public void Reset(SwapChainDesc desc)
    {
        EnsureNotDisposed();
        var nativeDesc = NativeSwapChainDesc.FromPublic(desc);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.SwapChainReset(_iswapChain, in nativeDesc)));
    }
}
