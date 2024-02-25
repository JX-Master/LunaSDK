/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DescriptorSet.hpp
* @author JXMaster
* @date 2019/7/17
*/
#pragma once
#include "PipelineState.hpp"
#include "Buffer.hpp"
#include <Luna/Runtime/Math/Vector.hpp>
#ifndef LUNA_RHI_API
#define LUNA_RHI_API
#endif

namespace Luna
{
    namespace RHI
    {
        //! @addtogroup RHI
        //! @{
        
        //! Describes one buffer view in descriptor set.
        struct BufferViewDesc
        {
            //! If this is a uniform buffer, specify the offset, in bytes, of the data to be viewed.
            //! If this is a structured buffer, specify the index of the first element to be accessed by the view.
            u64 first_element;
            //! The buffer to set.
            IBuffer* buffer;
            //! The number of elements in this view.
            //! This must be set to 1 if this is a uniform buffer view.
            u32 element_count;
            //! If this is a uniform buffer, specify the size, in bytes, of the uniform buffer.
            //! If this is a structured buffer, specify the size, in bytes, of each element in the buffer structure.
            u32 element_size;
            static BufferViewDesc uniform_buffer(IBuffer* buffer, u64 offset = 0, u32 size = U32_MAX)
            {
                BufferViewDesc ret;
                ret.buffer = buffer;
                ret.first_element = offset;
                ret.element_count = 1;
                ret.element_size = size;
                return ret;
            }
            static BufferViewDesc structured_buffer(IBuffer* buffer, u64 first_element, u32 element_count, u32 element_size)
            {
                BufferViewDesc ret;
                ret.buffer = buffer;
                ret.first_element = first_element;
                ret.element_count = element_count;
                ret.element_size = element_size;
                return ret;
            }
        };

        //! Describes one texture view in descriptor set.
        struct TextureViewDesc
        {
            //! The texture to set.
            ITexture* texture;
            //! The texture view type.
            //! If this is @ref TextureViewType::unspecified, the system uses the texture's native type as texture 
            //! view type.
            TextureViewType type;
            //! The texture view format.
            //! If this is @ref Format::unknown, the system uses the texture's native format as texture 
            //! view format.
            Format format;
            //! The index of the first mip included in this view.
            u32 mip_slice;
            //! The number of mips included in this view.
            //! Mips in range [`mip_slice`, `mip_slice + mip_size`) will be included in this view.
            //! 
            //! If this is @ref U32_MAX, all mips beginning from `mip_slice` will be included in this view.
            u32 mip_size;
            //! The index of the first texture array element included in this view.
            u32 array_slice;
            //! The number of array elements included in this view.
            //! Array elements in range [`array_slice`, `array_slice + array_size`) will be included in this view.
            //! 
            //! If this is @ref U32_MAX, all array elements beginning from `array_slice` will be included in this view.
            //! 
            //! This must be `1` if `type` is not an texture array or texture cube type.
            //! This must be times of 6 if `type` is a texture cube type.
            u32 array_size;
            TextureViewDesc() = default;
            TextureViewDesc(ITexture* texture, TextureViewType type = TextureViewType::unspecified, Format format = Format::unknown,
                u32 mip_slice = 0, u32 mip_size = U32_MAX, u32 array_slice = 0, u32 array_size = U32_MAX) :
                texture(texture),
                type(type),
                format(format),
                mip_slice(mip_slice),
                mip_size(mip_size),
                array_slice(array_slice),
                array_size(array_size) {}
            static TextureViewDesc tex1d(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX)
            {
                TextureViewDesc desc;
                desc.texture = texture;
                desc.type = TextureViewType::tex1d;
                desc.format = format;
                desc.mip_slice = mip_slice;
                desc.mip_size = mip_size;
                desc.array_slice = 0;
                desc.array_size = 1;
                return desc;
            }
            static TextureViewDesc tex1darray(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX, u32 array_slice = 0, u32 array_size = U32_MAX)
            {
                TextureViewDesc desc;
                desc.texture = texture;
                desc.type = TextureViewType::tex1darray;
                desc.format = format;
                desc.mip_slice = mip_slice;
                desc.mip_size = mip_size;
                desc.array_slice = array_slice;
                desc.array_size = array_size;
                return desc;
            }
            static TextureViewDesc tex2d(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX)
            {
                TextureViewDesc desc;
                desc.texture = texture;
                desc.format = format;
                desc.type = TextureViewType::tex2d;
                desc.mip_slice = mip_slice;
                desc.mip_size = mip_size;
                desc.array_slice = 0;
                desc.array_size = 1;
                return desc;
            }
            static TextureViewDesc tex2darray(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX, u32 array_slice = 0, u32 array_size = U32_MAX)
            {
                TextureViewDesc desc;
                desc.texture = texture;
                desc.format = format;
                desc.type = TextureViewType::tex2darray;
                desc.mip_slice = mip_slice;
                desc.mip_size = mip_size;
                desc.array_slice = array_slice;
                desc.array_size = array_size;
                return desc;
            }
            static TextureViewDesc tex2dms(ITexture* texture, Format format = Format::unknown)
            {
                TextureViewDesc desc;
                desc.texture = texture;
                desc.format = format;
                desc.type = TextureViewType::tex2dms;
                desc.mip_slice = 0;
                desc.mip_size = 1;
                desc.array_slice = 0;
                desc.array_size = 1;
                return desc;
            }
            static TextureViewDesc tex2dmsarray(ITexture* texture, Format format = Format::unknown, u32 array_slice = 0, u32 array_size = U32_MAX)
            {
                TextureViewDesc desc;
                desc.texture = texture;
                desc.format = format;
                desc.type = TextureViewType::tex2dmsarray;
                desc.mip_slice = 0;
                desc.mip_size = 1;
                desc.array_slice = array_slice;
                desc.array_size = array_size;
                return desc;
            }
            static TextureViewDesc tex3d(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX)
            {
                TextureViewDesc desc;
                desc.texture = texture;
                desc.format = format;
                desc.type = TextureViewType::tex3d;
                desc.mip_slice = mip_slice;
                desc.mip_size = mip_size;
                desc.array_slice = 0;
                desc.array_size = 1;
                return desc;
            }
            static TextureViewDesc texcube(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX)
            {
                TextureViewDesc desc;
                desc.texture = texture;
                desc.format = format;
                desc.type = TextureViewType::texcube;
                desc.mip_slice = mip_slice;
                desc.mip_size = mip_size;
                desc.array_slice = 0;
                desc.array_size = 6;
                return desc;
            }
            static TextureViewDesc texcubearray(ITexture* texture, Format format = Format::unknown, u32 mip_slice = 0, u32 mip_size = U32_MAX, u32 array_slice = 0, u32 array_size = U32_MAX)
            {
                TextureViewDesc desc;
                desc.texture = texture;
                desc.format = format;
                desc.type = TextureViewType::texcubearray;
                desc.mip_slice = mip_slice;
                desc.mip_size = mip_size;
                desc.array_slice = array_slice;
                desc.array_size = array_size;
                return desc;
            }
        };

