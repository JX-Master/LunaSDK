Resources (`IResource`) are GPU-accessible memories that stores certain data that can be used for reading, writing and sampling by GPU. Resources have two types: **buffers** and **textures**. Buffer resources can contain arbitrary binary data, and is usually used for storing parameters, geometry data, material data and so on. Texture resrouces can only contain image data of certain formats, and support hardware data sampling using samplers.

## Memory types
Memory type defines the memory properties of the resource, like which heap to allocate the memory for the resource, and the CPU access policy of the allocated memory. In RHI, we have three memory types: **local**, **upload** and **readback**.

The local memory type is allocated on memory that is visible only to GPU. Such memory gains maximum GPU bandwidth, but cannot be accessed by CPU. On platforms with non-uniform memory architecture (NUMA), the local memory will be allocated on video memory, which cannot be accessed by CPU; in a platform with uniform memory architecture (UMA), the local memory will be allocated on system memory. While it is technically possible for CPU to access local memory on UMA, preventing such access gives the hardware and driver more rooms for optimizing GPU access efficiency.

The upload memory type is allocated on system memory that is optimized for CPU writing. GPU cannot write to this memory and GPU reading from upload memory is slow. On NUMA platfroms, reading data from upload memory from GPU requires data transmission through PCI-Express bus, which is much slower than reading data in local memory from GPU. We recommend using upload memory only for uploading data to local memory or reading the data only once per CPU write.

The readback memory type is allocated on system memory that is optimized for CPU reading. GPU writing to read back memory  is slow, and the only operation allowed for GPU is to copy data to the memory. On NUMA platfroms, writing data to readback memory from GPU requires data transmission through PCI-Express bus, which is a slow operation.

The user should choose the suitable memory type based on the use situation. Here are some basic principles:

1. If you need to create texture resources, use local memory. If you need to upload texture data from CPU side, use upload memory to copy data to the local memory.
1. If you don't need to access resource data from CPU, use local memory.
1. If you only need to upload data from CPU side once, like setting the initial data for static vertex and index buffers, use one local memory to store the data, then use one temporary upload memory to copy data to the local memory.
1. If you need to upload data from CPU side multiple times, but the data is read by GPU only once per CPU update, use upload memory.
1. If you need to upload data from CPU side multiple times, and the data will be read by GPU multiple times per CPU update, use one local memory resource for GPU access and one upload memory resource for CPU access, and copy data between two resources when needed.
1. If you need to read resource data from CPU side, use readback memory.

## Buffers
Buffers are memory blocks that can store arbitrary binary data. Typically, you use buffers to:

1. Set uniform parameters that can be read by shaders using **uniform buffers**.
1. Store mesh geometries for rendering using **vertex buffers** and **index buffers**.
1. Pass array of structures (like model transform matrices, material parameters, etc.) using **structured buffers**.
1. Copy data between host memory and device-local memory using **upload buffers** and **readback buffers**.
1. Record GPU-generated draw and compute commands using **indirect buffers**.

Buffers are described by `BufferDesc`, and are created by `IDevice::new_buffer(memory_type, desc)`. When creating buffers, the user must specify the memory type of the buffer, and one `BufferDesc` descriptor for the buffer. Properties for one buffer includes the size of the buffer and possible usages of the buffer. If one buffer is created using **upload** or **readback** memory type, the application can fetch one pointer to the buffer memory by calling `IBuffer::map`, and must release the pointer when the application no longer needs access to the buffer memory by calling `IBuffer::unmap`.

### Buffer usages

Buffer usages specify the possible usages of one buffer. One buffer can have multiple usages, which can be specified using a bitwise OR combination of multiple `BufferUsageFlag` flags. The buffer usages include:

1. `copy_source`: Allows this buffer to be bound as copy source.
1. `copy_dest`: Allows this buffer to be bound as copy destination.
1. `uniform_buffer`: Allows this buffer to be bound to a uniform buffer view.
1. `read_buffer`: Allows this buffer to be bound to a read buffer view.
1. `read_write_buffer`: Allows this buffer to be bound to a read-write buffer view.
1. `vertex_buffer`: Allows this buffer to be bound as a vertex buffer.
1. `index_buffer`: Allows this buffer to be bound as a index buffer.
1. `indirect_buffer`: Allows this buffer to be bound as a buffer providing indirect draw arguments.

All possible usages for one buffer must be specified when the buffer is created, one buffer cannot be 

### Usage patterns
Buffers themselves are scheme-less (or typeless), they can store arbitrary binary data, and it is up to the user how to interpret buffer data. Here we list some typeical usage patterns for buffers.

#### Uniform buffer
Uniform buffers are used to store uniform parameters that will be passed to all shader threads, they are set by the application and is read-only in shader code.

