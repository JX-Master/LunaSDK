using System;
using System.Runtime.InteropServices;
using System.Text;
using Luna.Runtime;
using Luna.RHI.Internal;
using Luna.Window;

namespace Luna.RHI;

internal sealed class RhiDevice : ObjectBase, IDevice
{
    private readonly IntPtr _idevice;

    internal RhiDevice(IntPtr nativeObject, IntPtr nativeDevice, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeDevice == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeDevice));
        }
        _idevice = nativeDevice;
    }

    public uint CommandQueueCount
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceGetNumCommandQueues(_idevice, out var count)));
            return count;
        }
    }

    public CommandQueueDesc GetCommandQueueDesc(uint index)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceGetCommandQueueDesc(_idevice, index, out var desc)));
        return new CommandQueueDesc((CommandQueueType)desc.Type, (CommandQueueFlags)desc.Flags);
    }

    public double GetCommandQueueTimestampFrequency(uint index)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceGetCommandQueueTimestampFrequency(_idevice, index, out var frequency)));
        return frequency;
    }

    public DeviceFeatureData CheckFeature(DeviceFeature feature)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceCheckFeature(_idevice, (uint)feature, out var rawValue)));
        return new DeviceFeatureData(rawValue);
    }

    public TextureDataPlacementInfo GetTextureDataPlacementInfo(uint width, uint height, uint depth, Format format)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceGetTextureDataPlacementInfo(
            _idevice,
            width,
            height,
            depth,
            (uint)format,
            out var info)));
        return info.ToPublic();
    }

    public bool IsResourcesAliasingCompatible(MemoryType memoryType, BufferDesc[] buffers, TextureDesc[] textures)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(buffers);
        ArgumentNullException.ThrowIfNull(textures);
        var nativeBuffers = ToNativeBufferDescs(buffers);
        var nativeTextures = ToNativeTextureDescs(textures);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceIsResourcesAliasingCompatible(
            _idevice,
            (uint)memoryType,
            nativeBuffers.Length == 0 ? null : nativeBuffers,
            (ulong)nativeBuffers.Length,
            nativeTextures.Length == 0 ? null : nativeTextures,
            (ulong)nativeTextures.Length,
            out var compatible)));
        return compatible != 0;
    }

    public IDeviceMemory AllocateMemory(MemoryType memoryType, BufferDesc[] buffers, TextureDesc[] textures)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(buffers);
        ArgumentNullException.ThrowIfNull(textures);
        var nativeBuffers = ToNativeBufferDescs(buffers);
        var nativeTextures = ToNativeTextureDescs(textures);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceAllocateMemory(
            _idevice,
            (uint)memoryType,
            nativeBuffers.Length == 0 ? null : nativeBuffers,
            (ulong)nativeBuffers.Length,
            nativeTextures.Length == 0 ? null : nativeTextures,
            (ulong)nativeTextures.Length,
            out var memory)));
        return new RhiDeviceMemory(memory.Object, memory.IDeviceMemory, retain: false);
    }

    public IBuffer CreateBuffer(MemoryType memoryType, BufferDesc desc)
    {
        EnsureNotDisposed();
        var nativeDesc = NativeBufferDesc.FromPublic(desc);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewBuffer(_idevice, (uint)memoryType, in nativeDesc, out var buffer)));
        return new RhiBuffer(buffer.Object, buffer.IBuffer, retain: false);
    }

    public ITexture CreateTexture(MemoryType memoryType, TextureDesc desc)
    {
        EnsureNotDisposed();
        var nativeDesc = NativeTextureDesc.FromPublic(desc);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewTexture(_idevice, (uint)memoryType, in nativeDesc, out var texture)));
        return new RhiTexture(texture.Object, texture.ITexture, retain: false);
    }

    public ITexture CreateTexture(MemoryType memoryType, TextureDesc desc, ClearValue optimizedClearValue)
    {
        EnsureNotDisposed();
        var nativeDesc = NativeTextureDesc.FromPublic(desc);
        var nativeClearValue = new NativeClearValue(optimizedClearValue);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewTextureWithClearValue(
            _idevice,
            (uint)memoryType,
            in nativeDesc,
            in nativeClearValue,
            out var texture)));
        return new RhiTexture(texture.Object, texture.ITexture, retain: false);
    }

    public IBuffer CreateAliasingBuffer(IDeviceMemory deviceMemory, BufferDesc desc)
    {
        EnsureNotDisposed();
        var nativeDesc = NativeBufferDesc.FromPublic(desc);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewAliasingBuffer(
            _idevice,
            RhiDeviceMemory.GetNativeDeviceMemoryPointer(deviceMemory),
            in nativeDesc,
            out var buffer)));
        return new RhiBuffer(buffer.Object, buffer.IBuffer, retain: false);
    }

    public ITexture CreateAliasingTexture(IDeviceMemory deviceMemory, TextureDesc desc)
    {
        EnsureNotDisposed();
        var nativeDesc = NativeTextureDesc.FromPublic(desc);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewAliasingTexture(
            _idevice,
            RhiDeviceMemory.GetNativeDeviceMemoryPointer(deviceMemory),
            in nativeDesc,
            out var texture)));
        return new RhiTexture(texture.Object, texture.ITexture, retain: false);
    }

    public ITexture CreateAliasingTexture(IDeviceMemory deviceMemory, TextureDesc desc, ClearValue optimizedClearValue)
    {
        EnsureNotDisposed();
        var nativeDesc = NativeTextureDesc.FromPublic(desc);
        var nativeClearValue = new NativeClearValue(optimizedClearValue);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewAliasingTextureWithClearValue(
            _idevice,
            RhiDeviceMemory.GetNativeDeviceMemoryPointer(deviceMemory),
            in nativeDesc,
            in nativeClearValue,
            out var texture)));
        return new RhiTexture(texture.Object, texture.ITexture, retain: false);
    }

    public IFence CreateFence()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewFence(_idevice, out var fence)));
        return new RhiFence(fence.Object, fence.IFence, retain: false);
    }

    public IQueryHeap CreateQueryHeap(QueryHeapDesc desc)
    {
        EnsureNotDisposed();
        var nativeDesc = new NativeQueryHeapDesc(desc);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewQueryHeap(_idevice, in nativeDesc, out var queryHeap)));
        return new RhiQueryHeap(queryHeap.Object, queryHeap.IQueryHeap, retain: false);
    }

    public IDescriptorSetLayout CreateDescriptorSetLayout(DescriptorSetLayoutDesc desc)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(desc);
        var nativeBindings = new NativeDescriptorSetLayoutBinding[desc.Bindings.Length];
        for (var i = 0; i < nativeBindings.Length; ++i)
        {
            nativeBindings[i] = new NativeDescriptorSetLayoutBinding(desc.Bindings[i]);
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewDescriptorSetLayout(
            _idevice,
            nativeBindings,
            (ulong)nativeBindings.Length,
            (uint)desc.Flags,
            out var descriptorSetLayout)));
        return new RhiDescriptorSetLayout(descriptorSetLayout.Object, descriptorSetLayout.IDescriptorSetLayout, retain: false);
    }

    public IDescriptorSet CreateDescriptorSet(DescriptorSetDesc desc)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewDescriptorSet(
            _idevice,
            RhiDescriptorSetLayout.GetNativeDescriptorSetLayoutPointer(desc.Layout),
            desc.NumVariableDescriptors,
            out var descriptorSet)));
        return new RhiDescriptorSet(descriptorSet.Object, descriptorSet.IDescriptorSet, retain: false);
    }

    public IPipelineLayout CreatePipelineLayout(PipelineLayoutDesc desc)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(desc);
        var descriptorSetLayouts = new IntPtr[desc.DescriptorSetLayouts.Length];
        for (var i = 0; i < descriptorSetLayouts.Length; ++i)
        {
            descriptorSetLayouts[i] = RhiDescriptorSetLayout.GetNativeDescriptorSetLayoutPointer(desc.DescriptorSetLayouts[i]);
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewPipelineLayout(
            _idevice,
            descriptorSetLayouts,
            (ulong)descriptorSetLayouts.Length,
            (uint)desc.Flags,
            out var pipelineLayout)));
        return new RhiPipelineLayout(pipelineLayout.Object, pipelineLayout.IPipelineLayout, retain: false);
    }

    public IPipelineState CreateGraphicsPipelineState(GraphicsPipelineStateDesc desc)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(desc);
        if (desc.PipelineLayout is null)
        {
            throw new ArgumentException("PipelineLayout must be set.", nameof(desc));
        }
        if (desc.ColorFormats.Length > 8)
        {
            throw new ArgumentOutOfRangeException(nameof(desc), "A graphics pipeline can use at most 8 color attachments.");
        }

        var nativeBindings = new NativeInputBindingDesc[desc.InputBindings.Length];
        for (var i = 0; i < nativeBindings.Length; ++i)
        {
            nativeBindings[i] = new NativeInputBindingDesc(desc.InputBindings[i]);
        }

        var nativeAttributes = new NativeInputAttributeDesc[desc.InputAttributes.Length];
        GCHandle bindingsHandle = default;
        GCHandle attributesHandle = default;
        GCHandle vertexShaderHandle = default;
        GCHandle pixelShaderHandle = default;
        var vertexEntryPoint = IntPtr.Zero;
        var pixelEntryPoint = IntPtr.Zero;
        try
        {
            for (var i = 0; i < desc.InputAttributes.Length; ++i)
            {
                nativeAttributes[i] = new NativeInputAttributeDesc(desc.InputAttributes[i]);
            }

            bindingsHandle = nativeBindings.Length == 0 ? default : GCHandle.Alloc(nativeBindings, GCHandleType.Pinned);
            attributesHandle = nativeAttributes.Length == 0 ? default : GCHandle.Alloc(nativeAttributes, GCHandleType.Pinned);
            vertexShaderHandle = desc.VertexShader.Data.Length == 0 ? default : GCHandle.Alloc(desc.VertexShader.Data, GCHandleType.Pinned);
            pixelShaderHandle = desc.PixelShader.Data.Length == 0 ? default : GCHandle.Alloc(desc.PixelShader.Data, GCHandleType.Pinned);
            vertexEntryPoint = StringToNativeUtf8(desc.VertexShader.EntryPoint);
            pixelEntryPoint = StringToNativeUtf8(desc.PixelShader.EntryPoint);

            var nativeDesc = new NativeGraphicsPipelineStateDesc
            {
                InputBindings = bindingsHandle.IsAllocated ? bindingsHandle.AddrOfPinnedObject() : IntPtr.Zero,
                InputBindingCount = (ulong)nativeBindings.Length,
                InputAttributes = attributesHandle.IsAllocated ? attributesHandle.AddrOfPinnedObject() : IntPtr.Zero,
                InputAttributeCount = (ulong)nativeAttributes.Length,
                PipelineLayout = RhiPipelineLayout.GetNativePipelineLayoutPointer(desc.PipelineLayout),
                VertexShader = new NativeShaderData(
                    vertexShaderHandle.IsAllocated ? vertexShaderHandle.AddrOfPinnedObject() : IntPtr.Zero,
                    (ulong)desc.VertexShader.Data.Length,
                    vertexEntryPoint,
                    desc.VertexShader.Format),
                PixelShader = new NativeShaderData(
                    pixelShaderHandle.IsAllocated ? pixelShaderHandle.AddrOfPinnedObject() : IntPtr.Zero,
                    (ulong)desc.PixelShader.Data.Length,
                    pixelEntryPoint,
                    desc.PixelShader.Format),
                RasterizerState = new NativeRasterizerDesc(desc.RasterizerState),
                DepthStencilState = new NativeDepthStencilDesc(desc.DepthStencilState),
                BlendState = new NativeBlendDesc(desc.BlendState),
                IndexBufferStripCutValue = (uint)desc.IndexBufferStripCutValue,
                PrimitiveTopology = (uint)desc.PrimitiveTopology,
                NumColorAttachments = checked((byte)desc.ColorFormats.Length),
                DepthStencilFormat = (uint)desc.DepthStencilFormat,
                SampleCount = desc.SampleCount
            };
            for (var i = 0; i < desc.ColorFormats.Length; ++i)
            {
                nativeDesc.ColorFormats[i] = (uint)desc.ColorFormats[i];
            }

            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewGraphicsPipelineState(_idevice, nativeDesc, out var pipelineState)));
            return new RhiPipelineState(pipelineState.Object, pipelineState.IPipelineState, retain: false);
        }
        finally
        {
            if (bindingsHandle.IsAllocated) bindingsHandle.Free();
            if (attributesHandle.IsAllocated) attributesHandle.Free();
            if (vertexShaderHandle.IsAllocated) vertexShaderHandle.Free();
            if (pixelShaderHandle.IsAllocated) pixelShaderHandle.Free();
            if (vertexEntryPoint != IntPtr.Zero) Marshal.FreeHGlobal(vertexEntryPoint);
            if (pixelEntryPoint != IntPtr.Zero) Marshal.FreeHGlobal(pixelEntryPoint);
        }
    }

    public IPipelineState CreateComputePipelineState(ComputePipelineStateDesc desc)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(desc);
        if (desc.PipelineLayout is null)
        {
            throw new ArgumentException("PipelineLayout must be set.", nameof(desc));
        }
        if (desc.ComputeShader.Data is null)
        {
            throw new ArgumentException("ComputeShader must be set.", nameof(desc));
        }

        GCHandle computeShaderHandle = default;
        var computeEntryPoint = IntPtr.Zero;
        try
        {
            computeShaderHandle = desc.ComputeShader.Data.Length == 0 ? default : GCHandle.Alloc(desc.ComputeShader.Data, GCHandleType.Pinned);
            computeEntryPoint = StringToNativeUtf8(desc.ComputeShader.EntryPoint);

            var nativeDesc = new NativeComputePipelineStateDesc
            {
                PipelineLayout = RhiPipelineLayout.GetNativePipelineLayoutPointer(desc.PipelineLayout),
                ComputeShader = new NativeShaderData(
                    computeShaderHandle.IsAllocated ? computeShaderHandle.AddrOfPinnedObject() : IntPtr.Zero,
                    (ulong)desc.ComputeShader.Data.Length,
                    computeEntryPoint,
                    desc.ComputeShader.Format),
                MetalNumThreadsX = desc.MetalNumThreadsX,
                MetalNumThreadsY = desc.MetalNumThreadsY,
                MetalNumThreadsZ = desc.MetalNumThreadsZ
            };

            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewComputePipelineState(_idevice, nativeDesc, out var pipelineState)));
            return new RhiPipelineState(pipelineState.Object, pipelineState.IPipelineState, retain: false);
        }
        finally
        {
            if (computeShaderHandle.IsAllocated) computeShaderHandle.Free();
            if (computeEntryPoint != IntPtr.Zero) Marshal.FreeHGlobal(computeEntryPoint);
        }
    }

    private static IntPtr StringToNativeUtf8(string value)
    {
        ArgumentNullException.ThrowIfNull(value);
        var bytes = Encoding.UTF8.GetBytes(value);
        var native = Marshal.AllocHGlobal(bytes.Length + 1);
        Marshal.Copy(bytes, 0, native, bytes.Length);
        Marshal.WriteByte(native, bytes.Length, 0);
        return native;
    }

    private static NativeBufferDesc[] ToNativeBufferDescs(BufferDesc[] descs)
    {
        var nativeDescs = new NativeBufferDesc[descs.Length];
        for (var i = 0; i < nativeDescs.Length; ++i)
        {
            nativeDescs[i] = NativeBufferDesc.FromPublic(descs[i]);
        }
        return nativeDescs;
    }

    private static NativeTextureDesc[] ToNativeTextureDescs(TextureDesc[] descs)
    {
        var nativeDescs = new NativeTextureDesc[descs.Length];
        for (var i = 0; i < nativeDescs.Length; ++i)
        {
            nativeDescs[i] = NativeTextureDesc.FromPublic(descs[i]);
        }
        return nativeDescs;
    }

    public ISwapChain CreateSwapChain(uint commandQueueIndex, IWindow window, SwapChainDesc desc)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(window);
        var nativeDesc = NativeSwapChainDesc.FromPublic(desc);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewSwapChain(
            _idevice,
            commandQueueIndex,
            window.GetNativeHandle(),
            in nativeDesc,
            out var swapChain)));
        return new RhiSwapChain(swapChain.Object, swapChain.ISwapChain, retain: false);
    }

    public ICommandBuffer CreateCommandBuffer(uint commandQueueIndex)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceNewCommandBuffer(_idevice, commandQueueIndex, out var commandBuffer)));
        return new RhiCommandBuffer(commandBuffer.Object, commandBuffer.ICommandBuffer, retain: false);
    }
}