        //! Specifies the filtering mode for samplers.
        enum class Filter : u8
        {
            //! The data is taken from the nearest pixel of the sample point.
            nearest,
            //! The data is computed by linear interpolating between pixels around the sample point,
            //! using distances to such pixels as weights.
            linear,
        };
        
        //! Specifies the texture address mode for samplers when the sample position does not fall in range [0, 1].
        enum class TextureAddressMode : u8
        {
            //! Given the input position `x`, the sample position is computed as `x - floor(x)`.
            repeat,
            //! Given the input position `x`, the sample position is computed as `1 - abs(mod(x, 2) - 1)`
            mirror,
            //! Given the input position `x`, the sample position is computed as `clamp(x, 0, 1)`.
            clamp,
            //! Outputs one constant color if input position x is not in range [0, 1].
            border
        };

        //! Specifies the border color to use if texture address mode is @ref TextureAddressMode::border
        enum class BorderColor : u8
        {
            //! Outputs (0, 0, 0, 0) in floating-point format.
            float_0000,
            //! Outputs (0, 0, 0, 0) in integer format.
            int_0000,
            //! Outputs (0, 0, 0, 1) in floating-point format.
            float_0001,
            //! Outputs (0, 0, 0, 1) in integer format.
            int_0001,
            //! Outputs (1, 1, 1, 1) in floating-point format.
            float_1111,
            //! Outputs (1, 1, 1, 1) in integer format.
            int_1111,
        };

        //! Describes one sampler in descriptor set.
        struct SamplerDesc
        {
            //! The filter applied when the sample pixel size is greater than texel.
            Filter min_filter;
            //! The filter applied when the sample pixel size is smaller than texel.
            Filter mag_filter;
            //! The filter applied when sampling between different mipmaps.
            Filter mip_filter;
            //! The address mode used in the first direction (from left to right).
            TextureAddressMode address_u;
            //! The address mode used in the second direction (from top to down in D3D/Vulkan, from down to top in Metal).
            TextureAddressMode address_v;
            //! The address mode used in the third direction (from front to back in 3D textures).
            TextureAddressMode address_w;
            //! Whether to enable anisotropy sampling.
            bool anisotropy_enable;
            //! Whether to enable pixel comparing.
            bool compare_enable;
            //! The compare function used when `compare_enable` is `true`.
            CompareFunction compare_function;
            //! The border color used when `TextureAddressMode::border` is set.
            BorderColor border_color;
            //! The maximum anisotropy ratio.
            u32 max_anisotropy;
            //! The minimum LOD level that will be sampled.
            f32 min_lod;
            //! The maximum LOD level that will be sampled. 
            //! Specify one large enough value (like F32_MAX) to disable this limit.
            f32 max_lod;

