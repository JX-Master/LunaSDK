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
		inline VkImageAspectFlags get_aspect_mask_from_format(VkFormat format)
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
				result = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
				break;
				// Assume everything else is Color
			default:
				result = VK_IMAGE_ASPECT_COLOR_BIT;
				break;
			}
			return result;
		}
		void encode_image_view_create_info(VkImageViewCreateInfo& dst, const TextureViewDesc& src)
		{
			dst.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			dst.flags = 0;
			ImageResource* image = cast_object<ImageResource>(src.texture->get_object());
			dst.image = image->m_image;
			switch (src.type)
			{
			case TextureViewType::tex1d:
				dst.viewType = VK_IMAGE_VIEW_TYPE_1D;
				break;
			case TextureViewType::tex1darray:
				dst.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
				break;
			case TextureViewType::tex2d:
			case TextureViewType::tex2dms:
				dst.viewType = VK_IMAGE_VIEW_TYPE_2D;
				break;
			case TextureViewType::tex2darray:
			case TextureViewType::tex2dmsarray:
				dst.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
				break;
			case TextureViewType::texcube:
				dst.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
				break;
			case TextureViewType::texcubearray:
				dst.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
				break;
			case TextureViewType::tex3d:
				dst.viewType = VK_IMAGE_VIEW_TYPE_3D;
				break;
			default: lupanic();
			}
			dst.subresourceRange.baseArrayLayer = src.array_slice;
			dst.subresourceRange.layerCount = src.array_size;
			dst.subresourceRange.baseMipLevel = src.mip_slice;
			dst.subresourceRange.levelCount = src.mip_size;
			dst.format = encode_format(src.format);
			dst.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			dst.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			dst.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			dst.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			dst.subresourceRange.aspectMask = get_aspect_mask_from_format(dst.format);
		}
		RV ImageView::init(const TextureViewDesc& validated_desc)
		{
			lutry
			{
				m_desc = validated_desc;
				VkImageViewCreateInfo create_info{};
				encode_image_view_create_info(create_info, m_desc);
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