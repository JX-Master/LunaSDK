using System;
using System.Runtime.InteropServices;

namespace Luna.Image.Internal;

internal static class ImageNative
{
    private const string LibraryName = "LunaImageC";

    [DllImport(LibraryName, EntryPoint = "luna_image_init_module")]
    internal static extern UIntPtr InitModule();

    [DllImport(LibraryName, EntryPoint = "luna_image_read_file_desc")]
    internal static extern UIntPtr ReadFileDesc(
        [In] byte[] data,
        ulong dataSize,
        out NativeImageDesc outDesc);

    [DllImport(LibraryName, EntryPoint = "luna_image_read_file")]
    internal static extern UIntPtr ReadFile(
        [In] byte[] data,
        ulong dataSize,
        uint desiredFormat,
        out NativeImageData outImage);

    [DllImport(LibraryName, EntryPoint = "luna_image_write_png_file")]
    internal static extern UIntPtr WritePngFile(
        IntPtr stream,
        in NativeImageDesc desc,
        [In] byte[] data,
        ulong dataSize);

    [DllImport(LibraryName, EntryPoint = "luna_image_write_bmp_file")]
    internal static extern UIntPtr WriteBmpFile(
        IntPtr stream,
        in NativeImageDesc desc,
        [In] byte[] data,
        ulong dataSize);

    [DllImport(LibraryName, EntryPoint = "luna_image_write_tga_file")]
    internal static extern UIntPtr WriteTgaFile(
        IntPtr stream,
        in NativeImageDesc desc,
        [In] byte[] data,
        ulong dataSize);

    [DllImport(LibraryName, EntryPoint = "luna_image_write_jpg_file")]
    internal static extern UIntPtr WriteJpgFile(
        IntPtr stream,
        in NativeImageDesc desc,
        [In] byte[] data,
        ulong dataSize,
        uint quality);

    [DllImport(LibraryName, EntryPoint = "luna_image_write_hdr_file")]
    internal static extern UIntPtr WriteHdrFile(
        IntPtr stream,
        in NativeImageDesc desc,
        [In] byte[] data,
        ulong dataSize);

    [DllImport(LibraryName, EntryPoint = "luna_image_free_data")]
    internal static extern void FreeData(ref NativeImageData image);
}