To create a uniform buffer, call `IDevice::new_buffer(memory_type, desc)` with `MemoryType::upload` and `BufferUsageFlag::uniform_buffer`. The device has memory alignment requires for uniform buffers, which can be fetched by `IDevice::get_uniform_buffer_data_alignment()`. The buffer size for one uniform buffer must satisfy the alignment requires:

```c++
#include <Luna/RHI/Device.hpp>
#include <Luna/RHI/Buffer.hpp>

usize alignment = device->get_uniform_buffer_data_alignment();
BufferDesc desc;
desc.size = align_upper(sizeof(MyUniformBuffer), alignment);
desc.usages = BufferUsageFlag::uniform_buffer;
desc.flags = ResourceFlag::none;
luexp(buffer, device->new_buffer(MemoryType::upload, desc));
```

You can also pack multiple uniform buffers into one big buffer. In such case, the offset and size of each uniform buffer must also satisfy alignment requirements for uniform buffers:

```c++
#include <Luna/RHI/Device.hpp>
#include <Luna/RHI/Buffer.hpp>

usize alignment = device->get_uniform_buffer_data_alignment();
usize size = 0;
for(auto& my_buffer : my_buffers)
{
    my_buffer.uniform_buffer_offset = size;
    my_buffer.uniform_buffer_size = align_upper(my_buffer.data_size, alignment);
    size += my_buffer.uniform_buffer_size;
}
BufferDesc desc;
desc.size = size;
desc.usages = BufferUsageFlag::uniform_buffer;
desc.flags = ResourceFlag::none;
lulet(buffer, device->new_buffer(MemoryType::upload, desc));
```

When binding multiple uniform buffers in one `IBuffer` to descriptor sets, use `BufferViewDesc::uniform_buffer(buffer, offset, size)` to create proper buffer views for uniform buffers:

```c++
#include <Luna/RHI/Device.hpp>
#include <Luna/RHI/Buffer.hpp>
#include <Luna/RHI/DescriptorSet.hpp>

Ref<IDescriptorSet> ds = get_my_descriptor_set();
Ref<IBuffer> ub = get_my_uniform_buffer();
Vector<BufferViewDesc> buffer_views;
for(auto& my_buffer : my_buffers)
{
    BufferViewDesc view = BufferViewDesc::uniform_buffer(ub, my_buffer.uniform_buffer_offset, my_buffer.uniform_buffer_size);
    buffer_views.push_back(view);
}
WriteDescriptorSet write = WriteDescriptorSet::uniform_buffer_view_array(MY_UNIFORM_BUFFER_BINDING_SLOT, 0, {buffer_views.data(), buffer_views.size()});
luexp(ds->update_descriptors({&write, 1}));
```

#### Vertex buffer and index buffer
Vertex buffers and index buffers store vertex data of one geometry. To create a vertex buffer, call `IDevice::new_buffer(memory_type, desc)` with `MemoryType::local` and `BufferUsageFlag::vertex_buffer | BufferUsageFlag::copy_dest`. To create a index buffer, call `IDevice::new_buffer(memory_type, desc)` with `MemoryType::local` and `BufferUsageFlag::index_buffer | BufferUsageFlag::copy_dest`. The data of vertex buffers and index buffers can be uploaded using `copy_resource_data(command_buffer, copies)`:

```c++
#include <Luna/RHI/Device.hpp>
#include <Luna/RHI/Buffer.hpp>
#include <Luna/RHI/Utility.hpp>

u64 vb_size = sizeof(MyVertex) * num_vertices;
u64 ib_size = sizeof(u32) * num_indices;
BufferDesc desc;
desc.size = vb_size;
desc.usages = BufferUsageFlag::vertex_buffer | BufferUsageFlag::copy_dest;
desc.flags = ResourceFlag::none;
lulet(vb, device->new_buffer(MemoryType::local, desc));
desc.size = ib_size;
desc.usages = BufferUsageFlag::index_buffer | BufferUsageFlag::copy_dest;
lulet(ib, device->new_buffer(MemoryType::local, desc));

CopyResourceData copies[2] = {
    CopyResourceData::write_buffer(vb, 0, my_vertex_data, vb_size),
    CopyResourceData::write_buffer(ib, 0, my_index_data, ib_size)
};
luexp(copy_resource_data(get_copy_command_buffer(), {copies, 2}));
```

Vertex buffers and index buffers are described by `VertexBufferView` and `IndexBufferView`, and are bound to the pipeline directly by calling `ICommandBuffer::set_vertex_buffers(start_slot, views)` and `ICommandBuffer::set_index_buffer(view)`:

```c++
#include <Luna/RHI/CommandBuffer.hpp>

ICommandBuffer* cmdbuf = get_render_command_buffer();
VertexBufferView vb_view = VertexBufferView(vb, 0, sizeof(MyVertex) * num_vertices, sizeof(MyVertex));
cmdbuf->set_vertex_buffers(0, {&vb_view, 1});
IndexBufferView ib_view = IndexBufferView(ib, 0, sizeof(u32) * num_indices, Format::r32_uint);
cmdbuf->set_index_buffer(ib_view);
```

