/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RHI.cpp
* @author JXMaster
* @date 2022/4/14
*/
#include "RHI.hpp"
#include <Runtime/Module.hpp>
#include "../DescriptorSet.hpp"
namespace Luna
{
	namespace RHI
	{
		RV init()
		{
			lutry
			{
				luexp(render_api_init());
			}
			lucatchret;
			return ok;
		}

		void close()
		{
			render_api_close();
		}

		StaticRegisterModule m("RHI", "Window", init, close);
	}

	namespace RHIError
	{
		LUNA_RHI_API errcat_t errtype()
		{
			static errcat_t e = get_error_category_by_name("RHIError");
			return e;
		}

		LUNA_RHI_API ErrCode device_hung()
		{
			static ErrCode e = get_error_code_by_name("RHIError", "device_hung");
			return e;
		}
		LUNA_RHI_API ErrCode device_reset()
		{
			static ErrCode e = get_error_code_by_name("RHIError", "device_reset");
			return e;
		}
		LUNA_RHI_API ErrCode device_removed()
		{
			static ErrCode e = get_error_code_by_name("RHIError", "device_removed");
			return e;
		}
		LUNA_RHI_API ErrCode driver_internal_error()
		{
			static ErrCode e = get_error_code_by_name("RHIError", "driver_internal_error");
			return e;
		}
		LUNA_RHI_API ErrCode frame_statistics_disjoint()
		{
			static ErrCode e = get_error_code_by_name("RHIError", "frame_statistics_disjoint");
			return e;
		}
	}

	namespace RHI
	{
		LUNA_RHI_API ShaderResourceViewDesc get_default_srv_from_resource(IResource* resource)
		{
			ResourceDesc d = resource->get_desc();
			switch (d.type)
			{
			case ResourceType::texture_1d:
				return (d.depth_or_array_size) == 1 ?
					ShaderResourceViewDesc::as_tex1d(d.pixel_format, 0, d.mip_levels, 0.0f) :
					ShaderResourceViewDesc::as_tex1darray(d.pixel_format, 0, d.mip_levels, 0, d.depth_or_array_size, 0.0f);
			case ResourceType::texture_2d:
				return (d.depth_or_array_size == 1) ?
					((d.sample_count == 1) ?
						ShaderResourceViewDesc::as_tex2d(d.pixel_format, 0, d.mip_levels, 0.0f) :
						ShaderResourceViewDesc::as_tex2dms(d.pixel_format)) :
					((d.sample_count == 1) ?
						ShaderResourceViewDesc::as_tex2darray(d.pixel_format, 0, d.mip_levels, 0, d.depth_or_array_size, 0.0f) :
						ShaderResourceViewDesc::as_tex2dmsarray(d.pixel_format, 0, d.depth_or_array_size)
						);
			case ResourceType::texture_3d:
				return ShaderResourceViewDesc::as_tex3d(d.pixel_format, 0, d.mip_levels, 0.0f);
			case ResourceType::buffer:
				return ShaderResourceViewDesc::as_buffer(Format::unknown, 0, (u32)d.width_or_buffer_size, 1, false);
			default:
				break;
			}
			lupanic();
			return ShaderResourceViewDesc();
		}
		LUNA_RHI_API UnorderedAccessViewDesc get_default_uav_from_resource(IResource* resource)
		{
			ResourceDesc d = resource->get_desc();
			switch (d.type)
			{
			case ResourceType::buffer:
				return UnorderedAccessViewDesc::as_buffer(Format::unknown, 0, (u32)d.width_or_buffer_size, 1, 0, false);
			case ResourceType::texture_1d:
				return (d.depth_or_array_size) == 1 ?
					UnorderedAccessViewDesc::as_tex1d(d.pixel_format, 0) :
					UnorderedAccessViewDesc::as_tex1darray(d.pixel_format, 0, 0, d.depth_or_array_size);
			case ResourceType::texture_2d:
				return (d.depth_or_array_size == 1) ?
					UnorderedAccessViewDesc::as_tex2d(d.pixel_format, 0) :
					UnorderedAccessViewDesc::as_tex2darray(d.pixel_format, 0, 0, d.depth_or_array_size);
			case ResourceType::texture_3d:
				return UnorderedAccessViewDesc::as_tex3d(d.pixel_format, 0, 0, d.depth_or_array_size);
			default:
				break;
			}
			lupanic();
			return UnorderedAccessViewDesc();
		}
	}
}