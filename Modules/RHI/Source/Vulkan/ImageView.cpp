/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ImageView.cpp
* @author JXMaster
* @date 2023/5/4
*/
#include "ImageView.hpp"
#include "Resource.hpp"

namespace Luna
{
	namespace RHI
	{
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
		RV ImageView::init(const TextureViewDesc& desc)
		{
			lutry
			{
				VkImageViewCreateInfo create_info{};
				encode_image_view_create_info(create_info, desc);
				luexp(encode_vk_result(m_device->m_funcs.vkCreateImageView(m_device->m_device, &create_info, nullptr, &m_image_view)));
			}
			lucatchret;
			return ok;
		}
		ImageView::~ImageView()
		{
			if (m_image_view != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroyImageView(m_device->m_device, m_image_view, nullptr);
				m_image_view = VK_NULL_HANDLE;
			}
		}
	}
}