/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Resource.cpp
* @author JXMaster
* @date 2023/4/19
*/
#include "Resource.hpp"

namespace Luna
{
	namespace RHI
	{
		inline void encode_buffer_create_info(VkBufferCreateInfo& dest, const ResourceDesc& src)
		{
			dest.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			dest.size = src.width_or_buffer_size;
			dest.usage = 0;
			if (test_flags(src.usages, ResourceUsageFlag::copy_source)) dest.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			if (test_flags(src.usages, ResourceUsageFlag::copy_dest)) dest.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			if (test_flags(src.usages, ResourceUsageFlag::shader_resource)) dest.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			if (test_flags(src.usages, ResourceUsageFlag::constant_buffer)) dest.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			if (test_flags(src.usages, ResourceUsageFlag::unordered_access)) dest.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			if (test_flags(src.usages, ResourceUsageFlag::index_buffer)) dest.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			if (test_flags(src.usages, ResourceUsageFlag::vertex_buffer)) dest.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			if (test_flags(src.usages, ResourceUsageFlag::indirect_buffer)) dest.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			dest.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		inline void encode_allocation_info(VmaAllocationCreateInfo& dest, const ResourceDesc& src)
		{
			switch (src.heap_type)
			{
			case ResourceHeapType::local:
				dest.usage = VMA_MEMORY_USAGE_AUTO;
				dest.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
				dest.priority = 1.0f;
				break;
			case ResourceHeapType::upload:
				dest.usage = VMA_MEMORY_USAGE_AUTO;
				dest.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
				break;
			case ResourceHeapType::readback:
				dest.usage = VMA_MEMORY_USAGE_AUTO;
				dest.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
				break;
			}
		}
		RV BufferResource::init_as_committed(const ResourceDesc& desc)
		{
			lutry
			{
				m_desc = validate_resource_desc(desc);
				VkBufferCreateInfo create_info{};
				encode_buffer_create_info(create_info, m_desc);
				VmaAllocationCreateInfo allocation{};
				encode_allocation_info(allocation, m_desc);
				luexp(encode_vk_result(vmaCreateBuffer(m_device->m_allocator, &create_info, &allocation, &m_buffer, &m_allocation, &m_allocation_info)));
			}
			lucatchret;
			return ok;
		}
		BufferResource::~BufferResource()
		{
			if (m_buffer != VK_NULL_HANDLE && m_allocation != VK_NULL_HANDLE)
			{
				vmaDestroyBuffer(m_device->m_allocator, m_buffer, m_allocation);
				m_buffer = VK_NULL_HANDLE;
				m_allocation = VK_NULL_HANDLE;
			}
			else if (m_buffer != VK_NULL_HANDLE)
			{
				vkDestroyBuffer(m_device->m_device, m_buffer, nullptr);
				m_buffer = VK_NULL_HANDLE;
			}
		}
		inline void encode_image_create_info(VkImageCreateInfo& dest, const ResourceDesc& src)
		{
			dest.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			switch (src.type)
			{
			case ResourceType::texture_1d:
				dest.imageType = VK_IMAGE_TYPE_1D; break;
			case ResourceType::texture_2d:
				dest.imageType = VK_IMAGE_TYPE_2D; break;
			case ResourceType::texture_3d:
				dest.imageType = VK_IMAGE_TYPE_3D; break;
			default: lupanic();
			}
			dest.extent.width = (u32)src.width_or_buffer_size;
			dest.extent.height = 
				(src.type == ResourceType::texture_2d || src.type == ResourceType::texture_3d) ? src.height : 1;
			dest.extent.depth = src.type == ResourceType::texture_3d ? src.depth_or_array_size : 1;
			dest.mipLevels = src.mip_levels;
			dest.arrayLayers =
				(src.type == ResourceType::texture_1d || src.type == ResourceType::texture_2d) ? 
				src.depth_or_array_size : 1;
			dest.format = encode_format(src.pixel_format);
			dest.tiling = VK_IMAGE_TILING_OPTIMAL;
			dest.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			dest.usage = 0;
			if (test_flags(src.usages, ResourceUsageFlag::copy_source)) dest.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			if (test_flags(src.usages, ResourceUsageFlag::copy_dest)) dest.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if (test_flags(src.usages, ResourceUsageFlag::shader_resource)) dest.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if (test_flags(src.usages, ResourceUsageFlag::unordered_access)) dest.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
			if (test_flags(src.usages, ResourceUsageFlag::render_target)) dest.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			if (test_flags(src.usages, ResourceUsageFlag::depth_stencil)) dest.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			dest.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			switch (src.sample_count)
			{
			case 0:
			case 1: dest.samples = VK_SAMPLE_COUNT_1_BIT; break;
			case 2: dest.samples = VK_SAMPLE_COUNT_2_BIT; break;
			case 4: dest.samples = VK_SAMPLE_COUNT_4_BIT; break;
			case 8: dest.samples = VK_SAMPLE_COUNT_8_BIT; break;
			case 16: dest.samples = VK_SAMPLE_COUNT_16_BIT; break;
			case 32: dest.samples = VK_SAMPLE_COUNT_32_BIT; break;
			case 64: dest.samples = VK_SAMPLE_COUNT_64_BIT; break;
			}
		}
		RV ImageResource::init_as_committed(const ResourceDesc& desc)
		{
			lutry
			{
				m_desc = validate_resource_desc(desc);
				VkImageCreateInfo create_info{};
				encode_image_create_info(create_info, m_desc);
				VmaAllocationCreateInfo allocation{};
				encode_allocation_info(allocation, m_desc);
				luexp(encode_vk_result(vmaCreateImage(m_device->m_allocator, &create_info, &allocation, &m_image, &m_allocation, &m_allocation_info)));
			}
			lucatchret;
			return ok;
		}
		ImageResource::~ImageResource()
		{
			if (m_image != VK_NULL_HANDLE && m_allocation != VK_NULL_HANDLE)
			{
				vmaDestroyImage(m_device->m_allocator, m_image, m_allocation);
				m_image = VK_NULL_HANDLE;
				m_allocation = VK_NULL_HANDLE;
			}
			else if (m_image != VK_NULL_HANDLE)
			{
				vkDestroyImage(m_device->m_device, m_image, nullptr);
				m_image = VK_NULL_HANDLE;
			}
		}
	}
}