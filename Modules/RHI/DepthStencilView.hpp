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
		namespace DSV
		{
			struct Tex1D
			{
				u32 mip_slice;
			};
			struct Tex1DArray
			{
				u32 mip_slice;
				u32 first_array_slice;
				u32 array_size;
			};
			struct Tex2D
			{
				u32 mip_slice;
			};
			struct Tex2DArray
			{
				u32 mip_slice;
				u32 first_array_slice;
				u32 array_size;
			};
			struct Tex2DMS {};
			struct Tex2DMSArray
			{
				u32  first_array_slice;
				u32  array_size;
			};
		}

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
			bool depth_read_only;
			bool stencil_read_only;
			union
			{
				DSV::Tex1D tex1d;
				DSV::Tex1DArray tex1darray;
				DSV::Tex2D tex2d;
				DSV::Tex2DArray tex2darray;
				DSV::Tex2DMS tex2dms;
				DSV::Tex2DMSArray tex2dmsarray;
			};

			static DepthStencilViewDesc as_tex1d(Format _format, u32 _mip_slice, bool _depth_read_only = false, bool _stencil_read_only = false)
			{
				DepthStencilViewDesc desc;
				desc.format = _format;
				desc.type = DepthStencilViewType::tex1d;
				desc.tex1d.mip_slice = _mip_slice;
				desc.depth_read_only = _depth_read_only;
				desc.stencil_read_only = _stencil_read_only;
				return desc;
			}

			static DepthStencilViewDesc as_tex1darray(Format _format, u32 _mip_slice, u32 _first_array_slice, u32 _array_size, bool _depth_read_only = false, bool _stencil_read_only = false)
			{
				DepthStencilViewDesc desc;
				desc.format = _format;
				desc.type = DepthStencilViewType::tex1darray;
				desc.tex1darray.array_size = _array_size;
				desc.tex1darray.first_array_slice = _first_array_slice;
				desc.tex1darray.mip_slice = _mip_slice;
				desc.depth_read_only = _depth_read_only;
				desc.stencil_read_only = _stencil_read_only;
				return desc;
			}

			static DepthStencilViewDesc as_tex2d(Format _format, u32 _mip_slice, bool _depth_read_only = false, bool _stencil_read_only = false)
			{
				DepthStencilViewDesc desc;
				desc.format = _format;
				desc.type = DepthStencilViewType::tex2d;
				desc.tex2d.mip_slice = _mip_slice;
				desc.depth_read_only = _depth_read_only;
				desc.stencil_read_only = _stencil_read_only;
				return desc;
			}

			static DepthStencilViewDesc as_tex2darray(Format _format, u32 _mip_slice, u32 _first_array_slice, u32 _array_size, bool _depth_read_only = false, bool _stencil_read_only = false)
			{
				DepthStencilViewDesc desc;
				desc.format = _format;
				desc.type = DepthStencilViewType::tex2darray;
				desc.tex2darray.array_size = _array_size;
				desc.tex2darray.first_array_slice = _first_array_slice;
				desc.tex2darray.mip_slice = _mip_slice;
				desc.depth_read_only = _depth_read_only;
				desc.stencil_read_only = _stencil_read_only;
				return desc;
			}

			static DepthStencilViewDesc as_tex2dms(Format _format, bool _depth_read_only = false, bool _stencil_read_only = false)
			{
				DepthStencilViewDesc desc;
				desc.format = _format;
				desc.type = DepthStencilViewType::tex2dms;
				desc.depth_read_only = _depth_read_only;
				desc.stencil_read_only = _stencil_read_only;
				return desc;
			}

			static DepthStencilViewDesc as_tex2dmsarray(Format _format, u32 _first_array_slice, u32 _array_size, bool _depth_read_only = false, bool _stencil_read_only = false)
			{
				DepthStencilViewDesc desc;
				desc.format = _format;
				desc.type = DepthStencilViewType::tex2dmsarray;
				desc.tex2dmsarray.first_array_slice = _first_array_slice;
				desc.tex2dmsarray.array_size = _array_size;
				desc.depth_read_only = _depth_read_only;
				desc.stencil_read_only = _stencil_read_only;
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