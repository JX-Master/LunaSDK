using System;
using Luna.Runtime;

namespace Luna.RHI;

internal sealed class RhiDescriptorSetLayout : RhiDeviceChild, IDescriptorSetLayout
{
    private readonly IntPtr _idescriptorSetLayout;

    internal RhiDescriptorSetLayout(IntPtr nativeObject, IntPtr nativeDescriptorSetLayout, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeDescriptorSetLayout == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeDescriptorSetLayout));
        }
        _idescriptorSetLayout = nativeDescriptorSetLayout;
    }

    internal static IntPtr GetNativeDescriptorSetLayoutPointer(IDescriptorSetLayout descriptorSetLayout)
    {
        ArgumentNullException.ThrowIfNull(descriptorSetLayout);
        if (descriptorSetLayout is not RhiDescriptorSetLayout nativeDescriptorSetLayout)
        {
            throw new ArgumentException("The descriptor set layout must be created by Luna.RHI.", nameof(descriptorSetLayout));
        }
        nativeDescriptorSetLayout.EnsureNotDisposed();
        return nativeDescriptorSetLayout._idescriptorSetLayout;
    }
}