#### Structured buffers
Structured buffers can be used to store one array of structures, enabling shader code to index (read and write) any element in the buffer. Such buffers can be useful to store large-sized array like the model-to-world matrices for all meshes, the material parameters for all materials of the same type, etc.

To create a structured buffer, call `IDevice::new_buffer(memory_type, desc)` with `MemoryType::local` or `MemoryType::upload`, depends on your update frequency, and `BufferUsageFlag::read_buffer` if you only need to read the buffer data from shader code, or `BufferUsageFlag::read_write_buffer` if you need to read and write buffer data from shader code.

```c++
#include <Luna/RHI/Device.hpp>
#include <Luna/RHI/Buffer.hpp>

BufferDesc desc;
desc.size = sizeof(MyBufferElement) * num_elements;
desc.usages = BufferUsageFlag::read_buffer;
desc.flags = ResourceFlag::none;
luexp(buffer, device->new_buffer(MemoryType::upload, desc));
```

To bind one structured buffer to the descriptor set, use `BufferViewDesc::structured_buffer(buffer, first_element, element_count, element_size)` to create a view for the buffer:

```c++
#include <Luna/RHI/Device.hpp>
#include <Luna/RHI/Buffer.hpp>
#include <Luna/RHI/DescriptorSet.hpp>

Ref<IDescriptorSet> ds = get_my_descriptor_set();
Ref<IBuffer> buffer = get_my_buffer();
BufferViewDesc buffer_view = BufferViewDesc::structured_buffer(buffer, 0, num_elements, sizeof(MyBufferElement));
WriteDescriptorSet write = WriteDescriptorSet::read_buffer_view(MY_STRUCTURED_BUFFER_BINDING_SLOT, buffer_view);
luexp(ds->update_descriptors({&write, 1}));
```

## Textures
Textures are memory blocks that store 1D, 2D or 3D image data, and support hardware data sampling using samplers. Typically, you use textures to:

1. Store images loaded from files to use them in rendering or computing.
1. Store the render result of one render pass.
1. Store the depth information of the scene, which will be used in depth and stencil testing.
1. Store the intermediate render result in a multi-pass render pipeline.

Textures are described by `TextureDesc`, and are created by `IDevice::new_texture(memory_type, desc, optimized_clear_value)`. When creating textures, the user must specify one `TextureDesc` descriptor for the texture. Currently, textures can only be created in local memory, so `memory_type` should always by `MemoryType::local` when creating textures. The user can also specify one optional optimized clear value for one texture, which *may* improve performance on some hardware when the texture is cleared using the same clear value as the specified.

### Texture usages
Texture usages specify the possible usages of one texture. One texture can have multiple usages, which can be specified using a bitwise OR combination of multiple `TextureUsageFlag` flags. The texture usages include:

1. `copy_source`: Allows this texture to be bound as copy source.
1. `copy_dest`: Allows this texture to be bound as copy destination.
1. `read_texture`: Allows this texture to be bound to a read texture view.
1. `read_write_texture`: Allows this texture to be bound to a read-write texture view.
1. `color_attachment`: Allows this texture to be bound as color attachment.
1. `depth_stencil_attachment`: Allows this texture to be bound as depth stencil attachment.
1. `resolve_attachment`: Allows this texture to be bound to a resolve attachment.
1. `cube`: Allows this texture to be bound to a texture cube view.

### Texture types and dimensions
Texture types identify the type of the texture, including:

1. `TextureType::tex1d`: 1-dimensional texture, which represents a vector of pixels.
1. `TextureType::tex2d`: 2-dimensional texture, which represents a 2D matrix of pixels.
1. `TextureType::tex3d`: 3-dimensional texture, which represents a 3D matrix of pixels.

Textures have three dimensions of sizes: **width**, **height** and **depth**:

1. For 1D textures, only width is available, height and depth must always be 1.
1. For 2D textures, both width and height are available, depth must always be 1.
1. For 3D textures, width, height and depth are available.

### Pixel format
One texture also have a particular pixel format, which is identified by `Format` enumeration. Most formats are formed by a combination of the following three parts:

1. Number of color channels. One pixel may have one to four color channels, identified as `r`, `g`, `b` and `a`.
1. The bit width of every color channel. One pixel may have 8 to 64 bits per channel.
1. The number format of every color channel. One pixel may have the following number formats:
    1. `uint`: unsigned integer.
    1. `sing`: signed integer.
    1. `unorm`: unsigned normalized integer that maps the unsigned integer to [0.0, 1.0]. For example, if every channel have 8 bits, then the value range [0, 255] is mapped to [0.0, 1.0] in shader automatically. 
    1. `snorm`: signed normalized integer that maps the signed integer to [-1.0, 1.0]. For example, if every channel have 8 bits, then the value range [-128, 127] is mapped to [-1.0, 1.0] in shader automatically. 
    1. `float`: floating-point number.

