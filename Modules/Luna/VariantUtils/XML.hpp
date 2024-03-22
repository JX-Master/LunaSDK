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
        //! @addtogroup VariantUtils
        //! @{
        
        //! Creates one variant that represents one XML element.
        //! @param[in] name The name of the variant.
        //! @return Returns the created variant.
        LUNA_VARIANT_UTILS_API Variant new_xml_element(const Name& name);
        //! Gets the name of one XML element.
        //! @param[in] xml_element The variant that represents the XML element.
        //! @return Returns the name of the XML element.
        LUNA_VARIANT_UTILS_API Name get_xml_name(const Variant& xml_element);
        //! Sets the name of one XML element.
        //! @param[in] xml_element The variant that represents the XML element.
        //! @param[in] name The name to set.
        LUNA_VARIANT_UTILS_API void set_xml_name(Variant& xml_element, const Name& name);
        //! Gets attributes of one XML element.
        //! @param[in] xml_element The variant that represents the XML element.
        //! @return Returns one reference to the attribute map of the XML element. 
        //! The returned variant has object variant type, and attributes can be indexed using attribute names as map keys.
        LUNA_VARIANT_UTILS_API const Variant& get_xml_attributes(const Variant& xml_element);
        //! Gets attributes of one XML element.
        //! @param[in] xml_element The variant that represents the XML element.
        //! @return Returns one reference to the attribute map of the XML element.
        //! The returned variant has object variant type, and attributes can be indexed and changed using attribute names as map keys.
        LUNA_VARIANT_UTILS_API Variant& get_xml_attributes(Variant& xml_element);
        //! Gets content of one XML element.
        //! @param[in] xml_element The variant that represents the XML element.
        //! @return Returns one reference to the content array of the XML element. Every element in the content array is a variant that
        //! represents one child XML element.
        LUNA_VARIANT_UTILS_API const Variant& get_xml_content(const Variant& xml_element);
        //! Gets content of one XML element.
        //! @param[in] xml_element The variant that represents the XML element.
        //! @return Returns one reference to the content array of the XML element. Every element in the content array is a variant that
        //! represents one child XML element.
        LUNA_VARIANT_UTILS_API Variant& get_xml_content(Variant& xml_element);
        //! Finds the first XML child element in the specified XML element with the specified name.
        //! @param[in] xml_element The variant that represents the parent XML element.
        //! @param[in] name The name of the XML child element to search for.
        //! @param[in] start_index The index to start searching for the specified element.
        //! @param[out] out_index If not `nullptr`, returns the index of the found element. If the element is not found, returns @ref USIZE_MAX.
        //! @return Returns one reference to the found child element. Returns @ref Variant::npos if not found.
        LUNA_VARIANT_UTILS_API const Variant& find_first_xml_child_element(const Variant& xml_element, const Name& name, usize start_index = 0, usize* out_index = nullptr);

        //! Parses one XML string.
        //! @param[in] src The XML string to read.
        //! @param[in] src_size The maximum number of characters to read in `src`.
        //! The actual read chacaters may be small than this if the XML string ends early.
        //! 
        //! If this value is greater than `strlen(src)`, `strlen(src)` will be used as the maximum number of characters to read
        //! instead of this value. So specifing @ref USIZE_MAX will let the system detects the string length automatically.
        //! @return Returns one variant that contains the data read from the XML string.
        LUNA_VARIANT_UTILS_API R<Variant> read_xml(const void* src, usize src_size = USIZE_MAX);
        //! Parses one XML string.
        //! @param[in] stream The stream that contains the XML string to read. @ref IStream::read will be called to read XML string
        //! from the stream.
        //! @return Returns one variant that contains the data read from the XML string.
        LUNA_VARIANT_UTILS_API R<Variant> read_xml(IStream* stream);
        //! Writes one variant object to XML string.
        //! @param[in] v The variant object that represents the root XML element to write.
        //! @param[in] indent Whether to add indents and line breaks to the generated XML string, so that improves readability but
        //! also increases the string size.
        //! @return Returns the generated XML string.
        LUNA_VARIANT_UTILS_API String write_xml(const Variant& v, bool indent = true);
        //! Writes one variant object to XML string.
        //! @param[in] stream The stream to write XML string to.
        //! @param[in] v The variant object that represents the root XML element to write.
        //! @param[in] indent Whether to add indents and line breaks to the generated XML string, so that improves readability but
        //! also increases the string size.
        LUNA_VARIANT_UTILS_API RV write_xml(IStream* stream, const Variant& v, bool indent = true);
    
        //! @}
    }
}