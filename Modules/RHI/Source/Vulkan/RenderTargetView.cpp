/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file RenderTargetView.cpp
* @author JXMaster
* @date 2023/4/21
*/
#include "RenderTargetView.hpp"
#include "Resource.hpp"
namespace Luna
{
	namespace RHI
	{
		static R<RenderTargetViewDesc> get_default_rtv(ITexture* res)
		{
			TextureDesc d = res->get_desc();
			switch (d.type)
			{
			case TextureType::tex1d:
				return (d.array_size) == 1 ?
					RenderTargetViewDesc::tex1d(d.pixel_format, 0) :
					RenderTargetViewDesc::tex1darray(d.pixel_format, 0, 0, d.array_size);
			case TextureType::tex2d:
				return (d.array_size == 1) ?
					((d.sample_count == 1) ?
						RenderTargetViewDesc::tex2d(d.pixel_format, 0) :
						RenderTargetViewDesc::tex2dms(d.pixel_format)) :
					((d.sample_count == 1) ?
						RenderTargetViewDesc::tex2darray(d.pixel_format, 0, 0, d.array_size) :
						RenderTargetViewDesc::tex2dmsarray(d.pixel_format, 0, d.array_size)
						);
			case TextureType::tex3d:
				return RenderTargetViewDesc::tex3d(d.pixel_format, 0, 0, d.depth);
			default:
				lupanic();
				break;
			}
			return BasicError::failure();
		}
		RV RenderTargetView::init(ITexture* resource, const RenderTargetViewDesc* desc)
		{
			lutry
			{
				RenderTargetViewDesc d;
				if (desc)
				{
					d = *desc;
				}
				else
				{
					luset(d, get_default_rtv(resource));
				}
				m_resource = resource;
				m_desc = d;
				VkImageViewCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				info.flags = 0;
				ImageResource* res = cast_objct<ImageResource>(resource->get_object());
				if (!res) return BasicError::not_supported();
				info.image = res->m_image;
				switch (d.type)
				{
				case RenderTargetViewType::tex1d: 
					info.viewType = VK_IMAGE_VIEW_TYPE_1D; 
					info.subresourceRange.baseMipLevel = d.mip_slice;
					info.subresourceRange.levelCount = 1;
					info.subresourceRange.baseArrayLayer = 0;
					info.subresourceRange.layerCount = 1;
					break;
				case RenderTargetViewType::tex1darray: 
					info.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY; 
					info.subresourceRange.baseMipLevel = d.mip_slice;
					info.subresourceRange.levelCount = 1;
					info.subresourceRange.baseArrayLayer = d.first_depth_or_array_slice;
					info.subresourceRange.layerCount = d.depth_or_array_size;
					break;
				case RenderTargetViewType::tex2d:
					info.viewType = VK_IMAGE_VIEW_TYPE_2D;
					info.subresourceRange.baseMipLevel = d.mip_slice;
					info.subresourceRange.levelCount = 1;
					info.subresourceRange.baseArrayLayer = 0;
					info.subresourceRange.layerCount = 1;
					break;
				case RenderTargetViewType::tex2dms:
					info.viewType = VK_IMAGE_VIEW_TYPE_2D;
					info.subresourceRange.baseMipLevel = 0;
					info.subresourceRange.levelCount = 1;
					info.subresourceRange.baseArrayLayer = 0;
					info.subresourceRange.layerCount = 1;
					break;
				case RenderTargetViewType::tex2darray:
					info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
					info.subresourceRange.baseMipLevel = d.mip_slice;
					info.subresourceRange.levelCount = 1;
					info.subresourceRange.baseArrayLayer = d.first_depth_or_array_slice;
					info.subresourceRange.layerCount = d.depth_or_array_size;
					break;
				case RenderTargetViewType::tex2dmsarray:
					info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
					info.subresourceRange.baseMipLevel = 0;
					info.subresourceRange.levelCount = 1;
					info.subresourceRange.baseArrayLayer = d.first_depth_or_array_slice;
					info.subresourceRange.layerCount = d.depth_or_array_size;
					break;
				case RenderTargetViewType::tex3d:
					info.viewType = VK_IMAGE_VIEW_TYPE_3D; 
					info.subresourceRange.baseMipLevel = d.mip_slice;
					info.subresourceRange.levelCount = 1;
					info.subresourceRange.baseArrayLayer = 0;
					info.subresourceRange.layerCount = 1;
					break;
				}
				info.format = encode_format(res->m_desc.pixel_format);
				info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				luexp(encode_vk_result(m_device->m_funcs.vkCreateImageView(m_device->m_device, &info, nullptr, &m_view)));
			}
			lucatchret;
			return ok;
		}
		RenderTargetView::~RenderTargetView()
		{
			if (m_view != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroyImageView(m_device->m_device, m_view, nullptr);
				m_view = VK_NULL_HANDLE;
			}
		}
	}
}