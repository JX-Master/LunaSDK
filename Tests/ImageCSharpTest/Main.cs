using System;
using System.IO;
using Luna.Image;
using Luna.Runtime;

Runtime.Init();

try
{
    ImageModule.Init();

    if (RuntimeErrors.GetCategoryName(Errors.Category) != "ImageError")
    {
        throw new InvalidOperationException("Image error category lookup failed.");
    }
    if (RuntimeErrors.GetCodeName(Errors.FileParseError) != "file_parse_error")
    {
        throw new InvalidOperationException("Image file_parse_error lookup failed.");
    }

    var sourcePath = Path.Combine(AppContext.BaseDirectory, "uv_checker.png");
    var sourceBytes = RuntimeFile.LoadData(sourcePath);
    var descFromPath = ImageModule.ReadFileDesc(sourcePath);
    if (descFromPath.Width == 0 || descFromPath.Height == 0)
    {
        throw new InvalidOperationException("Image file description should contain non-zero dimensions.");
    }
    var descFromBytes = ImageModule.ReadFileDesc(sourceBytes);
    if (descFromBytes != descFromPath)
    {
        throw new InvalidOperationException("Image file descriptions should match across path and byte[] overloads.");
    }

    using var readFile = RuntimeFile.Open(sourcePath, FileOpenFlags.Read | FileOpenFlags.UserBuffering, FileCreationMode.OpenExisting);
    var descFromFile = ImageModule.ReadFileDesc(readFile);
    if (descFromFile != descFromPath)
    {
        throw new InvalidOperationException("Image file descriptions should match across path and file overloads.");
    }

    var sourceImage = ImageModule.ReadFile(sourcePath, ImageFormat.Rgba8Unorm);
    if (sourceImage.Data.Length != checked((int)(ImageModule.PixelSize(sourceImage.Desc.Format) * sourceImage.Desc.Width * sourceImage.Desc.Height)))
    {
        throw new InvalidOperationException("Loaded image data size does not match the declared image description.");
    }
    var sourceImageFromBytes = ImageModule.ReadFile(sourceBytes, ImageFormat.Rgba8Unorm);
    if (!sourceImageFromBytes.Desc.Equals(sourceImage.Desc) || !sourceImageFromBytes.Data.AsSpan().SequenceEqual(sourceImage.Data))
    {
        throw new InvalidOperationException("Image pixel data should match across path and byte[] overloads.");
    }

    using var readFileForPixels = RuntimeFile.Open(sourcePath, FileOpenFlags.Read | FileOpenFlags.UserBuffering, FileCreationMode.OpenExisting);
    var sourceImageFromFile = ImageModule.ReadFile(readFileForPixels, ImageFormat.Rgba8Unorm);
    if (!sourceImageFromFile.Desc.Equals(sourceImage.Desc) || !sourceImageFromFile.Data.AsSpan().SequenceEqual(sourceImage.Data))
    {
        throw new InvalidOperationException("Image file pixel data should match across path and file overloads.");
    }

    var testDirectory = Path.Combine(Path.GetTempPath(), $"LunaImageCSharpTest-{System.Guid.NewGuid():N}");
    Directory.CreateDirectory(testDirectory);
    try
    {
        var pngPath = Path.Combine(testDirectory, "roundtrip.png");
        var bmpPath = Path.Combine(testDirectory, "roundtrip.bmp");
        var tgaPath = Path.Combine(testDirectory, "roundtrip.tga");
        var jpgPath = Path.Combine(testDirectory, "roundtrip.jpg");
        var hdrPath = Path.Combine(testDirectory, "roundtrip.hdr");
        var pngStreamPath = Path.Combine(testDirectory, "roundtrip-stream.png");
        var bmpPathByPathOverload = Path.Combine(testDirectory, "roundtrip-path.bmp");
        var tgaPathByPathOverload = Path.Combine(testDirectory, "roundtrip-path.tga");
        var jpgStreamPath = Path.Combine(testDirectory, "roundtrip-stream.jpg");
        var hdrPathByPathOverload = Path.Combine(testDirectory, "roundtrip-path.hdr");

        ImageModule.WritePng(pngPath, sourceImage);
        using (var stream = RuntimeFile.Open(pngStreamPath, FileOpenFlags.Write | FileOpenFlags.UserBuffering, FileCreationMode.CreateAlways))
        {
            ImageModule.WritePng(stream, sourceImage.Desc, sourceImage.Data);
            stream.Flush();
        }
        using (var stream = RuntimeFile.Open(bmpPath, FileOpenFlags.Write | FileOpenFlags.UserBuffering, FileCreationMode.CreateAlways))
        {
            ImageModule.WriteBmp(stream, sourceImage);
            stream.Flush();
        }
        ImageModule.WriteBmp(bmpPathByPathOverload, sourceImage);
        using (var stream = RuntimeFile.Open(tgaPath, FileOpenFlags.Write | FileOpenFlags.UserBuffering, FileCreationMode.CreateAlways))
        {
            ImageModule.WriteTga(stream, sourceImage.Desc, sourceImage.Data);
            stream.Flush();
        }
        ImageModule.WriteTga(tgaPathByPathOverload, sourceImage);
        ImageModule.WriteJpg(jpgPath, sourceImage, 90);
        using (var stream = RuntimeFile.Open(jpgStreamPath, FileOpenFlags.Write | FileOpenFlags.UserBuffering, FileCreationMode.CreateAlways))
        {
            ImageModule.WriteJpg(stream, sourceImage.Desc, sourceImage.Data, 90);
            stream.Flush();
        }

        var hdrImage = CreateHdrImage();
        using (var stream = RuntimeFile.Open(hdrPath, FileOpenFlags.Write | FileOpenFlags.UserBuffering, FileCreationMode.CreateAlways))
        {
            ImageModule.WriteHdr(stream, hdrImage);
            stream.Flush();
        }
        ImageModule.WriteHdr(hdrPathByPathOverload, hdrImage);

        ValidateLosslessRoundtrip(pngPath, sourceImage, "PNG");
        ValidateLosslessRoundtrip(pngStreamPath, sourceImage, "PNG(stream)");
        ValidateLosslessRoundtrip(bmpPath, sourceImage, "BMP");
        ValidateLosslessRoundtrip(bmpPathByPathOverload, sourceImage, "BMP(path)");
        ValidateLosslessRoundtrip(tgaPath, sourceImage, "TGA");
        ValidateLosslessRoundtrip(tgaPathByPathOverload, sourceImage, "TGA(path)");
        ValidateJpgRoundtrip(jpgPath, sourceImage.Desc);
        ValidateJpgRoundtrip(jpgStreamPath, sourceImage.Desc);
        ValidateHdrRoundtrip(hdrPath, hdrImage);
        ValidateHdrRoundtrip(hdrPathByPathOverload, hdrImage);
    }
    finally
    {
        if (Directory.Exists(testDirectory))
        {
            foreach (var file in Directory.EnumerateFiles(testDirectory))
            {
                RuntimeFile.Delete(file);
            }
            RuntimeFile.Delete(testDirectory);
        }
    }

    Console.WriteLine("ImageCSharpTest passed.");
}
finally
{
    Runtime.Close();
}

