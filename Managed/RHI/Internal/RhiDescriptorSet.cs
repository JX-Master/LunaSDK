using System;
using Luna.Runtime;
using Luna.RHI.Internal;

namespace Luna.RHI;

internal sealed class RhiDescriptorSet : RhiDeviceChild, IDescriptorSet
{
    private readonly IntPtr _idescriptorSet;

    internal RhiDescriptorSet(IntPtr nativeObject, IntPtr nativeDescriptorSet, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeDescriptorSet == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeDescriptorSet));
        }
        _idescriptorSet = nativeDescriptorSet;
    }

    public void SetUniformBufferView(uint bindingSlot, BufferViewDesc view)
    {
        SetUniformBufferViews(bindingSlot, 0, new[] { view });
    }

    public void SetUniformBufferViews(uint bindingSlot, uint firstArrayIndex, BufferViewDesc[] views)
    {
        UpdateBufferViews(bindingSlot, firstArrayIndex, DescriptorType.UniformBufferView, views);
    }

    public void SetReadBufferView(uint bindingSlot, BufferViewDesc view)
    {
        SetReadBufferViews(bindingSlot, 0, new[] { view });
    }

    public void SetReadBufferViews(uint bindingSlot, uint firstArrayIndex, BufferViewDesc[] views)
    {
        UpdateBufferViews(bindingSlot, firstArrayIndex, DescriptorType.ReadBufferView, views);
    }

    public void SetReadWriteBufferView(uint bindingSlot, BufferViewDesc view)
    {
        SetReadWriteBufferViews(bindingSlot, 0, new[] { view });
    }

    public void SetReadWriteBufferViews(uint bindingSlot, uint firstArrayIndex, BufferViewDesc[] views)
    {
        UpdateBufferViews(bindingSlot, firstArrayIndex, DescriptorType.ReadWriteBufferView, views);
    }

    public void SetReadTextureView(uint bindingSlot, TextureViewDesc view)
    {
        SetReadTextureViews(bindingSlot, 0, new[] { view });
    }

    public void SetReadTextureViews(uint bindingSlot, uint firstArrayIndex, TextureViewDesc[] views)
    {
        UpdateTextureViews(bindingSlot, firstArrayIndex, DescriptorType.ReadTextureView, views);
    }

    public void SetReadWriteTextureView(uint bindingSlot, TextureViewDesc view)
    {
        SetReadWriteTextureViews(bindingSlot, 0, new[] { view });
    }

    public void SetReadWriteTextureViews(uint bindingSlot, uint firstArrayIndex, TextureViewDesc[] views)
    {
        UpdateTextureViews(bindingSlot, firstArrayIndex, DescriptorType.ReadWriteTextureView, views);
    }

    public void SetSampler(uint bindingSlot, SamplerDesc sampler)
    {
        SetSamplers(bindingSlot, 0, new[] { sampler });
    }

    public void SetSamplers(uint bindingSlot, uint firstArrayIndex, SamplerDesc[] samplers)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(samplers);
        var nativeSamplers = new NativeSamplerDesc[samplers.Length];
        for (var i = 0; i < nativeSamplers.Length; ++i)
        {
            nativeSamplers[i] = new NativeSamplerDesc(samplers[i]);
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DescriptorSetUpdateSampler(
            _idescriptorSet,
            bindingSlot,
            firstArrayIndex,
            nativeSamplers.Length == 0 ? null : nativeSamplers,
            (uint)nativeSamplers.Length)));
    }

    private void UpdateBufferViews(uint bindingSlot, uint firstArrayIndex, DescriptorType descriptorType, BufferViewDesc[] views)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(views);
        var nativeViews = new NativeBufferViewDesc[views.Length];
        for (var i = 0; i < nativeViews.Length; ++i)
        {
            nativeViews[i] = NativeBufferViewDesc.FromPublic(views[i]);
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DescriptorSetUpdateBufferView(
            _idescriptorSet,
            bindingSlot,
            firstArrayIndex,
            (uint)descriptorType,
            nativeViews.Length == 0 ? null : nativeViews,
            (uint)nativeViews.Length)));
    }

    private void UpdateTextureViews(uint bindingSlot, uint firstArrayIndex, DescriptorType descriptorType, TextureViewDesc[] views)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(views);
        var nativeViews = new NativeTextureViewDesc[views.Length];
        for (var i = 0; i < nativeViews.Length; ++i)
        {
            nativeViews[i] = NativeTextureViewDesc.FromPublic(views[i]);
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DescriptorSetUpdateTextureView(
            _idescriptorSet,
            bindingSlot,
            firstArrayIndex,
            (uint)descriptorType,
            nativeViews.Length == 0 ? null : nativeViews,
            (uint)nativeViews.Length)));
    }

    internal static IntPtr GetNativeDescriptorSetPointer(IDescriptorSet descriptorSet)
    {
        ArgumentNullException.ThrowIfNull(descriptorSet);
        if (descriptorSet is not RhiDescriptorSet nativeDescriptorSet)
        {
            throw new ArgumentException("The descriptor set must be created by Luna.RHI.", nameof(descriptorSet));
        }
        nativeDescriptorSet.EnsureNotDisposed();
        return nativeDescriptorSet._idescriptorSet;
    }
}
