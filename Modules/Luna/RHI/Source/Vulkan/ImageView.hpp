/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ImageView.hpp
* @author JXMaster
* @date 2023/5/4
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct ImageView
		{
			lustruct("RHI::ImageView", "{088541B4-3F57-4E31-AE0E-0AF6A08A8F21}");

			Ref<Device> m_device;
			VkImageView m_image_view = VK_NULL_HANDLE;
			TextureViewDesc m_desc;

			RV init(const TextureViewDesc& validated_desc);
			~ImageView();
		};
	}
}