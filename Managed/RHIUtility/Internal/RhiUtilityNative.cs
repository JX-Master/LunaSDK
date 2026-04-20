using System;
using System.Runtime.InteropServices;
using Luna.RHI.Internal;

namespace Luna.RHIUtility.Internal;

internal static class RhiUtilityNative
{
    private const string LibraryName = "LunaRHIUtilityC";

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_init_module")]
    internal static extern UIntPtr InitModule();

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_new_resource_write_context")]
    internal static extern UIntPtr NewResourceWriteContext(
        IntPtr deviceObject,
        out NativeRhiUtilityResourceWriteContextHandle outContext);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_new_resource_read_context")]
    internal static extern UIntPtr NewResourceReadContext(
        IntPtr deviceObject,
        out NativeRhiUtilityResourceReadContextHandle outContext);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_new_blit_context")]
    internal static extern UIntPtr NewBlitContext(
        IntPtr deviceObject,
        uint destinationFormat,
        out NativeRhiUtilityBlitContextHandle outContext);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_new_mipmap_generation_context")]
    internal static extern UIntPtr NewMipmapGenerationContext(
        IntPtr deviceObject,
        out NativeRhiUtilityMipmapGenerationContextHandle outContext);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_resource_write_context_reset")]
    internal static extern UIntPtr ResourceWriteContextReset(IntPtr context);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_resource_write_context_write_buffer")]
    internal static extern UIntPtr ResourceWriteContextWriteBuffer(
        IntPtr context,
        IntPtr bufferObject,
        ulong offset,
        [In] byte[]? data,
        ulong dataSize);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_resource_write_context_write_texture")]
    internal static extern UIntPtr ResourceWriteContextWriteTexture(
        IntPtr context,
        IntPtr textureObject,
        NativeSubresourceIndex subresource,
        uint x,
        uint y,
        uint z,
        uint width,
        uint height,
        uint depth,
        [In] byte[] data,
        ulong dataSize,
        uint sourceRowPitch,
        uint sourceSlicePitch,
        uint copyBytesPerRow,
        out NativeRhiUtilityTextureWriteInfo outInfo);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_resource_write_context_commit")]
    internal static extern UIntPtr ResourceWriteContextCommit(
        IntPtr context,
        IntPtr commandBufferObject,
        int submitAndWait);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_resource_read_context_reset")]
    internal static extern UIntPtr ResourceReadContextReset(IntPtr context);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_resource_read_context_read_buffer")]
    internal static extern UIntPtr ResourceReadContextReadBuffer(
        IntPtr context,
        IntPtr bufferObject,
        ulong offset,
        ulong dataSize,
        out ulong outHandle);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_resource_read_context_read_texture")]
    internal static extern UIntPtr ResourceReadContextReadTexture(
        IntPtr context,
        IntPtr textureObject,
        NativeSubresourceIndex subresource,
        uint x,
        uint y,
        uint z,
        uint width,
        uint height,
        uint depth,
        out ulong outHandle);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_resource_read_context_commit")]
    internal static extern UIntPtr ResourceReadContextCommit(
        IntPtr context,
        IntPtr commandBufferObject,
        int submitAndWait);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_resource_read_context_get_buffer_data")]
    internal static extern UIntPtr ResourceReadContextGetBufferData(
        IntPtr context,
        ulong handle,
        [Out] byte[]? outData,
        ulong dataSize);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_resource_read_context_get_texture_data")]
    internal static extern UIntPtr ResourceReadContextGetTextureData(
        IntPtr context,
        ulong handle,
        uint copyBytesPerRow,
        uint height,
        uint depth,
        [Out] byte[]? outData,
        ulong dataSize,
        out NativeRhiUtilityTextureReadInfo outInfo);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_blit_context_reset")]
    internal static extern UIntPtr BlitContextReset(IntPtr context);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_blit_context_blit")]
    internal static extern UIntPtr BlitContextBlit(
        IntPtr context,
        IntPtr destinationTextureObject,
        NativeSubresourceIndex destinationSubresource,
        in NativeTextureViewDesc sourceView,
        in NativeSamplerDesc sampler,
        float topLeftX,
        float topLeftY,
        float topRightX,
        float topRightY,
        float bottomLeftX,
        float bottomLeftY,
        float bottomRightX,
        float bottomRightY);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_blit_context_commit")]
    internal static extern UIntPtr BlitContextCommit(
        IntPtr context,
        IntPtr commandBufferObject,
        int submitAndWait);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_mipmap_generation_context_reset")]
    internal static extern UIntPtr MipmapGenerationContextReset(IntPtr context);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_mipmap_generation_context_generate_mipmaps")]
    internal static extern UIntPtr MipmapGenerationContextGenerateMipmaps(
        IntPtr context,
        IntPtr textureObject,
        uint sourceMip,
        uint numGenerateMips);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_utility_mipmap_generation_context_commit")]
    internal static extern UIntPtr MipmapGenerationContextCommit(
        IntPtr context,
        IntPtr commandBufferObject,
        int submitAndWait);
}