For exmaple:

1. `Format::rgba8_unorm` represents a 4-channels pixel format, where every channel stores one 8-bit unsigned integer that will be mapped to [0.0, 1.0] in shader.
1. `Format::rgba16_float` represents a 4-channels pixel format, where every channel stores one 16-bit floating-point number (half-precision).
1. `Format::rg32_float` represents a 2-channels pixel format, where every channel stores one 32-bit floating-point number (single-precision).

There are also some special formats:

1. Formats begin with `d` like `Format::d16_unorm`, `Format::d32_float`, etc. are special formats used for depth stencil textures.
1. Formats ends with `_srgb` are sRGB formats, the hardware will perform sRGB to linear color conversion when reading data from such formats.
1. Some formats may reorder color channels, like `Format::bgra8_unorm`. Such formats are usually used for special cases like back buffers, where hardware has special requirements for the format that can be used for presenting the render results. The color channels will always be reordered to `rgba` implicitly when used in shaders.
1. Some formats are compressed formats like BC, ASTC, etc. They uses pixel compression techniques to reduce texture file size and the memory consumption when being loaded. Compressed formats often has special requirements for texture dimensions sizes. For example, block compression (BC) format series only works on 2D textures, and requires the width and height of the texture being multiple of 4. Such formats will be hardware-uncompressed automatically when being sampled in shader.

### Mipmap
Mipmapping is a computer graphics technology that reduces aliasing artifacts when the texture is sampled in a lower resolution than the texture's original resolution. Such artifacts is usually called [Moiré pattern](https://en.wikipedia.org/wiki/Moir%C3%A9_pattern). When mipmapping is used, textures will be stored as as a sequence of sub-textures. The original texture is the first sub-texture in the sequence (called **mip level 0**), and every succeeding sub-texture in the sequence is a coarse representation of the previous sub-texture. In particular, for the **mip level N** sub-texture in the sequence, we have:

```
tex[N].width  = ceil(tex[N-1].width / 2.0)
tex[N].height = ceil(tex[N-1].height / 2.0)
tex[N].depth  = ceil(tex[N-1].depth / 2.0)
```

The user may control the number of sub-textures in the mipmap sequence by setting `mip_levels`. If this is `0`, the system will generate a *full mipmap chain* for the texture, which repeats the half-divide process until the last sub-texture in the mipmap sequence has width, height and depth all equal to `1`.

### Texture array
One 1D and 2D texture resource may contain multiple textures, which forms a texture array. The texture size, pixel format, usages and other properties apply to all textures in the texture array. If mipmaping is used, each texture in the texture array will have its independent mipmap chain. One texture array is bound to the pipeline as a single resource, and the user can access each texture of the texture array independently in shader code.

### Subresources
Subresources are sub-textures that belongs to one texture resource. One texture resource may have mipmap subresources and/or array subresources, the total number of subresources one texture resource have are `mip_levels * array_size`. The following figure shows one 8x8 texture resource with 4 mip levels and 3 array elements, which counts to 12 subresources in total.
![[subresources.svg]]
To index one subresource, the user should pass the mip index and array index of the subresource. Every subresource in one texture can be indexed and manipulated independently.

### Usage patterns

Here we list some typeical usage patterns for textures.

#### Color attachments and depth stencil attachments
Textures can be used as color attachments and depth stencil attachments for render passes. To create one texture used as color attachment, add `TextureUsageFlag::color_attachment` usage flag to texture usages. To create one texture used as depth stencil attachment, choose one depth format for the texture and add `TextureUsageFlag::depth_stencil_attachment` usage flag to the texture usages. Attachments usually have both `mip_levels` and `array_size` set to `1`.

To bind attachments to render passes, firstly set textures in `RenderPassDesc`, then call `ICommandBuffer::begin_render_pass(desc)` with the render pass descriptor. Attachments will be bound to the render pass until `ICommandBuffer::end_render_pass()` is called.

#### Static textures
Static textures store data loaded from image files, such texture is usually used for texturing models in the scene. To create one static texture, firstly add `TextureUsageFlag::read_texture` and `TextureUsageFlag::copy_dest` usage flags to texture usages. `mip_levels` is usually set to `0` to generate a full mipmap chain for such texture. After the texture is created, use `copy_resource_data(command_buffer, copies)` or upload buffers to upload texture data to the mip 0 of the texture.

After the texture is created, we need to generate mipmaps for the texture. This can be done by using a compute shader to downsample from a detailed mip to a coarse mip.