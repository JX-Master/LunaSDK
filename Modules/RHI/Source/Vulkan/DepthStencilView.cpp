/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DepthStencilView.cpp
* @author JXMaster
* @date 2023/4/23
*/
#include "DepthStencilView.hpp"
#include "Resource.hpp"
namespace Luna
{
	namespace RHI
	{
		inline VkImageAspectFlags get_aspect_mask_from_format(VkFormat format, bool include_stencil_bit)
		{
			VkImageAspectFlags result = 0;
			switch (format)
			{
				// Depth
			case VK_FORMAT_D16_UNORM:
			case VK_FORMAT_X8_D24_UNORM_PACK32:
			case VK_FORMAT_D32_SFLOAT:
				result = VK_IMAGE_ASPECT_DEPTH_BIT;
				break;
				// Stencil
			case VK_FORMAT_S8_UINT:
				result = VK_IMAGE_ASPECT_STENCIL_BIT;
				break;
				// Depth/stencil
			case VK_FORMAT_D16_UNORM_S8_UINT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				result = VK_IMAGE_ASPECT_DEPTH_BIT;
				if (include_stencil_bit)
					result |= VK_IMAGE_ASPECT_STENCIL_BIT;
				break;
				// Assume everything else is Color
			default:
				result = VK_IMAGE_ASPECT_COLOR_BIT;
				break;
			}
			return result;
		}
		static R<DepthStencilViewDesc> get_default_dsv(ITexture* res)
		{
			TextureDesc d = res->get_desc();
			if (d.pixel_format != Format::d16_unorm &&
				d.pixel_format != Format::d24_unorm_s8_uint &&
				d.pixel_format != Format::d32_float &&
				d.pixel_format != Format::d32_float_s8_uint_x24)
			{
				return BasicError::bad_arguments();
			}
			switch (d.type)
			{
			case TextureType::tex3d:
				return BasicError::bad_arguments();
			case TextureType::tex1d:
				return (d.array_size) == 1 ?
					DepthStencilViewDesc::as_tex1d(d.pixel_format, 0) :
					DepthStencilViewDesc::as_tex1darray(d.pixel_format, 0, 0, d.array_size);
			case TextureType::tex2d:
				return (d.array_size == 1) ?
					((d.sample_count == 1) ?
						DepthStencilViewDesc::as_tex2d(d.pixel_format, 0) :
						DepthStencilViewDesc::as_tex2dms(d.pixel_format)) :
					((d.sample_count == 1) ?
						DepthStencilViewDesc::as_tex2darray(d.pixel_format, 0, 0, d.array_size) :
						DepthStencilViewDesc::as_tex2dmsarray(d.pixel_format, 0, d.array_size)
						);
			default:
				lupanic();
				break;
			}
			return BasicError::failure();
		}
		RV DepthStencilView::init(ITexture* resource, const DepthStencilViewDesc* desc)
		{
			lutry
			{
				DepthStencilViewDesc d;
				if (desc)
				{
					d = *desc;
				}
				else
				{
					luset(d, get_default_dsv(resource));
				}
				m_resource = resource;
				m_desc = d;
				VkImageViewCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				info.flags = 0;
				ImageResource* res = cast_object<ImageResource>(resource->get_object());
				if (!res) return BasicError::not_supported();
				info.image = res->m_image;
				switch (d.type)
				{
				case DepthStencilViewType::tex1d:
					info.viewType = VK_IMAGE_VIEW_TYPE_1D;
					info.subresourceRange.baseMipLevel = d.mip_slice;
					info.subresourceRange.levelCount = 1;
					info.subresourceRange.baseArrayLayer = 0;
					info.subresourceRange.layerCount = 1;
					break;
				case DepthStencilViewType::tex1darray:
					info.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
					info.subresourceRange.baseMipLevel = d.mip_slice;
					info.subresourceRange.levelCount = 1;
					info.subresourceRange.baseArrayLayer = d.first_array_slice;
					info.subresourceRange.layerCount = d.array_size;
					break;
				case DepthStencilViewType::tex2d:
					info.viewType = VK_IMAGE_VIEW_TYPE_2D;
					info.subresourceRange.baseMipLevel = d.mip_slice;
					info.subresourceRange.levelCount = 1;
					info.subresourceRange.baseArrayLayer = 0;
					info.subresourceRange.layerCount = 1;
					break;
				case DepthStencilViewType::tex2dms:
					info.viewType = VK_IMAGE_VIEW_TYPE_2D;
					info.subresourceRange.baseMipLevel = 0;
					info.subresourceRange.levelCount = 1;
					info.subresourceRange.baseArrayLayer = 0;
					info.subresourceRange.layerCount = 1;
					break;
				case DepthStencilViewType::tex2darray:
					info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
					info.subresourceRange.baseMipLevel = d.mip_slice;
					info.subresourceRange.levelCount = 1;
					info.subresourceRange.baseArrayLayer = d.first_array_slice;
					info.subresourceRange.layerCount = d.array_size;
					break;
				case DepthStencilViewType::tex2dmsarray:
					info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
					info.subresourceRange.baseMipLevel = 0;
					info.subresourceRange.levelCount = 1;
					info.subresourceRange.baseArrayLayer = d.first_array_slice;
					info.subresourceRange.layerCount = d.array_size;
					break;
				}
				info.viewType = VK_IMAGE_VIEW_TYPE_2D;
				info.format = encode_format(res->m_desc.pixel_format);
				info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.subresourceRange.aspectMask = get_aspect_mask_from_format(info.format, true);
				luexp(encode_vk_result(m_device->m_funcs.vkCreateImageView(m_device->m_device, &info, nullptr, &m_view)));
			}
			lucatchret;
			return ok;
		}
		DepthStencilView::~DepthStencilView()
		{
			if (m_view != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroyImageView(m_device->m_device, m_view, nullptr);
				m_view = VK_NULL_HANDLE;
			}
		}
	}
}