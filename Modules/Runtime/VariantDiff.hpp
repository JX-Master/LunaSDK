/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VariantDiff.hpp
* @author JXMaster
* @date 2022/6/26
*/
#pragma once
#include "Variant.hpp"
#include "Path.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

namespace Luna
{

	// Creates one variant that records differences between two variant values.
	LUNA_RUNTIME_API Variant diff_variant(const Variant& before, const Variant& after);

	//! Applys the difference to the variant, so that it contains the same data as `after` when the diff object
	//! is created.
	LUNA_RUNTIME_API void patch_variant_diff(Variant& before, const Variant& diff);

	//! Reversed the difference made in `after`, so that it contains the same data as `before` when the diff
	//! object is created.
	LUNA_RUNTIME_API void reverse_variant_diff(Variant& after, const Variant& diff);

	//! Adds prefix to the diff object.
	LUNA_RUNTIME_API void variant_diff_prefix(Variant& diff, const Vector<Variant>& prefix_nodes);
}