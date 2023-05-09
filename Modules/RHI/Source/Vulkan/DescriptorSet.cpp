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
				MutexGuard guard(m_device->m_desc_pool_mtx);
				luexp(encode_vk_result(m_device->m_funcs.vkAllocateDescriptorSets(m_device->m_device, &alloc_info, &m_desc_set)));
			}
			lucatchret;
			return ok;
		}
		DescriptorSet::~DescriptorSet()
		{
			if (m_desc_set != VK_NULL_HANDLE)
			{
				MutexGuard guard(m_device->m_desc_pool_mtx);
				m_device->m_funcs.vkFreeDescriptorSets(m_device->m_device, m_device->m_desc_pool, 1, &m_desc_set);
				m_desc_set = VK_NULL_HANDLE;
			}
		}
		RV DescriptorSet::update_descriptors(Span<const DescriptorSetWrite> writes, Span<const DescriptorSetCopy> copies)
		{
			lutry
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
									auto image_view = new_object<ImageView>();
									image_view->m_device = m_device;
									luexp(image_view->init(s_image));
									m_image_views.insert_or_assign(s.binding_slot + s.first_array_index + j, image_view);
									d_image.imageView = image_view->m_image_view;
									d_image.sampler = VK_NULL_HANDLE;
								}
								d.pImageInfo = infos;
							}
						}
						break;
						case DescriptorType::sampler:
						{
							d.descriptorCount = (u32)s.samplers.size();
							if (d.descriptorCount)
							{
								VkDescriptorImageInfo* infos = (VkDescriptorImageInfo*)alloca(sizeof(VkDescriptorImageInfo) * d.descriptorCount);
								memzero(infos, sizeof(VkDescriptorImageInfo) * d.descriptorCount);
								for (u32 j = 0; j < d.descriptorCount; ++j)
								{
									auto& s_sampler = s.samplers[j];
									auto& d_sampler = infos[j];
									d_sampler.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
									d_sampler.imageView = VK_NULL_HANDLE;
									auto sampler = new_object<Sampler>();
									sampler->m_device = m_device;
									luexp(sampler->init(s_sampler));
									m_samplers.insert_or_assign(s.binding_slot + s.first_array_index + j, sampler);
									d_sampler.sampler = sampler->m_sampler;
								}
								d.pImageInfo = infos;
							}
						}
						break;
						}
					}
				}
				VkCopyDescriptorSet* d_copies = nullptr;
				u32 num_copies = (u32)copies.size();
				if (num_copies)
				{
					d_copies = (VkCopyDescriptorSet*)alloca(sizeof(VkCopyDescriptorSet) * num_copies);
					memzero(d_copies, sizeof(VkCopyDescriptorSet)* num_copies);
					for (u32 i = 0; i < num_copies; ++i)
					{
						VkCopyDescriptorSet& d = d_copies[i];
						const DescriptorSetCopy& s = copies[i];
						d.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
						DescriptorSet* src = cast_objct<DescriptorSet>(s.src->get_object());
						d.srcSet = src->m_desc_set;
						d.srcBinding = s.src_binding_slot;
						d.srcArrayElement = s.src_first_array_index;
						d.dstSet = m_desc_set;
						d.dstBinding = s.dst_binding_slot;
						d.dstArrayElement = s.dst_first_array_index;
						d.descriptorCount = s.num_descs;
						for (u32 j = 0; j < s.num_descs; ++j)
						{
							u32 dst_slot = s.dst_binding_slot + s.dst_first_array_index + i;
							u32 src_slot = s.src_binding_slot + s.src_first_array_index + i;
							auto iter1 = src->m_image_views.find(src_slot);
							if (iter1 != src->m_image_views.end()) m_image_views.insert_or_assign(dst_slot, iter1->second);
							auto iter2 = src->m_samplers.find(src_slot);
							if (iter2 != src->m_samplers.end()) m_samplers.insert_or_assign(dst_slot, iter2->second);
						}
					}
				}
				m_device->m_funcs.vkUpdateDescriptorSets(m_device->m_device, num_writes, d_writes, num_copies, d_copies);
			}
			lucatchret;
			return ok;
		}
	}
}