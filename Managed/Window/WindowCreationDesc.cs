using Luna.Runtime;

namespace Luna.Window;

public sealed class WindowCreationDesc
{
    public string Title { get; init; } = "Luna Window";

    public int X { get; init; } = WindowModule.DefaultPosition;

    public int Y { get; init; } = WindowModule.DefaultPosition;

    public uint Width { get; init; }

    public uint Height { get; init; }

    public WindowStyleFlags StyleFlags { get; init; } = WindowStyleFlags.Resizable;

    public WindowCreationFlags CreationFlags { get; init; }
}