            SamplerDesc() = default;
            SamplerDesc(Filter min_filter,
                Filter mag_filter,
                Filter mip_filter,
                TextureAddressMode address_u,
                TextureAddressMode address_v,
                TextureAddressMode address_w,
                bool anisotropy_enable = false,
                u32 max_anisotropy = 1,
                BorderColor border_color = BorderColor::float_0000,
                f32 min_lod = 0.0f,
                f32 max_lod = F32_MAX,
                bool compare_enable = false,
                CompareFunction compare_function = CompareFunction::always
                ) :
                min_filter(min_filter),
                mag_filter(mag_filter),
                mip_filter(mip_filter),
                address_u(address_u),
                address_v(address_v),
                address_w(address_w),
                anisotropy_enable(anisotropy_enable),
                compare_enable(compare_enable),
                compare_function(compare_function),
                border_color(border_color),
                max_anisotropy(max_anisotropy),
                min_lod(min_lod),
                max_lod(max_lod) {}
            bool operator==(const SamplerDesc& rhs)
            {
                return !memcmp(this, &rhs, sizeof(SamplerDesc));
            }
            bool operator!= (const SamplerDesc& rhs)
            {
                return !(*this == rhs);
            }
        };

        //! Describes one descriptor set.
        struct DescriptorSetDesc
        {
            //! The descriptor layout for this descriptor set.
            IDescriptorSetLayout* layout;
            //! If the descriptor layout has `DescriptorSetLayoutFlag::variable_descriptors` 
            //! set, this is the number of variable descriptors being allocated for this else.
            //! Otherwise, this should always be 0.
            u32 num_variable_descriptors = 0;

            DescriptorSetDesc() {}
            DescriptorSetDesc(IDescriptorSetLayout* layout, u32 num_variable_descriptors = 0) :
                layout(layout),
                num_variable_descriptors(num_variable_descriptors) {}
        };

        //! Describes one descriptor set write operation.
        //! Every descriptor set write operation may update one continuous range of descriptors 
        //! in the same binding slot.
        struct WriteDescriptorSet
        {
            //! The binding slot of the descriptors to update.
            u32 binding_slot;
            //! The slot offset of the first descriptor to update relative to the binding slot.
            u32 first_array_index;
            //! The number of descriptors to write.
            u32 num_descs;
            //! The type of the descriptors to write. This must be equal to the actual type of the 
            //! descriptors in the descriptor array.
            DescriptorType type;
            union
            {
                //! The pointer to buffer view descriptors data array to be written.
                //! This array must have at least `num_descs` elements, and will be used 
                //! if `type` is @ref DescriptorType::uniform_buffer_view, @ref DescriptorType::read_buffer_view
                //! or @ref DescriptorType::read_write_buffer_view.
                const BufferViewDesc* buffer_views;
                //! The pointer to texture view descriptors data array to be written.
                //! This array must have at least `num_descs` elements, and will be used 
                //! if `type` is @ref DescriptorType::read_texture_view and
                //!  @ref DescriptorType::read_write_texture_view.
                const TextureViewDesc* texture_views;
                //! The pointer to sampler descriptors data array to be written.
                //! This array must have at least `num_descs` elements, and will be used 
                //! if `type` is @ref DescriptorType::sampler.
                const SamplerDesc* samplers;
            };
            
