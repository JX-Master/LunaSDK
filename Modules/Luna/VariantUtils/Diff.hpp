/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Diff.hpp
* @author JXMaster
* @date 2022/6/26
*/
#pragma once
#include <Luna/Runtime/Variant.hpp>

#ifndef LUNA_VARIANT_UTILS_API
#define LUNA_VARIANT_UTILS_API
#endif

namespace Luna
{
	namespace VariantUtils
	{
		// Creates one variant that records differences between two variant values.
		LUNA_VARIANT_UTILS_API Variant diff(const Variant& before, const Variant& after);

		//! Applys the difference to the variant, so that it contains the same data as `after` when the diff object
		//! is created.
		LUNA_VARIANT_UTILS_API void patch(Variant& before, const Variant& delta);

		//! Reversed the difference made in `after`, so that it contains the same data as `before` when the diff
		//! object is created.
		LUNA_VARIANT_UTILS_API void reverse(Variant& after, const Variant& delta);

		//! Adds prefix to the diff object.
		LUNA_VARIANT_UTILS_API void add_diff_prefix(Variant& delta, const Vector<Variant>& prefix_nodes);
	}
}