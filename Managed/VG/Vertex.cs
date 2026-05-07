using System.Numerics;
using System.Runtime.InteropServices;

namespace Luna.VG;

[StructLayout(LayoutKind.Sequential)]
public readonly struct Vertex
{
    public Vertex(Vector2 position, Vector2 shapeCoord, Vector2 texCoord, uint beginCommand, uint numCommands, Vector4 color)
    {
        Position = position;
        ShapeCoord = shapeCoord;
        TexCoord = texCoord;
        BeginCommand = beginCommand;
        NumCommands = numCommands;
        Color = color;
    }

    public Vector2 Position { get; }
    public Vector2 ShapeCoord { get; }
    public Vector2 TexCoord { get; }
    public uint BeginCommand { get; }
    public uint NumCommands { get; }
    public Vector4 Color { get; }
}
