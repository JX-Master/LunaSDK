/*
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

		LUNA_RHI_API ErrCode device_lost()
		{
			static ErrCode e = get_error_code_by_name("RHIError", "device_lost");
			return e;
		}
	}
}