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
			switch (src.filter)
			{
			case Filter::min_mag_mip_point:
				dst.minFilter = VK_FILTER_NEAREST;
				dst.magFilter = VK_FILTER_NEAREST;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_FALSE;
				break;
			case Filter::min_mag_point_mip_linear:
				dst.minFilter = VK_FILTER_NEAREST;
				dst.magFilter = VK_FILTER_NEAREST;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_FALSE;
				break;
			case Filter::min_point_mag_linear_mip_point:
				dst.minFilter = VK_FILTER_NEAREST;
				dst.magFilter = VK_FILTER_LINEAR;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_FALSE;
				break;
			case Filter::min_point_mag_mip_linear:
				dst.minFilter = VK_FILTER_NEAREST;
				dst.magFilter = VK_FILTER_LINEAR;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_FALSE;
				break;
			case Filter::min_linear_mag_mip_point:
				dst.minFilter = VK_FILTER_LINEAR;
				dst.magFilter = VK_FILTER_NEAREST;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_FALSE;
				break;
			case Filter::min_linear_mag_point_mip_linear:
				dst.minFilter = VK_FILTER_LINEAR;
				dst.magFilter = VK_FILTER_NEAREST;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_FALSE;
				break;
			case Filter::min_mag_linear_mip_point:
				dst.minFilter = VK_FILTER_LINEAR;
				dst.magFilter = VK_FILTER_LINEAR;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_FALSE;
				break;
			case Filter::min_mag_mip_linear:
				dst.minFilter = VK_FILTER_LINEAR;
				dst.magFilter = VK_FILTER_LINEAR;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_FALSE;
				break;
			case Filter::anisotropic:
				dst.minFilter = VK_FILTER_LINEAR;
				dst.magFilter = VK_FILTER_LINEAR;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				dst.anisotropyEnable = VK_TRUE;
				dst.compareEnable = VK_FALSE;
				break;
			case Filter::comparison_min_mag_mip_point:
				dst.minFilter = VK_FILTER_NEAREST;
				dst.magFilter = VK_FILTER_NEAREST;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_TRUE;
				break;
			case Filter::comparison_min_mag_point_mip_linear:
				dst.minFilter = VK_FILTER_NEAREST;
				dst.magFilter = VK_FILTER_NEAREST;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_TRUE;
				break;
			case Filter::comparison_min_point_mag_linear_mip_point:
				dst.minFilter = VK_FILTER_NEAREST;
				dst.magFilter = VK_FILTER_LINEAR;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_TRUE;
				break;
			case Filter::comparison_min_point_mag_mip_linear:
				dst.minFilter = VK_FILTER_NEAREST;
				dst.magFilter = VK_FILTER_LINEAR;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_TRUE;
				break;
			case Filter::comparison_min_linear_mag_mip_point:
				dst.minFilter = VK_FILTER_LINEAR;
				dst.magFilter = VK_FILTER_NEAREST;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_TRUE;
				break;
			case Filter::comparison_min_linear_mag_point_mip_linear:
				dst.minFilter = VK_FILTER_LINEAR;
				dst.magFilter = VK_FILTER_NEAREST;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_TRUE;
				break;
			case Filter::comparison_min_mag_linear_mip_point:
				dst.minFilter = VK_FILTER_LINEAR;
				dst.magFilter = VK_FILTER_LINEAR;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_TRUE;
				break;
			case Filter::comparison_min_mag_mip_linear:
				dst.minFilter = VK_FILTER_LINEAR;
				dst.magFilter = VK_FILTER_LINEAR;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				dst.anisotropyEnable = VK_FALSE;
				dst.compareEnable = VK_TRUE;
				break;
			case Filter::comparison_anisotropic:
				dst.minFilter = VK_FILTER_LINEAR;
				dst.magFilter = VK_FILTER_LINEAR;
				dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				dst.anisotropyEnable = VK_TRUE;
				dst.compareEnable = VK_TRUE;
				break;
			}
			dst.addressModeU = encode_address_mode(src.address_u);
			dst.addressModeV = encode_address_mode(src.address_v);
			dst.addressModeW = encode_address_mode(src.address_w);
			dst.mipLodBias = src.mip_lod_bias;
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