/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DepthStencilView.hpp
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
		enum class DepthStencilViewType
		{
			unknown = 0,
			tex1d,
			tex1darray,
			tex2d,
			tex2darray,
			tex2dms,
			tex2dmsarray
		};

		struct DepthStencilViewDesc
		{
			Format format;
			DepthStencilViewType type;
			u32 mip_slice;
			u32 first_array_slice;
			u32 array_size;

			static DepthStencilViewDesc as_tex1d(Format format, u32 mip_slice)
			{
				DepthStencilViewDesc desc;
				desc.format = format;
				desc.type = DepthStencilViewType::tex1d;
				desc.mip_slice = mip_slice;
				desc.first_array_slice = 0;
				desc.array_size = 1;
				return desc;
			}
			static DepthStencilViewDesc as_tex1darray(Format format, u32 mip_slice, u32 first_array_slice, u32 array_size)
			{
				DepthStencilViewDesc desc;
				desc.format = format;
				desc.type = DepthStencilViewType::tex1darray;
				desc.mip_slice = mip_slice;
				desc.array_size = array_size;
				desc.first_array_slice = first_array_slice;
				return desc;
			}
			static DepthStencilViewDesc as_tex2d(Format format, u32 mip_slice)
			{
				DepthStencilViewDesc desc;
				desc.format = format;
				desc.type = DepthStencilViewType::tex2d;
				desc.mip_slice = mip_slice;
				desc.first_array_slice = 0;
				desc.array_size = 1;
				return desc;
			}
			static DepthStencilViewDesc as_tex2darray(Format format, u32 mip_slice, u32 first_array_slice, u32 array_size)
			{
				DepthStencilViewDesc desc;
				desc.format = format;
				desc.type = DepthStencilViewType::tex2darray;
				desc.array_size = array_size;
				desc.first_array_slice = first_array_slice;
				desc.mip_slice = mip_slice;
				return desc;
			}
			static DepthStencilViewDesc as_tex2dms(Format format)
			{
				DepthStencilViewDesc desc;
				desc.format = format;
				desc.type = DepthStencilViewType::tex2dms;
				desc.mip_slice = 0;
				desc.first_array_slice = 0;
				desc.array_size = 1;
				return desc;
			}
			static DepthStencilViewDesc as_tex2dmsarray(Format format, u32 first_array_slice, u32 array_size)
			{
				DepthStencilViewDesc desc;
				desc.format = format;
				desc.type = DepthStencilViewType::tex2dmsarray;
				desc.mip_slice = 0;
				desc.first_array_slice = first_array_slice;
				desc.array_size = array_size;
				return desc;
			}
		};

		struct IDepthStencilView : virtual IDeviceChild
		{
			luiid("{C672876F-C6BB-49CA-BE77-DA112069F0C1}");

			virtual IResource* get_resource() = 0;
			virtual DepthStencilViewDesc get_desc() = 0;
		};
	}
}