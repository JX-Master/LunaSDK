/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Variant.cpp
* @author JXMaster
* @date 2021/12/6
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../Variant.hpp"

namespace Luna
{
	LUNA_RUNTIME_API const Variant& Variant::npos()
	{
		static Variant v(VariantType::null);
		return v;
	}
}