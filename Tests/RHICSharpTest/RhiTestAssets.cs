using System;
using System.IO;
using Luna.Image;

internal static class RhiTestAssets
{
    public static ImageData LoadTestImage()
    {
        var image = ImageModule.ReadFile(Path.Combine(AppContext.BaseDirectory, "uv_checker.png"), ImageFormat.Rgba8Unorm);
        var expectedSize = checked((int)(image.Desc.Width * image.Desc.Height * ImageModule.PixelSize(image.Desc.Format)));
        if (image.Desc.Format != ImageFormat.Rgba8Unorm || image.Desc.Width == 0 || image.Desc.Height == 0 || image.Data.Length != expectedSize)
        {
            throw new InvalidOperationException("Image module returned unexpected decoded image data.");
        }
        return image;
    }

    public static byte[] CreateTriangleVertexData()
    {
        const int vertexStride = 24;
        var data = new byte[vertexStride * 3];
        WriteTriangleVertex(data, 0, 0.0f, 0.7f, 1.0f, 0.0f, 0.0f, 1.0f);
        WriteTriangleVertex(data, vertexStride, 0.7f, -0.7f, 0.0f, 1.0f, 0.0f, 1.0f);
        WriteTriangleVertex(data, vertexStride * 2, -0.7f, -0.7f, 0.0f, 0.0f, 1.0f, 1.0f);
        return data;
    }

    public static byte[] CreateTexturedQuadVertexData(uint textureWidth, uint textureHeight, uint framebufferWidth, uint framebufferHeight)
    {
        const int vertexStride = 16;
        var width = (float)textureWidth;
        var height = (float)textureHeight;
        var w = (float)framebufferWidth;
        var h = (float)framebufferHeight;
        var data = new byte[vertexStride * 4];
        WriteTexturedQuadVertex(data, 0, -width / w, height / h, 0.0f, 0.0f);
        WriteTexturedQuadVertex(data, vertexStride, width / w, height / h, 1.0f, 0.0f);
        WriteTexturedQuadVertex(data, vertexStride * 2, -width / w, -height / h, 0.0f, 1.0f);
        WriteTexturedQuadVertex(data, vertexStride * 3, width / w, -height / h, 1.0f, 1.0f);
        return data;
    }

    public static byte[] CreateQuadIndexData()
    {
        var data = new byte[sizeof(uint) * 6];
        WriteUInt32(data, 0, 0);
        WriteUInt32(data, sizeof(uint), 1);
        WriteUInt32(data, sizeof(uint) * 2, 2);
        WriteUInt32(data, sizeof(uint) * 3, 1);
        WriteUInt32(data, sizeof(uint) * 4, 3);
        WriteUInt32(data, sizeof(uint) * 5, 2);
        return data;
    }

