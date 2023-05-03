/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RenderTargetView.hpp
* @author JXMaster
* @date 2022/8/5
*/
#pragma once
#include "DeviceChild.hpp"
#include "Resource.hpp"

namespace Luna
{
	namespace RHI
	{
		enum class RenderTargetViewType : u32
		{
			unknown = 0,
			tex1d,
			tex1darray,
			tex2d,
			tex2darray,
			tex2dms,
			tex2dmsarray,
			tex3d
		};

		struct RenderTargetViewDesc
		{
			Format format;
			RenderTargetViewType type;
			u32 mip_slice;
			u32 first_depth_or_array_slice;
			u32 depth_or_array_size;

			static RenderTargetViewDesc tex1d(Format format, u32 mip_slice)
			{
				RenderTargetViewDesc desc;
				desc.format = format;
				desc.type = RenderTargetViewType::tex1d;
				desc.mip_slice = mip_slice;
				desc.first_depth_or_array_slice = 0;
				desc.depth_or_array_size = 1;
				return desc;
			}
			static RenderTargetViewDesc tex1darray(Format format, u32 mip_slice, u32 first_array_slice, u32 array_size)
			{
				RenderTargetViewDesc desc;
				desc.format = format;
				desc.type = RenderTargetViewType::tex1darray;
				desc.mip_slice = mip_slice;
				desc.first_depth_or_array_slice = first_array_slice;
				desc.depth_or_array_size = array_size;
				return desc;
			}
			static RenderTargetViewDesc tex2d(Format format, u32 mip_slice)
			{
				RenderTargetViewDesc desc;
				desc.format = format;
				desc.type = RenderTargetViewType::tex2d;
				desc.mip_slice = mip_slice;
				desc.first_depth_or_array_slice = 0;
				desc.depth_or_array_size = 1;
				return desc;
			}
			static RenderTargetViewDesc tex2darray(Format format, u32 mip_slice, u32 first_array_slice, u32 array_size)
			{
				RenderTargetViewDesc desc;
				desc.format = format;
				desc.type = RenderTargetViewType::tex2darray;
				desc.mip_slice = mip_slice;
				desc.first_depth_or_array_slice = first_array_slice;
				desc.depth_or_array_size = array_size;
				return desc;
			}
			static RenderTargetViewDesc tex2dms(Format format)
			{
				RenderTargetViewDesc desc;
				desc.format = format;
				desc.type = RenderTargetViewType::tex2dms;
				desc.mip_slice = 0; // MSAA textures only have one mip.
				desc.first_depth_or_array_slice = 0;
				desc.depth_or_array_size = 1;
				return desc;
			}
			static RenderTargetViewDesc tex2dmsarray(Format format, u32 first_array_slice, u32 array_size)
			{
				RenderTargetViewDesc desc;
				desc.format = format;
				desc.type = RenderTargetViewType::tex2dmsarray;
				desc.mip_slice = 0; // MSAA textures only have one mip.
				desc.first_depth_or_array_slice = first_array_slice;
				desc.depth_or_array_size = array_size;
				return desc;
			}
			static RenderTargetViewDesc tex3d(Format format, u32 mip_slice, u32 first_layer_slice, u32 layer_size)
			{
				RenderTargetViewDesc desc;
				desc.format = format;
				desc.type = RenderTargetViewType::tex3d;
				desc.mip_slice = mip_slice;
				desc.first_depth_or_array_slice = first_layer_slice;
				desc.depth_or_array_size = layer_size;
				return desc;
			}
		};

		struct IRenderTargetView : virtual IDeviceChild
		{
			luiid("{C672876F-C6BB-49CA-BE77-DA112069F0C1}");

			virtual ITexture* get_resource() = 0;
			virtual RenderTargetViewDesc get_desc() = 0;
		};
	}
}