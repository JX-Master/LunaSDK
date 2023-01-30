/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RenderTargetView.cpp
* @author JXMaster
* @date 2022/8/5
*/
#include "RenderTargetView.hpp"

#ifdef LUNA_RHI_D3D12

namespace Luna
{
	namespace RHI
	{
		static R<RenderTargetViewDesc> get_default_rtv(Resource* res)
		{
			ResourceDesc d = res->m_desc;
			if (d.pixel_format == Format::unknown)
			{
				return BasicError::bad_arguments();
			}
			switch (d.type)
			{
			case ResourceType::buffer:
				return BasicError::bad_arguments();
			case ResourceType::texture_1d:
				return (d.depth_or_array_size) == 1 ?
					RenderTargetViewDesc::as_tex1d(d.pixel_format, 0) :
					RenderTargetViewDesc::as_tex1darray(d.pixel_format, 0, 0, d.depth_or_array_size);
			case ResourceType::texture_2d:
				return (d.depth_or_array_size == 1) ?
					((d.sample_count == 1) ?
						RenderTargetViewDesc::as_tex2d(d.pixel_format, 0) :
						RenderTargetViewDesc::as_tex2dms(d.pixel_format)) :
					((d.sample_count == 1) ?
						RenderTargetViewDesc::as_tex2darray(d.pixel_format, 0, 0, d.depth_or_array_size) :
						RenderTargetViewDesc::as_tex2dmsarray(d.pixel_format, 0, d.depth_or_array_size)
						);
			case ResourceType::texture_3d:
				return RenderTargetViewDesc::as_tex3d(d.pixel_format, 0, 0, d.depth_or_array_size);
			default:
				lupanic();
				break;
			}
			return BasicError::failure();
		}
		RV RenderTargetView::init(IResource* resource, const RenderTargetViewDesc* desc)
		{
			lucheck(resource);
			RenderTargetViewDesc d;
			lutry
			{
				Resource * reso = static_cast<Resource*>(resource->get_object());
				if (desc) d = *desc;
				else
				{
					luset(d, get_default_rtv(reso));
				}
				m_desc = d;
				luset(m_heap, m_device->m_rtv_heap.allocate_view());
				m_resource = resource;
				{
					ID3D12Resource* res = reso->m_res.Get();
					D3D12_RENDER_TARGET_VIEW_DESC rtv;
					switch (m_desc.type)
					{
					case RenderTargetViewType::buffer:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
						rtv.Buffer.FirstElement = m_desc.buffer.offset;
						rtv.Buffer.NumElements = m_desc.buffer.count;
						break;
					case RenderTargetViewType::tex1d:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
						rtv.Texture1D.MipSlice = m_desc.tex1d.mip_slice;
						break;
					case RenderTargetViewType::tex1darray:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
						rtv.Texture1DArray.ArraySize = m_desc.tex1darray.array_size;
						rtv.Texture1DArray.FirstArraySlice = m_desc.tex1darray.first_array_slice;
						rtv.Texture1DArray.MipSlice = m_desc.tex1darray.mip_slice;
						break;
					case RenderTargetViewType::tex2d:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
						rtv.Texture2D.MipSlice = m_desc.tex2d.mip_slice;
						rtv.Texture2D.PlaneSlice = 0;
						break;
					case RenderTargetViewType::tex2darray:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
						rtv.Texture2D.MipSlice = m_desc.tex2d.mip_slice;
						rtv.Texture2D.PlaneSlice = 0;
						break;
					case RenderTargetViewType::tex2dms:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
						break;
					case RenderTargetViewType::tex2dmsarray:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
						rtv.Texture2DMSArray.ArraySize = m_desc.tex2dmsarray.array_size;
						rtv.Texture2DMSArray.FirstArraySlice = m_desc.tex2dmsarray.first_array_slice;
						break;
					case RenderTargetViewType::tex3d:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
						rtv.Texture3D.FirstWSlice = m_desc.tex3d.first_layer_slice;
						rtv.Texture3D.MipSlice = m_desc.tex3d.mip_slice;
						rtv.Texture3D.WSize = m_desc.tex3d.layer_size;
						break;
					default:
						lupanic();
					}
					rtv.Format = encode_pixel_format(m_desc.format);
					usize addr = m_heap->GetCPUDescriptorHandleForHeapStart().ptr;
					D3D12_CPU_DESCRIPTOR_HANDLE h;
					h.ptr = addr;
					m_device->m_device->CreateRenderTargetView(res, &rtv, h);
				}
			}
			lucatchret;
			return ok;
		}
	}
}

#endif