    public static byte[] CreateBoxVertexData()
    {
        const int vertexStride = 20;
        var data = new byte[vertexStride * 24];
        var offset = 0;
        WriteBoxVertex(data, ref offset, +0.5f, -0.5f, -0.5f, 0.0f, 1.0f);
        WriteBoxVertex(data, ref offset, +0.5f, +0.5f, -0.5f, 0.0f, 0.0f);
        WriteBoxVertex(data, ref offset, +0.5f, +0.5f, +0.5f, 1.0f, 0.0f);
        WriteBoxVertex(data, ref offset, +0.5f, -0.5f, +0.5f, 1.0f, 1.0f);

        WriteBoxVertex(data, ref offset, +0.5f, -0.5f, +0.5f, 0.0f, 1.0f);
        WriteBoxVertex(data, ref offset, +0.5f, +0.5f, +0.5f, 0.0f, 0.0f);
        WriteBoxVertex(data, ref offset, -0.5f, +0.5f, +0.5f, 1.0f, 0.0f);
        WriteBoxVertex(data, ref offset, -0.5f, -0.5f, +0.5f, 1.0f, 1.0f);

        WriteBoxVertex(data, ref offset, -0.5f, -0.5f, +0.5f, 0.0f, 1.0f);
        WriteBoxVertex(data, ref offset, -0.5f, +0.5f, +0.5f, 0.0f, 0.0f);
        WriteBoxVertex(data, ref offset, -0.5f, +0.5f, -0.5f, 1.0f, 0.0f);
        WriteBoxVertex(data, ref offset, -0.5f, -0.5f, -0.5f, 1.0f, 1.0f);

        WriteBoxVertex(data, ref offset, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f);
        WriteBoxVertex(data, ref offset, -0.5f, +0.5f, -0.5f, 0.0f, 0.0f);
        WriteBoxVertex(data, ref offset, +0.5f, +0.5f, -0.5f, 1.0f, 0.0f);
        WriteBoxVertex(data, ref offset, +0.5f, -0.5f, -0.5f, 1.0f, 1.0f);

        WriteBoxVertex(data, ref offset, -0.5f, +0.5f, -0.5f, 0.0f, 1.0f);
        WriteBoxVertex(data, ref offset, -0.5f, +0.5f, +0.5f, 0.0f, 0.0f);
        WriteBoxVertex(data, ref offset, +0.5f, +0.5f, +0.5f, 1.0f, 0.0f);
        WriteBoxVertex(data, ref offset, +0.5f, +0.5f, -0.5f, 1.0f, 1.0f);

        WriteBoxVertex(data, ref offset, +0.5f, -0.5f, -0.5f, 0.0f, 1.0f);
        WriteBoxVertex(data, ref offset, +0.5f, -0.5f, +0.5f, 0.0f, 0.0f);
        WriteBoxVertex(data, ref offset, -0.5f, -0.5f, +0.5f, 1.0f, 0.0f);
        WriteBoxVertex(data, ref offset, -0.5f, -0.5f, -0.5f, 1.0f, 1.0f);
        return data;
    }

    public static byte[] CreateBoxIndexData()
    {
        uint[] indices =
        {
            0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
            8, 9, 10, 8, 10, 11,
            12, 13, 14, 12, 14, 15,
            16, 17, 18, 16, 18, 19,
            20, 21, 22, 20, 22, 23
        };
        var data = new byte[sizeof(uint) * indices.Length];
        for (var i = 0; i < indices.Length; ++i)
        {
            WriteUInt32(data, sizeof(uint) * i, indices[i]);
        }
        return data;
    }

    public static byte[] CreateWorldToProjectionMatrix(uint framebufferWidth, uint framebufferHeight)
    {
        var eye = (X: 2.3f, Y: 1.2f, Z: 1.7f);
        var target = (X: 0.0f, Y: 0.0f, Z: 0.0f);
        var up = (X: 0.0f, Y: 1.0f, Z: 0.0f);
        var view = MakeLookAt(eye, target, up);
        var projection = MakePerspective((float)Math.PI / 3.0f, (float)framebufferWidth / framebufferHeight, 1.0f, 4.0f);
        var worldToProjection = Multiply(projection, view);
        var data = new byte[sizeof(float) * 16];
        for (var i = 0; i < 16; ++i)
        {
            WriteFloat(data, sizeof(float) * i, worldToProjection[i]);
        }
        return data;
    }

    public static bool BytesEqual(byte[] left, byte[] right)
    {
        if (left.Length != right.Length)
        {
            return false;
        }
        for (var i = 0; i < left.Length; ++i)
        {
            if (left[i] != right[i])
            {
                return false;
            }
        }
        return true;
    }

    public static ulong AlignUp(ulong value, uint alignment)
    {
        if (alignment <= 1)
        {
            return value;
        }
        var mask = (ulong)alignment - 1;
        return (value + mask) & ~mask;
    }

