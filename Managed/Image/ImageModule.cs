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
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ImageNativeGenerated.InitModule()));
    }

    public static ImageDesc ReadFileDesc(byte[] fileData)
    {
        ArgumentNullException.ThrowIfNull(fileData);
        using var pinnedData = PinnedByteArray.Create(fileData);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ImageNativeGenerated.ReadFileDesc(pinnedData.Pointer, (ulong)fileData.Length, out var desc)));
        return ToManaged(desc);
    }

    public static ImageDesc ReadFileDesc(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        return ReadFileDesc(RuntimeFile.LoadData(path));
    }

    public static ImageDesc ReadFileDesc(IFile file)
    {
        ArgumentNullException.ThrowIfNull(file);
        return ReadFileDesc(RuntimeFile.LoadData(file));
    }

    public static ImageData ReadFile(byte[] fileData, ImageFormat desiredFormat)
    {
        ArgumentNullException.ThrowIfNull(fileData);
        using var pinnedData = PinnedByteArray.Create(fileData);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ImageNativeGenerated.ReadFile(pinnedData.Pointer, (ulong)fileData.Length, (uint)desiredFormat, out var nativeImage)));
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
            return new ImageData(data, ToManaged(nativeImage.Desc));
        }
        finally
        {
            ImageNativeGenerated.FreeData(ref nativeImage);
        }
    }

    public static ImageData ReadFile(string path, ImageFormat desiredFormat)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        return ReadFile(RuntimeFile.LoadData(path), desiredFormat);
    }

    public static ImageData ReadFile(IFile file, ImageFormat desiredFormat)
    {
        ArgumentNullException.ThrowIfNull(file);
        return ReadFile(RuntimeFile.LoadData(file), desiredFormat);
    }

    public static void WritePng(ISeekableStream stream, ImageData image)
    {
        ArgumentNullException.ThrowIfNull(image);
        WritePng(stream, image.Desc, image.Data);
    }

    public static void WritePng(ISeekableStream stream, ImageDesc desc, byte[] data)
    {
        Write(stream, desc, data, ImageNativeGenerated.WritePngFile);
    }

    public static void WritePng(string path, ImageData image)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        ArgumentNullException.ThrowIfNull(image);
        using var file = OpenOutputFile(path);
        WritePng(file, image);
        file.Flush();
    }

    public static void WriteBmp(ISeekableStream stream, ImageData image)
    {
        ArgumentNullException.ThrowIfNull(image);
        WriteBmp(stream, image.Desc, image.Data);
    }

    public static void WriteBmp(ISeekableStream stream, ImageDesc desc, byte[] data)
    {
        Write(stream, desc, data, ImageNativeGenerated.WriteBmpFile);
    }

    public static void WriteBmp(string path, ImageData image)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        ArgumentNullException.ThrowIfNull(image);
        using var file = OpenOutputFile(path);
        WriteBmp(file, image);
        file.Flush();
    }

    public static void WriteTga(ISeekableStream stream, ImageData image)
    {
        ArgumentNullException.ThrowIfNull(image);
        WriteTga(stream, image.Desc, image.Data);
    }

    public static void WriteTga(ISeekableStream stream, ImageDesc desc, byte[] data)
    {
        Write(stream, desc, data, ImageNativeGenerated.WriteTgaFile);
    }

    public static void WriteTga(string path, ImageData image)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        ArgumentNullException.ThrowIfNull(image);
        using var file = OpenOutputFile(path);
        WriteTga(file, image);
        file.Flush();
    }

    public static void WriteJpg(ISeekableStream stream, ImageData image, uint quality)
    {
        ArgumentNullException.ThrowIfNull(image);
        WriteJpg(stream, image.Desc, image.Data, quality);
    }

    public static void WriteJpg(ISeekableStream stream, ImageDesc desc, byte[] data, uint quality)
    {
        ValidateImageData(desc, data);
        if (quality is < 1 or > 100)
        {
            throw new ArgumentOutOfRangeException(nameof(quality), "JPEG quality must be between 1 and 100.");
        }
        var nativeStream = GetNativeHandle(stream);
        var nativeDesc = ToNative(desc);
        using var pinnedData = PinnedByteArray.Create(data);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ImageNativeGenerated.WriteJpgFile(nativeStream, in nativeDesc, pinnedData.Pointer, (ulong)data.Length, quality)));
    }

    public static void WriteJpg(string path, ImageData image, uint quality)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        ArgumentNullException.ThrowIfNull(image);
        using var file = OpenOutputFile(path);
        WriteJpg(file, image, quality);
        file.Flush();
    }

    public static void WriteHdr(ISeekableStream stream, ImageData image)
    {
        ArgumentNullException.ThrowIfNull(image);
        WriteHdr(stream, image.Desc, image.Data);
    }

    public static void WriteHdr(ISeekableStream stream, ImageDesc desc, byte[] data)
    {
        Write(stream, desc, data, ImageNativeGenerated.WriteHdrFile);
    }

    public static void WriteHdr(string path, ImageData image)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        ArgumentNullException.ThrowIfNull(image);
        using var file = OpenOutputFile(path);
        WriteHdr(file, image);
        file.Flush();
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

    private delegate UIntPtr ImageWriteCallback(IntPtr stream, in NativeImageDesc desc, IntPtr data, ulong dataSize);

    private static void Write(ISeekableStream stream, ImageDesc desc, byte[] data, ImageWriteCallback callback)
    {
        ValidateImageData(desc, data);
        var nativeStream = GetNativeHandle(stream);
        var nativeDesc = ToNative(desc);
        using var pinnedData = PinnedByteArray.Create(data);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(callback(nativeStream, in nativeDesc, pinnedData.Pointer, (ulong)data.Length)));
    }

    private static void ValidateImageData(ImageDesc desc, byte[] data)
    {
        ArgumentNullException.ThrowIfNull(data);
        var pixelSize = PixelSize(desc.Format);
        if (pixelSize == 0)
        {
            throw new ArgumentException("The image format is not supported.", nameof(desc));
        }
        var requiredSize = checked((ulong)pixelSize * desc.Width * desc.Height);
        if ((ulong)data.Length != requiredSize)
        {
            throw new ArgumentException($"The image data size ({data.Length}) does not match the image description requirement ({requiredSize}).", nameof(data));
        }
    }

    private static IntPtr GetNativeHandle(ISeekableStream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);
        if (stream is not IObject obj)
        {
            throw new ArgumentException("The stream was not created by the Luna.Runtime binding.", nameof(stream));
        }
        return obj.GetNativeHandle();
    }

    private static IFile OpenOutputFile(string path)
    {
        return RuntimeFile.Open(path, FileOpenFlags.Write | FileOpenFlags.UserBuffering, FileCreationMode.CreateAlways);
    }

    private static ImageDesc ToManaged(NativeImageDesc desc)
    {
        return new ImageDesc((ImageFormat)desc.Format, desc.Width, desc.Height);
    }

    private static NativeImageDesc ToNative(ImageDesc desc)
    {
        return new NativeImageDesc
        {
            Format = (uint)desc.Format,
            Width = desc.Width,
            Height = desc.Height
        };
    }

    private readonly struct PinnedByteArray : IDisposable
    {
        private readonly GCHandle m_handle;

        public IntPtr Pointer { get; }

        private PinnedByteArray(GCHandle handle, IntPtr pointer)
        {
            m_handle = handle;
            Pointer = pointer;
        }

        public static PinnedByteArray Create(byte[] data)
        {
            if (data.Length == 0)
            {
                return new PinnedByteArray(default, IntPtr.Zero);
            }
            var handle = GCHandle.Alloc(data, GCHandleType.Pinned);
            return new PinnedByteArray(handle, handle.AddrOfPinnedObject());
        }

        public void Dispose()
        {
            if (m_handle.IsAllocated)
            {
                m_handle.Free();
            }
        }
    }
}
