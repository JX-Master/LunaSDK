/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file XML.hpp
* @author JXMaster
* @date 2023/10/11
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
        LUNA_VARIANT_UTILS_API Variant new_xml_element(const Name& name);
		LUNA_VARIANT_UTILS_API Name get_xml_name(const Variant& xml_element);
		LUNA_VARIANT_UTILS_API void set_xml_name(Variant& xml_element, const Name& name);
		LUNA_VARIANT_UTILS_API const Variant& get_xml_attributes(const Variant& xml_element);
		LUNA_VARIANT_UTILS_API Variant& get_xml_attributes(Variant& xml_element);
		LUNA_VARIANT_UTILS_API const Variant& get_xml_content(const Variant& xml_element);
		LUNA_VARIANT_UTILS_API Variant& get_xml_content(Variant& xml_element);
		LUNA_VARIANT_UTILS_API const Variant& find_first_xml_child_element(const Variant& xml_element, const Name& name, usize start_index = 0, usize* out_index = nullptr);

		//! Reads one XML string.
		LUNA_VARIANT_UTILS_API R<Variant> read_xml(const void* src, usize src_size = USIZE_MAX);
		LUNA_VARIANT_UTILS_API R<Variant> read_xml(IStream* stream);
		LUNA_VARIANT_UTILS_API String write_xml(const Variant& v, bool indent = true);
		LUNA_VARIANT_UTILS_API RV write_xml(IStream* stream, const Variant& v, bool indent = true);
	}
}