            static WriteDescriptorSet uniform_buffer_view(u32 binding_slot, const BufferViewDesc& desc)
            {
                WriteDescriptorSet ret;
                ret.binding_slot = binding_slot;
                ret.first_array_index = 0;
                ret.type = DescriptorType::uniform_buffer_view;
                ret.num_descs = 1;
                ret.buffer_views = &desc;
                return ret;
            }
            static WriteDescriptorSet uniform_buffer_view_array(u32 binding_slot, u32 first_array_index, Span<const BufferViewDesc> descs)
            {
                WriteDescriptorSet ret;
                ret.binding_slot = binding_slot;
                ret.first_array_index = first_array_index;
                ret.type = DescriptorType::uniform_buffer_view;
                ret.num_descs = (u32)descs.size();
                ret.buffer_views = descs.data();
                return ret;
            }
            static WriteDescriptorSet read_buffer_view(u32 binding_slot, const BufferViewDesc& desc)
            {
                WriteDescriptorSet ret;
                ret.binding_slot = binding_slot;
                ret.first_array_index = 0;
                ret.type = DescriptorType::read_buffer_view;
                ret.num_descs = 1;
                ret.buffer_views = &desc;
                return ret;
            }
            static WriteDescriptorSet read_buffer_view_array(u32 binding_slot, u32 first_array_index, Span<const BufferViewDesc> descs)
            {
                WriteDescriptorSet ret;
                ret.binding_slot = binding_slot;
                ret.first_array_index = first_array_index;
                ret.type = DescriptorType::read_buffer_view;
                ret.num_descs = (u32)descs.size();
                ret.buffer_views = descs.data();
                return ret;
            }
            static WriteDescriptorSet read_write_buffer_view(u32 binding_slot, const BufferViewDesc& desc)
            {
                WriteDescriptorSet ret;
                ret.binding_slot = binding_slot;
                ret.first_array_index = 0;
                ret.type = DescriptorType::read_write_buffer_view;
                ret.num_descs = 1;
                ret.buffer_views = &desc;
                return ret;
            }
            static WriteDescriptorSet read_write_buffer_view_array(u32 binding_slot, u32 first_array_index, Span<const BufferViewDesc> descs)
            {
                WriteDescriptorSet ret;
                ret.binding_slot = binding_slot;
                ret.first_array_index = first_array_index;
                ret.type = DescriptorType::read_write_buffer_view;
                ret.num_descs = (u32)descs.size();
                ret.buffer_views = descs.data();
                return ret;
            }
            static WriteDescriptorSet read_texture_view(u32 binding_slot, const TextureViewDesc& desc)
            {
                WriteDescriptorSet ret;
                ret.binding_slot = binding_slot;
                ret.first_array_index = 0;
                ret.type = DescriptorType::read_texture_view;
                ret.num_descs = 1;
                ret.texture_views = &desc;
                return ret;
            }
            static WriteDescriptorSet read_texture_view_array(u32 binding_slot, u32 first_array_index, Span<const TextureViewDesc> descs)
            {
                WriteDescriptorSet ret;
                ret.binding_slot = binding_slot;
                ret.first_array_index = first_array_index;
                ret.type = DescriptorType::read_texture_view;
                ret.num_descs = (u32)descs.size();
                ret.texture_views = descs.data();
                return ret;
            }
            static WriteDescriptorSet read_write_texture_view(u32 binding_slot, const TextureViewDesc& desc)
            {
                WriteDescriptorSet ret;
                ret.binding_slot = binding_slot;
                ret.first_array_index = 0;
                ret.type = DescriptorType::read_write_texture_view;
                ret.num_descs = 1;
                ret.texture_views = &desc;
                return ret;
            }
            static WriteDescriptorSet read_write_texture_view_array(u32 binding_slot, u32 first_array_index, Span<const TextureViewDesc> descs)
            {
                WriteDescriptorSet ret;
                ret.binding_slot = binding_slot;
                ret.first_array_index = first_array_index;
                ret.type = DescriptorType::read_write_texture_view;
                ret.num_descs = (u32)descs.size();
                ret.texture_views = descs.data();
                return ret;
            }
            static WriteDescriptorSet sampler(u32 binding_slot, const SamplerDesc& desc)
            {
                WriteDescriptorSet ret;
                ret.binding_slot = binding_slot;
                ret.first_array_index = 0;
                ret.type = DescriptorType::sampler;
                ret.num_descs = 1;
                ret.samplers = &desc;
                return ret;
            }
            static WriteDescriptorSet sampler_array(u32 binding_slot, u32 first_array_index, Span<const SamplerDesc> descs)
            {
                WriteDescriptorSet ret;
                ret.binding_slot = binding_slot;
                ret.first_array_index = first_array_index;
                ret.type = DescriptorType::sampler;
                ret.num_descs = (u32)descs.size();
                ret.samplers = descs.data();
                return ret;
            }
        };

        //! @interface IDescriptorSet
        //! Describes which views and samples are bound to the pipeline.
        //! This can be set at any time before the draw call or dispatch call
        //! is submitted.
        struct IDescriptorSet : virtual IDeviceChild
        {
            luiid("{f12bc4b0-2aad-42bb-8b8c-237ed0593aa3}");

            //! Updates descriptors' data in this descriptor set.
            //! @param[in] writes An array of descriptor set write operations that will be 
            //! performed in this call.
            virtual RV update_descriptors(Span<const WriteDescriptorSet> writes) = 0;
        };

        //! @}
    }
}