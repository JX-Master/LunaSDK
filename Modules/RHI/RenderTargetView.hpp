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
		namespace RTV
		{
			struct Buffer
			{
				//! Number of bytes between the beginning of the buffer and the first element to access.
				u64 offset;
				//! The total number of elements in the view.
				u32 count;
			};
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
				u32 first_array_slice;
				u32 array_size;
			};
			struct Tex3D
			{
				u32 mip_slice;
				u32 first_layer_slice;
				u32 layer_size;
			};
		}

		enum class RenderTargetViewType : u32
		{
			unknown = 0,
			buffer,
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
			union
			{
				RTV::Buffer buffer;
				RTV::Tex1D tex1d;
				RTV::Tex1DArray tex1darray;
				RTV::Tex2D tex2d;
				RTV::Tex2DArray tex2darray;
				RTV::Tex2DMS tex2dms;
				RTV::Tex2DMSArray tex2dmsarray;
				RTV::Tex3D tex3d;
			};

			static RenderTargetViewDesc as_buffer(Format _format, u64 _offset, u32 _count)
			{
				RenderTargetViewDesc desc;
				desc.format = _format;
				desc.type = RenderTargetViewType::buffer;
				desc.buffer.offset = _offset;
				desc.buffer.count = _count;
				return desc;
			}

			static RenderTargetViewDesc as_tex1d(Format _format, u32 _mip_slice)
			{
				RenderTargetViewDesc desc;
				desc.format = _format;
				desc.type = RenderTargetViewType::tex1d;
				desc.tex1d.mip_slice = _mip_slice;
				return desc;
			}

			static RenderTargetViewDesc as_tex1darray(Format _format, u32 _mip_slice, u32 _first_array_slice, u32 _array_size)
			{
				RenderTargetViewDesc desc;
				desc.format = _format;
				desc.type = RenderTargetViewType::tex1darray;
				desc.tex1darray.array_size = _array_size;
				desc.tex1darray.first_array_slice = _first_array_slice;
				desc.tex1darray.mip_slice = _mip_slice;
				return desc;
			}

			static RenderTargetViewDesc as_tex2d(Format _format, u32 _mip_slice)
			{
				RenderTargetViewDesc desc;
				desc.format = _format;
				desc.type = RenderTargetViewType::tex2d;
				desc.tex2d.mip_slice = _mip_slice;
				return desc;
			}

			static RenderTargetViewDesc as_tex2darray(Format _format, u32 _mip_slice, u32 _first_array_slice, u32 _array_size)
			{
				RenderTargetViewDesc desc;
				desc.format = _format;
				desc.type = RenderTargetViewType::tex2darray;
				desc.tex2darray.array_size = _array_size;
				desc.tex2darray.first_array_slice = _first_array_slice;
				desc.tex2darray.mip_slice = _mip_slice;
				return desc;
			}

			static RenderTargetViewDesc as_tex2dms(Format _format)
			{
				RenderTargetViewDesc desc;
				desc.format = _format;
				desc.type = RenderTargetViewType::tex2dms;
				return desc;
			}

			static RenderTargetViewDesc as_tex2dmsarray(Format _format, u32 _first_array_slice, u32 _array_size)
			{
				RenderTargetViewDesc desc;
				desc.format = _format;
				desc.type = RenderTargetViewType::tex2dmsarray;
				desc.tex2dmsarray.first_array_slice = _first_array_slice;
				desc.tex2dmsarray.array_size = _array_size;
				return desc;
			}

			static RenderTargetViewDesc as_tex3d(Format _format, u32 _mip_slice, u32 _first_layer_slice, u32 _layer_size)
			{
				RenderTargetViewDesc desc;
				desc.format = _format;
				desc.type = RenderTargetViewType::tex3d;
				desc.tex3d.first_layer_slice = _first_layer_slice;
				desc.tex3d.layer_size = _layer_size;
				desc.tex3d.mip_slice = _mip_slice;
				return desc;
			}
		};

		struct IRenderTargetView : virtual IDeviceChild
		{
			luiid("{C672876F-C6BB-49CA-BE77-DA112069F0C1}");

			virtual IResource* get_resource() = 0;
			virtual RenderTargetViewDesc get_desc() = 0;
		};
	}
}