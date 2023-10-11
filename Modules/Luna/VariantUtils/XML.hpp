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
		//! @brief Reads one XML string.
		LUNA_VARIANT_UTILS_API R<Variant> read_xml(const c8* src, usize src_size = USIZE_MAX);

		LUNA_VARIANT_UTILS_API R<Variant> read_xml(IStream* stream);

		LUNA_VARIANT_UTILS_API String write_xml(const Variant& v, bool indent = true);
		
		LUNA_VARIANT_UTILS_API RV write_xml(IStream* stream, const Variant& v, bool indent = true);

        LUNA_VARIANT_UTILS_API Variant new_xml_document(const c8* version = "1.0", const c8* encoding = "UTF-8");


        LUNA_VARIANT_UTILS_API const Name& get_xml_version(const Variant& xml_document);
        LUNA_VARIANT_UTILS_API void set_xml_version(Variant& xml_document);

        LUNA_VARIANT_UTILS_API Name& get_xml_name(Variant& xml_node);
	}
}