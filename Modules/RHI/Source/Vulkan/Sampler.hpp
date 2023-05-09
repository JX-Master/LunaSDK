/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Sampler.hpp
* @author JXMaster
* @date 2023/5/4
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct Sampler
		{
			lustruct("RHI::Sampler", "{CF6D9848-D8CB-4F29-8BD7-270D4D123EC1}");

			Ref<Device> m_device;
			VkSampler m_sampler = VK_NULL_HANDLE;

			RV init(const SamplerDesc& desc);
			~Sampler();
		};
	}
}