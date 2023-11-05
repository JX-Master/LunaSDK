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
#include <Luna/Runtime/Module.hpp>
#include "../DescriptorSet.hpp"
namespace Luna
{
	namespace RHI
	{
#ifdef LUNA_MEMORY_PROFILER_ENABLED
		Name g_memory_domain_gpu;
		Name g_memory_type_buffer;
		Name g_memory_type_texture;
		Name g_memory_type_aliasing_memory;
#endif
		RV init()
		{
			lutry
			{
#ifdef LUNA_MEMORY_PROFILER_ENABLED
				g_memory_domain_gpu = "GPU";
				g_memory_type_buffer = "Buffer";
				g_memory_type_texture = "Texture";
				g_memory_type_aliasing_memory = "Aliasing Memory";
#endif
				luexp(render_api_init());
			}
			lucatchret;
			return ok;
		}

		void close()
		{
			render_api_close();
#ifdef LUNA_MEMORY_PROFILER_ENABLED
			g_memory_domain_gpu.reset();
			g_memory_type_buffer.reset();
			g_memory_type_texture.reset();
			g_memory_type_aliasing_memory.reset();
#endif
		}

		LUNA_STATIC_REGISTER_MODULE(RHI, "Window", init, close);
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
		LUNA_RHI_API ErrCode swap_chain_out_of_date()
		{
			static ErrCode e = get_error_code_by_name("RHIError", "swap_chain_out_of_date");
			return e;
		}
	}
}