Shader resources are resources that can be read/written by shader codes. Shader resources have the following types:

1. Uniform buffer, which is a `IBuffer` that stores constants that can be read by shaders.
1. Read buffer, which is a `IBuffer` that stores array of structures that can be read by shaders.
1. Read write buffer, which is a `IBuffer` that stores array of structures that can be read and written by shaders.
1. Read texture, which is a `ITexture` that can be sampled or read by shaders.
1. Read write texture, which is a `ITexture` that can be sampled, read and written by shaders.

## Descriptor sets and descriptor set layouts
Shader resources are not bound to the pipeline directly, instead, they should be bound to a **descriptor set** (`IDescriptorSet`) firstly, then all resources referred by one descriptor set is bound to the pipeline in one call by `ICommandBuffer::set_graphics_descriptor_set(index, descriptor_set)` or `ICommandBuffer::set_compute_descriptor_set(index, descriptor_set)`. To create a descriptor set, the user should firstly create one descriptor set layout object (`IDescriptorSetLayout`). The descriptor set layout object describes the type and format of each descriptors in the descriptor set. In order to create one descriptor set layout object, the user should fill `DescriptorSetLayoutDesc` descriptor first, then call `IDevice::new_descriptor_set_layout(desc)`, passing the descriptor object. 

One `DescriptorSetLayoutDesc` describes several bindings, every binding describes one or an array of resources of the same type that can be accessed by shaders as one parameter entry. One binding is represented by `DescriptorSetLayoutBinding`, and contains the following properties:

1. `binding_slot`: The binding number of this binding. Every binding should have one unique binding number. Binding numbers for bindings in the same descriptor set do not need to be continuous, but must be incremental.
1. `num_descs`: The number of descriptors of this binding. The binding number range `[binding_slot, binding_slot + num_descs)` is occupied by this binding and cannot overlap with the binding number ranges of other bindings.
1. `type`: The type of descriptors of this binding. All descriptors in the same binding must have the same type.
1. `texture_view_type`: The texture view type of descriptors of this binding if `type` is `read_texture_view` or `read_write_texture_view`. All descriptors in the same binding must have the same texture view type.
1. `shader_visibility_flags`: Specify shaders that have access to resources of this binding.

After one descriptor set layout object is created, the user can use this object to create descriptor set objects by calling `IDevice::new_descriptor_set(desc)`. When creating descriptor set objects, the user should fill one `DescriptorSetDesc` descriptor object, which is basically only stores one pointer to the descriptor set layout object being used.

### Unbound descriptor sets
In normal cases, the number of descriptors for each binding must be determined when creating descriptor set layout objects. However, if `DeviceFeature::unbound_descriptor_array` is supported, the number of descriptors of the **last binding** can be uncertain when creating the descriptor set layout. This allows the user to perform some advanced resource binding techniques, for example, allocate one big descriptor set to bind all resources that might be used in rendering, and select resources in shader code so that meshes using different materials can be rendered without switching descriptor sets, such feature is usually called bindless rendering and is crucial for implemeting GPU-driven rendering.

To create one variable descriptor set layout, specify `DescriptorSetLayoutFlag::variable_descriptors` when creating the descriptor set layout. In such case, `num_descs` of the last binding specifies the maximum number of descriptors that can be allocated. When creating descriptor sets from one variable descriptor set layout, the user should set `DescriptorSetDesc::num_variable_descriptors` to a non-zero value, which is the real number of descriptors allocated for the last binding. When using unbound descriptor sets, the shader code does not know the real number of descriptors of the last binding, the user should ensure that the shader code does not index one descriptor that is out of real descriptor array range.

## Updating descriptor sets
After creating descriptor sets, the user can write data to descriptors in descriptor sets. Updating descriptors in descriptor sets are done by `IDescriptorSet::update_descriptors(writes)`. The user may update multiple descriptors in multiple bindings in one update call, which is described by an array of `WriteDescriptorSet` structures passed to the update call. Every `WriteDescriptorSet` structure describes one continuous range of descriptors in the same `DescriptorSetLayoutBinding`. Based on the type of descriptors to be updated, the user should attach the descriptor array pointer to `buffer_views`, `texture_views` or `samplers`, and set the array size to `num_descs`.

