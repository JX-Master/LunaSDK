/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JSON.hpp
* @author JXMaster
* @date 2021/4/19
*/
#pragma once
#include <Luna/Runtime/Variant.hpp>
#include <Luna/Runtime/Stream.hpp>

#ifndef LUNA_VARIANT_UTILS_API
#define LUNA_VARIANT_UTILS_API
#endif

namespace Luna
{
	namespace VariantUtils
	{
		//! Reads one JSON string.
		LUNA_VARIANT_UTILS_API R<Variant> read_json(const c8* src, usize src_size = USIZE_MAX);

		LUNA_VARIANT_UTILS_API R<Variant> read_json(IStream* stream);

		LUNA_VARIANT_UTILS_API String write_json(const Variant& v, bool indent = true);
		
		LUNA_VARIANT_UTILS_API RV write_json(IStream* stream, const Variant& v, bool indent = true);
	}
}