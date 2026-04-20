namespace Luna.RHI;

public enum SwapChainSurfaceTransform : uint
{
    Unspecified = 0,
    Identity = 1,
    Rotate90 = 2,
    Rotate180 = 3,
    Rotate270 = 4,
    HorizontalMirror = 5,
    HorizontalMirrorRotate90 = 6,
    HorizontalMirrorRotate180 = 7,
    HorizontalMirrorRotate270 = 8
}
