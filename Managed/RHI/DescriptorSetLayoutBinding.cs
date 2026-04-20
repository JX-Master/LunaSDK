namespace Luna.RHI;

public readonly struct DescriptorSetLayoutBinding
{
    public static DescriptorSetLayoutBinding UniformBufferView(uint bindingSlot, uint count, ShaderVisibilityFlags visibility)
    {
        return new DescriptorSetLayoutBinding(bindingSlot, count, DescriptorType.UniformBufferView, TextureViewType.Unspecified, visibility);
    }

    public static DescriptorSetLayoutBinding ReadBufferView(uint bindingSlot, uint count, ShaderVisibilityFlags visibility)
    {
        return new DescriptorSetLayoutBinding(bindingSlot, count, DescriptorType.ReadBufferView, TextureViewType.Unspecified, visibility);
    }

    public static DescriptorSetLayoutBinding ReadWriteBufferView(uint bindingSlot, uint count, ShaderVisibilityFlags visibility)
    {
        return new DescriptorSetLayoutBinding(bindingSlot, count, DescriptorType.ReadWriteBufferView, TextureViewType.Unspecified, visibility);
    }

    public static DescriptorSetLayoutBinding ReadTextureView(TextureViewType viewType, uint bindingSlot, uint count, ShaderVisibilityFlags visibility)
    {
        return new DescriptorSetLayoutBinding(bindingSlot, count, DescriptorType.ReadTextureView, viewType, visibility);
    }

    public static DescriptorSetLayoutBinding ReadWriteTextureView(TextureViewType viewType, uint bindingSlot, uint count, ShaderVisibilityFlags visibility)
    {
        return new DescriptorSetLayoutBinding(bindingSlot, count, DescriptorType.ReadWriteTextureView, viewType, visibility);
    }

    public static DescriptorSetLayoutBinding Sampler(uint bindingSlot, uint count, ShaderVisibilityFlags visibility)
    {
        return new DescriptorSetLayoutBinding(bindingSlot, count, DescriptorType.Sampler, TextureViewType.Unspecified, visibility);
    }

    public DescriptorSetLayoutBinding(uint bindingSlot, uint count, DescriptorType type, TextureViewType textureViewType, ShaderVisibilityFlags visibility)
    {
        BindingSlot = bindingSlot;
        Count = count;
        Type = type;
        TextureViewType = textureViewType;
        Visibility = visibility;
    }

    public uint BindingSlot { get; }

    public uint Count { get; }

    public DescriptorType Type { get; }

    public TextureViewType TextureViewType { get; }

    public ShaderVisibilityFlags Visibility { get; }
}
