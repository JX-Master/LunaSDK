/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Sampler.cpp
* @author JXMaster
* @date 2023/5/4
*/
#include "Sampler.hpp"

namespace Luna
{
	namespace RHI
	{
		inline void encode_sampler_create_info(VkSamplerCreateInfo& dst, const SamplerDesc& src)
		{
			dst.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			dst.flags = 0;
			dst.minFilter = encode_filter(src.min_filter);
			dst.magFilter = encode_filter(src.mag_filter);
			dst.mipmapMode = encode_mipmap_mode(src.mip_filter);
			dst.anisotropyEnable = src.anisotropy_enable ? VK_TRUE : VK_FALSE;
			dst.compareEnable = src.compare_enable ? VK_TRUE : VK_FALSE;
			dst.addressModeU = encode_address_mode(src.address_u);
			dst.addressModeV = encode_address_mode(src.address_v);
			dst.addressModeW = encode_address_mode(src.address_w);
			dst.mipLodBias = 0;
			dst.maxAnisotropy = src.max_anisotropy;
			dst.compareOp = encode_compare_op(src.compare_function);
			dst.minLod = src.min_lod;
			dst.maxLod = src.max_lod;
			switch (src.border_color)
			{
			case BorderColor::float_0000: dst.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK; break;
			case BorderColor::int_0000: dst.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK; break;
			case BorderColor::float_0001: dst.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK; break;
			case BorderColor::int_0001: dst.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; break;
			case BorderColor::float_1111: dst.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE; break;
			case BorderColor::int_1111: dst.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE; break;
			}
			dst.unnormalizedCoordinates = VK_FALSE;
		}
		RV Sampler::init(const SamplerDesc& desc)
		{
			lutry
			{
				VkSamplerCreateInfo create_info{};
				encode_sampler_create_info(create_info, desc);
				luexp(encode_vk_result(m_device->m_funcs.vkCreateSampler(m_device->m_device, &create_info, nullptr, &m_sampler)));
			}
			lucatchret;
			return ok;
		}
		Sampler::~Sampler()
		{
			if (m_sampler != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroySampler(m_device->m_device, m_sampler, nullptr);
				m_sampler = VK_NULL_HANDLE;
			}
		}
	}
}