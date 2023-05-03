/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DescriptorSet.cpp
* @author JXMaster
* @date 2023/4/19
*/
#include "DescriptorSet.hpp"
#include "Resource.hpp"
namespace Luna
{
	namespace RHI
	{
		RV DescriptorSet::init(const DescriptorSetDesc& desc)
		{
			lutry
			{
				m_layout = (DescriptorSetLayout*)desc.layout->get_object();
				VkDescriptorSetAllocateInfo alloc_info{};
				alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				alloc_info.descriptorPool = m_device->m_desc_pool;
				alloc_info.descriptorSetCount = 1;
				alloc_info.pSetLayouts = &m_layout->m_layout;
				if (test_flags(m_layout->m_desc.flags, DescriptorSetLayoutFlag::variable_descriptors))
				{
					return BasicError::not_supported();
				}
				/*VkDescriptorSetVariableDescriptorCountAllocateInfo variable_info{};
				if (test_flags(m_layout->m_desc.flags, DescriptorSetLayoutFlag::variable_descriptors))
				{
					variable_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
					u32 counts = desc.num_variable_descriptors;
					variable_info.pDescriptorCounts = &counts;
					variable_info.descriptorSetCount = 1;
					alloc_info.pNext = &variable_info;
				}*/
				luexp(encode_vk_result(m_device->m_funcs.vkAllocateDescriptorSets(m_device->m_device, &alloc_info, &m_desc_set)));
			}
			lucatchret;
			return ok;
		}
		DescriptorSet::~DescriptorSet()
		{
			for (auto& i : m_image_views)
			{
				if (i.second != VK_NULL_HANDLE)
				{
					m_device->m_funcs.vkDestroyImageView(m_device->m_device, i.second, nullptr);
					i.second = VK_NULL_HANDLE;
				}
			}
			m_image_views.clear();
			for (auto& i : m_samplers)
			{
				if (i.second != VK_NULL_HANDLE)
				{
					m_device->m_funcs.vkDestroySampler(m_device->m_device, i.second, nullptr);
					i.second = VK_NULL_HANDLE;
				}
			}
			m_samplers.clear();
			if (m_desc_set != VK_NULL_HANDLE)
			{
				MutexGuard guard(m_device->m_mtx);
				m_device->m_funcs.vkFreeDescriptorSets(m_device->m_device, m_device->m_desc_pool, 1, &m_desc_set);
				m_desc_set = VK_NULL_HANDLE;
			}
		}
		void DescriptorSet::set_image_view(u32 slot, VkImageView image_view)
		{
			auto iter = m_image_views.insert(make_pair(slot, image_view));
			if (!iter.second)
			{
				m_device->m_funcs.vkDestroyImageView(m_device->m_device, iter.first->second, nullptr);
				iter.first->second = image_view;
			}
		}
		void DescriptorSet::set_sampler(u32 slot, VkSampler sampler)
		{
			auto iter = m_samplers.insert(make_pair(slot, sampler));
			if (!iter.second)
			{
				m_device->m_funcs.vkDestroySampler(m_device->m_device, iter.first->second, nullptr);
				iter.first->second = sampler;
			}
		}
		inline void encode_image_view_create_info(VkImageViewCreateInfo& dst, const TextureViewDesc& src)
		{
			dst.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			dst.flags = 0;
			ImageResource* image = cast_objct<ImageResource>(src.texture->get_object());
			dst.image = image->m_image;
			switch (src.type)
			{
			case TextureViewType::tex1d:
				dst.viewType = VK_IMAGE_VIEW_TYPE_1D;
				dst.subresourceRange.baseMipLevel = src.mip_slice;
				dst.subresourceRange.levelCount = src.mip_size;
				dst.subresourceRange.baseArrayLayer = 0;
				dst.subresourceRange.layerCount = 1;
				break;
			case TextureViewType::tex1darray:
				dst.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
				dst.subresourceRange.baseMipLevel = src.mip_slice;
				dst.subresourceRange.levelCount = src.mip_size;
				dst.subresourceRange.baseArrayLayer = src.array_slice;
				dst.subresourceRange.layerCount = src.array_size;
				break;
			case TextureViewType::tex2d:
				dst.viewType = VK_IMAGE_VIEW_TYPE_2D;
				dst.subresourceRange.baseMipLevel = src.mip_slice;
				dst.subresourceRange.levelCount = src.mip_size;
				dst.subresourceRange.baseArrayLayer = 0;
				dst.subresourceRange.layerCount = 1;
				break;
			case TextureViewType::tex2darray:
				dst.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
				dst.subresourceRange.baseMipLevel = src.mip_slice;
				dst.subresourceRange.levelCount = src.mip_size;
				dst.subresourceRange.baseArrayLayer = src.array_slice;
				dst.subresourceRange.layerCount = src.array_size;
				break;
			case TextureViewType::texcube:
				dst.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
				dst.subresourceRange.baseMipLevel = src.mip_slice;
				dst.subresourceRange.levelCount = src.mip_size;
				dst.subresourceRange.baseArrayLayer = src.array_slice;
				dst.subresourceRange.layerCount = src.array_size;
				break;
			case TextureViewType::texcubearray:
				dst.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
				dst.subresourceRange.baseMipLevel = src.mip_slice;
				dst.subresourceRange.levelCount = src.mip_size;
				dst.subresourceRange.baseArrayLayer = src.array_slice;
				dst.subresourceRange.layerCount = src.array_size;
				break;
			case TextureViewType::tex3d:
				dst.viewType = VK_IMAGE_VIEW_TYPE_3D;
				dst.subresourceRange.baseMipLevel = src.mip_slice;
				dst.subresourceRange.levelCount = src.mip_size;
				dst.subresourceRange.baseArrayLayer = 0;
				dst.subresourceRange.layerCount = 1;
				break;
			}
			dst.format = encode_format(src.format);
			dst.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			dst.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			dst.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			dst.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			dst.subresourceRange.aspectMask = is_depth_stencil_format(src.format) ?
				VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
				VK_IMAGE_ASPECT_COLOR_BIT;
		}
		void DescriptorSet::update_descriptors(Span<const DescriptorSetWrite> writes, Span<const DescriptorSetCopy> copies)
		{
			VkWriteDescriptorSet* d_writes = nullptr;
			u32 num_writes = (u32)writes.size();
			if (num_writes)
			{
				d_writes = (VkWriteDescriptorSet*)alloca(sizeof(VkWriteDescriptorSet) * num_writes);
				memzero(d_writes, sizeof(VkWriteDescriptorSet) * num_writes);
				for (u32 i = 0; i < num_writes; ++i)
				{
					VkWriteDescriptorSet& d = d_writes[i];
					auto& s = writes[i];
					d.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					d.dstSet = m_desc_set;
					d.dstBinding = s.binding_slot;
					d.dstArrayElement = s.first_array_index;
					d.descriptorType = encode_descriptor_type(s.type);
					switch (s.type)
					{
					case DescriptorType::uniform_buffer_view:
					case DescriptorType::read_buffer_view:
					case DescriptorType::read_write_buffer_view:
					{
						d.descriptorCount = (u32)s.buffer_views.size();
						if (d.descriptorCount)
						{
							VkDescriptorBufferInfo* infos = (VkDescriptorBufferInfo*)alloca(sizeof(VkDescriptorBufferInfo) * d.descriptorCount);
							for (u32 j = 0; j < d.descriptorCount; ++j)
							{
								auto& s_buffer = s.buffer_views[j];
								auto& d_buffer = infos[j];
								BufferResource* buf = cast_objct<BufferResource>(s_buffer.buffer->get_object());
								d_buffer.buffer = buf->m_buffer;
								d_buffer.offset = s_buffer.offset;
								if (s.type == DescriptorType::uniform_buffer_view)
								{
									d_buffer.range = s_buffer.element_size;
								}
								else
								{
									if (s_buffer.format != Format::unknown)
									{
										d_buffer.range = bits_per_pixel(s_buffer.format) * s_buffer.element_count / 8;
									}
									else
									{
										d_buffer.range = s_buffer.element_size * s_buffer.element_count;
									}
								}
							}
							d.pBufferInfo = infos;
						}
					}
					break;
					case DescriptorType::sampled_texture_view:
					case DescriptorType::read_texture_view:
					case DescriptorType::read_write_texture_view:
					{
						d.descriptorCount = (u32)s.texture_views.size();
						if (d.descriptorCount)
						{
							VkDescriptorImageInfo* infos = (VkDescriptorImageInfo*)alloca(sizeof(VkDescriptorImageInfo) * d.descriptorCount);
							memzero(infos, sizeof(VkDescriptorImageInfo) * d.descriptorCount);
							for (u32 j = 0; j < d.descriptorCount; ++j)
							{
								auto& s_image = s.texture_views[j];
								auto& d_image = infos[j];
								if (s.type == DescriptorType::sampled_texture_view ||
									s.type == DescriptorType::read_texture_view)
								{
									d_image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
								}
								else
								{
									d_image.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
								}
								VkImageViewCreateInfo create_info{};
								encode_image_view_create_info(create_info, s_image);
								
								
							}
						}
					}
					break;
					case DescriptorType::sampler:
						d.descriptorCount = (u32)s.samplers.size(); break;
					}
				}

			}
		}
	}
}