static void ValidateLosslessRoundtrip(string path, ImageData original, string label)
{
    var roundtrip = ImageModule.ReadFile(path, original.Desc.Format);
    if (!roundtrip.Desc.Equals(original.Desc))
    {
        throw new InvalidOperationException($"{label} roundtrip description mismatch.");
    }
    if (!roundtrip.Data.AsSpan().SequenceEqual(original.Data))
    {
        throw new InvalidOperationException($"{label} roundtrip data mismatch.");
    }
}

static void ValidateJpgRoundtrip(string path, ImageDesc expectedDesc)
{
    var roundtrip = ImageModule.ReadFile(path, ImageFormat.Rgba8Unorm);
    if (roundtrip.Desc.Width != expectedDesc.Width || roundtrip.Desc.Height != expectedDesc.Height)
    {
        throw new InvalidOperationException("JPEG roundtrip dimensions mismatch.");
    }
    if (roundtrip.Data.Length != checked((int)(ImageModule.PixelSize(roundtrip.Desc.Format) * roundtrip.Desc.Width * roundtrip.Desc.Height)))
    {
        throw new InvalidOperationException("JPEG roundtrip data size mismatch.");
    }
}

static void ValidateHdrRoundtrip(string path, ImageData original)
{
    var roundtripDesc = ImageModule.ReadFileDesc(path);
    if (roundtripDesc.Width != original.Desc.Width || roundtripDesc.Height != original.Desc.Height)
    {
        throw new InvalidOperationException("HDR roundtrip dimensions mismatch.");
    }

    var roundtrip = ImageModule.ReadFile(path, ImageFormat.Rgb32Float);
    if (roundtrip.Data.Length != original.Data.Length)
    {
        throw new InvalidOperationException("HDR roundtrip data size mismatch.");
    }

    var expected = new float[original.Data.Length / sizeof(float)];
    var actual = new float[roundtrip.Data.Length / sizeof(float)];
    Buffer.BlockCopy(original.Data, 0, expected, 0, original.Data.Length);
    Buffer.BlockCopy(roundtrip.Data, 0, actual, 0, roundtrip.Data.Length);
    for (var i = 0; i < expected.Length; ++i)
    {
        if (Math.Abs(expected[i] - actual[i]) > 0.01f)
        {
            throw new InvalidOperationException($"HDR roundtrip value mismatch at {i}: expected {expected[i]}, got {actual[i]}.");
        }
    }
}

static ImageData CreateHdrImage()
{
    var pixels = new float[]
    {
        0.0f, 0.5f, 1.0f,
        1.5f, 0.25f, 0.75f,
        0.1f, 2.0f, 0.3f,
        1.2f, 1.4f, 1.6f
    };
    var data = new byte[pixels.Length * sizeof(float)];
    Buffer.BlockCopy(pixels, 0, data, 0, data.Length);
    return new ImageData(data, new ImageDesc(ImageFormat.Rgb32Float, 2, 2));
}
