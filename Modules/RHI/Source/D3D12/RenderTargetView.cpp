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
namespace Luna
{
	namespace RHI
	{
		static R<RenderTargetViewDesc> get_default_rtv(TextureResource* res)
		{
			TextureDesc d = res->m_desc;
			if (d.pixel_format == Format::unknown)
			{
				return BasicError::bad_arguments();
			}
			switch (d.type)
			{
			case TextureType::tex1d:
				return (d.array_size) == 1 ?
					RenderTargetViewDesc::tex1d(d.pixel_format, 0) :
					RenderTargetViewDesc::tex1darray(d.pixel_format, 0, 0, d.array_size);
			case TextureType::tex2d:
				return (d.array_size == 1) ?
					((d.sample_count == 1) ?
						RenderTargetViewDesc::tex2d(d.pixel_format, 0) :
						RenderTargetViewDesc::tex2dms(d.pixel_format)) :
					((d.sample_count == 1) ?
						RenderTargetViewDesc::tex2darray(d.pixel_format, 0, 0, d.array_size) :
						RenderTargetViewDesc::tex2dmsarray(d.pixel_format, 0, d.array_size)
						);
			case TextureType::tex3d:
				return RenderTargetViewDesc::tex3d(d.pixel_format, 0, 0, d.depth);
			default:
				lupanic();
				break;
			}
			return BasicError::failure();
		}
		RV RenderTargetView::init(ITexture* texture, const RenderTargetViewDesc* desc)
		{
			lucheck(texture);
			RenderTargetViewDesc d;
			lutry
			{
				TextureResource* reso = cast_object<TextureResource>(texture->get_object());
				if (desc) d = *desc;
				else
				{
					luset(d, get_default_rtv(reso));
				}
				m_desc = d;
				luset(m_heap, m_device->m_rtv_heap.allocate_view());
				m_texture = texture;
				{
					ID3D12Resource* res = reso->m_res.Get();
					D3D12_RENDER_TARGET_VIEW_DESC rtv;
					switch (m_desc.type)
					{
					case RenderTargetViewType::tex1d:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
						rtv.Texture1D.MipSlice = m_desc.mip_slice;
						break;
					case RenderTargetViewType::tex1darray:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
						rtv.Texture1DArray.ArraySize = m_desc.depth_or_array_size;
						rtv.Texture1DArray.FirstArraySlice = m_desc.first_depth_or_array_slice;
						rtv.Texture1DArray.MipSlice = m_desc.mip_slice;
						break;
					case RenderTargetViewType::tex2d:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
						rtv.Texture2D.MipSlice = m_desc.mip_slice;
						rtv.Texture2D.PlaneSlice = 0;
						break;
					case RenderTargetViewType::tex2darray:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
						rtv.Texture2D.MipSlice = m_desc.mip_slice;
						rtv.Texture2D.PlaneSlice = 0;
						break;
					case RenderTargetViewType::tex2dms:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
						break;
					case RenderTargetViewType::tex2dmsarray:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
						rtv.Texture2DMSArray.ArraySize = m_desc.depth_or_array_size;
						rtv.Texture2DMSArray.FirstArraySlice = m_desc.first_depth_or_array_slice;
						break;
					case RenderTargetViewType::tex3d:
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
						rtv.Texture3D.FirstWSlice = m_desc.first_depth_or_array_slice;
						rtv.Texture3D.MipSlice = m_desc.mip_slice;
						rtv.Texture3D.WSize = m_desc.depth_or_array_size;
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