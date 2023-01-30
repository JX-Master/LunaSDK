/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DepthStencilView.cpp
* @author JXMaster
* @date 2022/8/5
*/
#include "DepthStencilView.hpp"

#ifdef LUNA_RHI_D3D12

namespace Luna
{
	namespace RHI
	{
		static R<DepthStencilViewDesc> get_default_dsv(Resource* res)
		{
			ResourceDesc d = res->m_desc;
			if (d.pixel_format != Format::d16_unorm &&
				d.pixel_format != Format::d24_unorm_s8_uint &&
				d.pixel_format != Format::d32_float &&
				d.pixel_format != Format::d32_float_s8_uint_x24)
			{
				return BasicError::bad_arguments();
			}
			switch (d.type)
			{
			case ResourceType::buffer:
			case ResourceType::texture_3d:
				return BasicError::bad_arguments();
			case ResourceType::texture_1d:
				return (d.depth_or_array_size) == 1 ?
					DepthStencilViewDesc::as_tex1d(d.pixel_format, 0) :
					DepthStencilViewDesc::as_tex1darray(d.pixel_format, 0, 0, d.depth_or_array_size);
			case ResourceType::texture_2d:
				return (d.depth_or_array_size == 1) ?
					((d.sample_count == 1) ?
						DepthStencilViewDesc::as_tex2d(d.pixel_format, 0) :
						DepthStencilViewDesc::as_tex2dms(d.pixel_format)) :
					((d.sample_count == 1) ?
						DepthStencilViewDesc::as_tex2darray(d.pixel_format, 0, 0, d.depth_or_array_size) :
						DepthStencilViewDesc::as_tex2dmsarray(d.pixel_format, 0, d.depth_or_array_size)
						);
			default:
				lupanic();
				break;
			}
			return BasicError::failure();
		}

		RV DepthStencilView::init(IResource* resource, const DepthStencilViewDesc* desc)
		{
			lucheck(resource);
			DepthStencilViewDesc d;
			lutry
			{
				Resource * reso = static_cast<Resource*>(resource->get_object());
				if (desc) d = *desc;
				else
				{
					luset(d, get_default_dsv(reso));
				}
				m_desc = d;
				luset(m_heap, m_device->m_dsv_heap.allocate_view());
				m_resource = resource;
				{
					ID3D12Resource* res = reso->m_res.Get();
					D3D12_DEPTH_STENCIL_VIEW_DESC dsv;
					dsv.Format = encode_pixel_format(m_desc.format);
					dsv.Flags = D3D12_DSV_FLAG_NONE;
					if (m_desc.depth_read_only)
					{
						dsv.Flags = dsv.Flags | D3D12_DSV_FLAG_READ_ONLY_DEPTH;
					}
					if (m_desc.stencil_read_only)
					{
						dsv.Flags = dsv.Flags | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
					}
					switch (m_desc.type)
					{
					case DepthStencilViewType::tex1d:
						dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
						dsv.Texture1D.MipSlice = m_desc.tex1d.mip_slice;
						break;
					case DepthStencilViewType::tex1darray:
						dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
						dsv.Texture1DArray.ArraySize = m_desc.tex1darray.array_size;
						dsv.Texture1DArray.FirstArraySlice = m_desc.tex1darray.first_array_slice;
						dsv.Texture1DArray.MipSlice = m_desc.tex1darray.mip_slice;
						break;
					case DepthStencilViewType::tex2d:
						dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
						dsv.Texture2D.MipSlice = m_desc.tex2d.mip_slice;
						break;
					case DepthStencilViewType::tex2darray:
						dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
						dsv.Texture2DArray.ArraySize = m_desc.tex2darray.array_size;
						dsv.Texture2DArray.FirstArraySlice = m_desc.tex2darray.first_array_slice;
						dsv.Texture2DArray.MipSlice = m_desc.tex2darray.mip_slice;
						break;
					case DepthStencilViewType::tex2dms:
						dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
						break;
					case DepthStencilViewType::tex2dmsarray:
						dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
						dsv.Texture2DMSArray.ArraySize = m_desc.tex2dmsarray.array_size;
						dsv.Texture2DMSArray.FirstArraySlice = m_desc.tex2dmsarray.first_array_slice;
					default:
						lupanic();
					}
					m_device->m_device->CreateDepthStencilView(res, &dsv, m_heap->GetCPUDescriptorHandleForHeapStart());
				}
			}
			lucatchret;
			return ok;
		}	
	}
}

#endif