    private static void WriteBoxVertex(byte[] data, ref int offset, float x, float y, float z, float u, float v)
    {
        WriteFloat(data, offset, x);
        WriteFloat(data, offset + 4, y);
        WriteFloat(data, offset + 8, z);
        WriteFloat(data, offset + 12, u);
        WriteFloat(data, offset + 16, v);
        offset += 20;
    }

    private static void WriteTriangleVertex(byte[] data, int offset, float x, float y, float red, float green, float blue, float alpha)
    {
        WriteFloat(data, offset, x);
        WriteFloat(data, offset + 4, y);
        WriteFloat(data, offset + 8, red);
        WriteFloat(data, offset + 12, green);
        WriteFloat(data, offset + 16, blue);
        WriteFloat(data, offset + 20, alpha);
    }

    private static void WriteTexturedQuadVertex(byte[] data, int offset, float x, float y, float u, float v)
    {
        WriteFloat(data, offset, x);
        WriteFloat(data, offset + 4, y);
        WriteFloat(data, offset + 8, u);
        WriteFloat(data, offset + 12, v);
    }

    private static void WriteFloat(byte[] data, int offset, float value)
    {
        if (!BitConverter.TryWriteBytes(data.AsSpan(offset, sizeof(float)), value))
        {
            throw new InvalidOperationException("Failed to encode vertex data.");
        }
    }

    private static void WriteUInt32(byte[] data, int offset, uint value)
    {
        if (!BitConverter.TryWriteBytes(data.AsSpan(offset, sizeof(uint)), value))
        {
            throw new InvalidOperationException("Failed to encode index data.");
        }
    }

    private static float[] MakeLookAt(
        (float X, float Y, float Z) eye,
        (float X, float Y, float Z) target,
        (float X, float Y, float Z) up)
    {
        var z = Normalize((target.X - eye.X, target.Y - eye.Y, target.Z - eye.Z));
        var x = Normalize(Cross(up, z));
        var y = Cross(z, x);
        return new[]
        {
            x.X, x.Y, x.Z, -Dot(x, eye),
            y.X, y.Y, y.Z, -Dot(y, eye),
            z.X, z.Y, z.Z, -Dot(z, eye),
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    private static float[] MakePerspective(float fovY, float aspect, float nearZ, float farZ)
    {
        var yScale = 1.0f / MathF.Tan(fovY * 0.5f);
        var xScale = yScale / aspect;
        return new[]
        {
            xScale, 0.0f, 0.0f, 0.0f,
            0.0f, yScale, 0.0f, 0.0f,
            0.0f, 0.0f, farZ / (farZ - nearZ), -(nearZ * farZ) / (farZ - nearZ),
            0.0f, 0.0f, 1.0f, 0.0f
        };
    }

    private static float[] Multiply(float[] left, float[] right)
    {
        var result = new float[16];
        for (var row = 0; row < 4; ++row)
        {
            for (var column = 0; column < 4; ++column)
            {
                result[row * 4 + column] =
                    left[row * 4] * right[column] +
                    left[row * 4 + 1] * right[4 + column] +
                    left[row * 4 + 2] * right[8 + column] +
                    left[row * 4 + 3] * right[12 + column];
            }
        }
        return result;
    }

    private static (float X, float Y, float Z) Normalize((float X, float Y, float Z) value)
    {
        var length = MathF.Sqrt(value.X * value.X + value.Y * value.Y + value.Z * value.Z);
        return (value.X / length, value.Y / length, value.Z / length);
    }

    private static (float X, float Y, float Z) Cross((float X, float Y, float Z) left, (float X, float Y, float Z) right)
    {
        return (
            left.Y * right.Z - left.Z * right.Y,
            left.Z * right.X - left.X * right.Z,
            left.X * right.Y - left.Y * right.X);
    }

    private static float Dot((float X, float Y, float Z) left, (float X, float Y, float Z) right)
    {
        return left.X * right.X + left.Y * right.Y + left.Z * right.Z;
    }
}
