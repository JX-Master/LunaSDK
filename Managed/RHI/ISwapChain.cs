using Luna.Window;

namespace Luna.RHI;

public interface ISwapChain : IDeviceChild
{
    IWindow Window { get; }

    SwapChainDesc Desc { get; }

    SwapChainSurfaceTransform SurfaceTransform { get; }

    bool ResetSuggested { get; }

    ITexture GetCurrentBackBuffer();

    void Present();

    void Reset(SwapChainDesc desc);
}
