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
        //! @addtogroup VariantUtils
        //! @{

        //! Parses one JSON string.
        //! @param[in] src The JSON string to read.
        //! @param[in] src_size The maximum number of characters to read in `src`.
        //! The actual read chacaters may be small than this if the JSON string ends early.
        //! 
        //! If this value is greater than `strlen(src)`, `strlen(src)` will be used as the maximum number of characters to read
        //! instead of this value. So specifing @ref USIZE_MAX will let the system detects the string length automatically.
        //! @return Returns one variant that contains the data read from the JSON string.
        LUNA_VARIANT_UTILS_API R<Variant> read_json(const c8* src, usize src_size = USIZE_MAX);

        //! Parses one JSON string.
        //! @param[in] stream The stream that contains the JSON string to read. @ref IStream::read will be called to read JSON string
        //! from the stream.
        //! @return Returns one variant that contains the data read from the JSON string.
        LUNA_VARIANT_UTILS_API R<Variant> read_json(IStream* stream);

        //! Writes one variant object to JSON string.
        //! @param[in] v The variant object that contains data to write.
        //! @param[in] indent Whether to add indents and line breaks to the generated JSON string, so that improves readability but
        //! also increases the string size.
        //! @return Returns the generated JSON string.
        LUNA_VARIANT_UTILS_API String write_json(const Variant& v, bool indent = true);
        
        //! Writes one variant object to JSON string.
        //! @param[in] stream The stream to write JSON string to.
        //! @param[in] v The variant object that contains data to write.
        //! @param[in] indent Whether to add indents and line breaks to the generated JSON string, so that improves readability but
        //! also increases the string size.
        LUNA_VARIANT_UTILS_API RV write_json(IStream* stream, const Variant& v, bool indent = true);

        //! @}
    }
}