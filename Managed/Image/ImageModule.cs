using System;
using System.Runtime.InteropServices;
using Luna.Image.Internal;
using Luna.Runtime;

namespace Luna.Image;

public static class ImageModule
{
    public static void Init()
    {
        if (!global::Luna.Runtime.Runtime.IsInitialized)
        {
            throw new InvalidOperationException("Luna runtime must be initialized before initializing the Image module.");
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ImageNative.InitModule()));
    }

    public static ImageDesc ReadFileDesc(byte[] fileData)
    {
        ArgumentNullException.ThrowIfNull(fileData);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ImageNative.ReadFileDesc(fileData, (ulong)fileData.Length, out var desc)));
        return desc.ToManaged();
    }

    public static ImageDesc ReadFileDesc(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        return ReadFileDesc(RuntimeFile.LoadData(path));
    }

    public static ImageData ReadFile(byte[] fileData, ImageFormat desiredFormat)
    {
        ArgumentNullException.ThrowIfNull(fileData);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ImageNative.ReadFile(fileData, (ulong)fileData.Length, (uint)desiredFormat, out var nativeImage)));
        try
        {
            if (nativeImage.DataSize > int.MaxValue)
            {
                throw new InvalidOperationException("The image data is too large to copy into a managed byte array.");
            }
            var data = new byte[(int)nativeImage.DataSize];
            if (nativeImage.DataSize > 0)
            {
                Marshal.Copy(nativeImage.Data, data, 0, data.Length);
            }
            return new ImageData(data, nativeImage.Desc.ToManaged());
        }
        finally
        {
            ImageNative.FreeData(ref nativeImage);
        }
    }

    public static ImageData ReadFile(string path, ImageFormat desiredFormat)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        return ReadFile(RuntimeFile.LoadData(path), desiredFormat);
    }

    public static uint PixelSize(ImageFormat format)
    {
        return format switch
        {
            ImageFormat.R8Unorm => 1,
            ImageFormat.Rg8Unorm or ImageFormat.R16Unorm => 2,
            ImageFormat.Rgb8Unorm => 3,
            ImageFormat.Rgba8Unorm or ImageFormat.Rg16Unorm or ImageFormat.R32Float => 4,
            ImageFormat.Rgb16Unorm => 6,
            ImageFormat.Rgba16Unorm or ImageFormat.Rg32Float => 8,
            ImageFormat.Rgb32Float => 12,
            ImageFormat.Rgba32Float => 16,
            _ => 0
        };
    }
}
