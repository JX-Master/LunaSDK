/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ResolveTargetView.cpp
* @author JXMaster
* @date 2023/4/21
*/
#include "ResolveTargetView.hpp"
#include "Resource.hpp"
namespace Luna
{
	namespace RHI
	{
		RV ResolveTargetView::init(IResource* resource, const ResolveTargetViewDesc* desc)
		{
			lutry
			{
				ResolveTargetViewDesc d;
				if (desc)
				{
					d = *desc;
				}
				else
				{
					d.mip_slice = 0;
					d.array_slice = 0;
				}
				m_resource = resource;
				m_desc = d;
				VkImageViewCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				info.flags = 0;
				ImageResource* res = cast_objct<ImageResource>(resource->get_object());
				if (!res) return BasicError::not_supported();
				info.image = res->m_image;
				info.viewType = VK_IMAGE_VIEW_TYPE_2D;
				info.format = encode_format(res->m_desc.pixel_format);
				info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				info.subresourceRange.baseMipLevel = d.mip_slice;
				info.subresourceRange.levelCount = 1;
				info.subresourceRange.baseArrayLayer = d.array_slice;
				info.subresourceRange.layerCount = 1;
				luexp(encode_vk_result(m_device->m_funcs.vkCreateImageView(m_device->m_device, &info, nullptr, &m_view)));
			}
			lucatchret;
			return ok;
		}
		ResolveTargetView::~ResolveTargetView()
		{
			if (m_view != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroyImageView(m_device->m_device, m_view, nullptr);
				m_view = VK_NULL_HANDLE;
			}
		}
	}
}