### Buffer view descriptor
For descriptors with `DescriptorType::uniform_buffer_view`, `DescriptorType::read_buffer_view` and `DescriptorType::read_write_buffer_view` types, buffer view descriptors (`BufferViewDesc`) are used to update descriptors' data. One buffer view descriptor can restrain shader to access only one portion of the buffer, so that multiple data sections can be packed into one buffer and viewed by different view descriptors.

#### Uniform buffer
For uniform buffers, the user should use `BufferViewDesc::first_element` to specify the byte offset from the beginning of the buffer data to the first byte to be used, and `BufferViewDesc::element_size` to specify the size of the uniform buffer data to be used. The offset and size of one uniform data must satisfy hardware alignment requirements, which can be fetched by `IDevice::get_uniform_buffer_data_alignment()`. The user may use `BufferViewDesc::uniform_buffer(buffer, offset, size)` static constructor to create one `BufferViewDesc` for one uniform buffer descriptor quickly.

#### Structured buffer
For read buffers and read-write buffers, the user should use `BufferViewDesc::element_size` to specify the size of each element in the buffer, `BufferViewDesc::first_element` to specify the index of the first element to be used, and `BufferViewDesc::element_count` to specify the number of elements to be used. The user may use `BufferViewDesc::structured_buffer(buffer, first_element, element_count, element_size)` static constructor to create one `BufferViewDesc` for one read buffer/read-write buffer descriptor quickly.

### Texture view descriptor
For descriptors with `DescriptorType::read_texture_view` and `DescriptorType::read_write_texture_view` types, texture view descriptors (`TextureViewDesc`) are used to update descriptors' data. One texture view descriptor can restrain shader to access only a certain range of subresources of the texture, so that different subresources of one texture may be bound to the same or different pipeline using different texture views.

#### Texture view type
`TextureViewDesc::type` specifies the texture view type, which has the following options:

| Option         | Texture type | Array | Multisample | Cube |
|----------------|--------------|-------|-------------|------|
| `tex1d`        | 1D texture   | No    | No          | No   |
| `tex2d`        | 2D texture   | No    | No          | No   |
| `tex2dms`      | 2D texture   | No    | Yes         | No   |
| `tex3d`        | 3D texture   | No    | No          | No   |
| `texcube`      | 2D texture   | No    | No          | Yes  |
| `tex1darray`   | 1D texture   | Yes   | No          | No   |
| `tex2darray`   | 2D texture   | Yes   | No          | No   |
| `tex2dmsarray` | 2D texture   | Yes   | Yes         | No   |
| `texcubearray` | 2D texture   | Yes   | No          | Yes  |

The texture view type of one texture view must be compatible with the viewing texture. For example, you cannot bind one 2D texture with `TextureViewType::tex1d` texture view. If you don't know the exact texture view type to use, you can specify `TextureViewType::unspecified` to let the system to choose one shitable texture view type based on the binding texture and view settings.

#### Texture view format
`TextureViewDesc::format` tells the pipeline how to interpret data for one texture. When supported by the backend, one texture view may specify one format that is different than the native format of the binding texture, which will let the driver to reinterpret texture data when accessing textures. If format reinterpreting is not supported by the driver, the user must specify the same format for both texture and texture view. The texture view format can be `Format::unknown`, which tells the system to use the binding texture's native format as the texture view format.

#### Mip and array slice
`TextureViewDesc::mip_slice` and `TextureViewDesc::mip_size` specifies the mip range $[mip\_slice, mip\_slice + mip\_size)$ that will be bind to the pipeline. After specified, `TextureViewDesc::mip_slice` becomes the mip level 0 in shader code. `TextureViewDesc::mip_size` must be `1` for `tex2dms` and `tex2dmsarray` views, and views used for read-write texture descriptors. `TextureViewDesc::mip_size` may be `U32_MAX`, which tells the system to use all available mips since `TextureViewDesc::mip_slice`.

`TextureViewDesc::array_slice` and `TextureViewDesc::array_size` specifies the array range $[array\_slice, array\_slice + array\_size)$ that will be bind to the pipeline. After specified, `TextureViewDesc::array_slice` becomes the first array element in shader code. `TextureViewDesc::array_slice` must be `0` for non-array texture views, `TextureViewDesc::array_size` must be:

* `1` for `tex1d`, `tex2d`, `tex2dms` and `tex3d` views.
* `6` for `texcube` views.
* times of `6` for `texcubearray` views.

`TextureViewDesc::array_size` may be `U32_MAX`, which tells the system to use all available array elements since `TextureViewDesc::array_slice`.

#### Static constructors
The user may use the following static constructors to declare texture views of different types quickly:

* `TextureViewDesc::tex1d(texture, format, mip_slice, mip_size)`
* `TextureViewDesc::tex1darray(texture, format, mip_slice, mip_size, array_slice, array_size)`
* `TextureViewDesc::tex2d(texture, format, mip_slice, mip_size)`
* `TextureViewDesc::tex2darray(texture, format, mip_slice, mip_size, array_slice, array_size)`
* `TextureViewDesc::tex2dms(texture, format)`
* `TextureViewDesc::tex2dmsarray(texture, format, array_slice, array_size)`
* `TextureViewDesc::tex3d(texture, format, mip_slice, mip_size)`
* `TextureViewDesc::texcube(texture, format, mip_slice, mip_size)`
* `TextureViewDesc::texcubearray(texture, format, mip_slice, mip_size, array_slice, array_size)`

### Sampler descriptor
For descriptors with `DescriptorType::sampler` type, sampler descriptors (`SamplerDesc`) are used to update descriptors' data. One sampler descriptor describes sampling configurations that shaders can use to sample textures.

## Pipeline layouts
Pipeline layouts (`IPipelineLayout`) describes the descriptor sets binding layout for one graphics or compute pipeline. Before descriptor sets are attached to one pipeline, its pipeline layout must be set firstly by `ICommandBuffer::set_graphics_pipeline_layout(pipeline_layout)` or `ICommandBuffer::set_compute_pipeline_layout(pipeline_layout)`.

To create one pipeline layout, the user should fill one pipeline layout descriptor object (`PipelineLayoutDesc`), and pass the object to `IDevice::new_pipeline_layout(desc)`. When configuring `PipelineLayoutDesc`, the user should specify the number of descriptor sets that will be attached to the pipeline, and the descriptor set layout of every descriptor set. The user can also use `PipelineLayoutFlag` to control shaders and stages that can access bound resources, dening shaders and stages access to resources may improve performance on some hardwares.

## Binding descriptor sets
Once descriptors in descriptor sets are updated, the user can bind descriptor sets to pipelines so that all resources and configurations stored in descriptor sets are used in succeeding draw or dispatch calls. Binding descriptor sets to pipelines are achieved via `ICommandBuffer::set_graphics_descriptor_set(index, descriptor_set)` or `ICommandBuffer::set_graphics_descriptor_sets(start_index, descriptor_sets)` for graphics pipelines, and `ICommandBuffer::set_compute_descriptor_set(index, descriptor_set)` or `ICommandBuffer::set_compute_descriptor_sets(start_index, descriptor_sets)` for compute pipelines. Compatible pipeline layout must be set to the pipelines before binding descriptor sets. After one descriptor set is bound to one pipeline, the user cannot update descriptors in the descriptor set or bind the same descriptor set to another pipeline until the command buffer is finished executing or reset. If one pipeline have multiple descriptor sets, every descriptor set can be bound independently using dedicated binding calls, and descriptor sets bound to the pipeline stays bound until changed by another binding call, or the render or compute pass is closed.