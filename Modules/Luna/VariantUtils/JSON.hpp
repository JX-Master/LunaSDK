/*!
* This file is a portion of LunaSDK.
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

        //! Configures JSON parsing behavior.
        struct JSONReadOptions
        {
            //! Allows C and C++ style comments between JSON tokens.
            bool allow_comments = true;
            //! Allows one trailing comma before the closing token of an object or array.
            bool allow_trailing_commas = true;
            //! Allows the non-standard `\0` and `\'` string escapes.
            bool allow_nonstandard_escapes = true;
            //! Allows Unicode whitespace characters in addition to JSON whitespace characters.
            bool allow_nonstandard_whitespace = true;
            //! Allows content after the first complete root value to be ignored.
            bool allow_trailing_content = true;
            //! Decodes strings using the LunaSDK `@base64@` or `@base85@` format as BLOB variants.
            bool decode_blobs = true;
            //! Allows UTF-16 input with a byte-order mark. JSON without a UTF-16 byte-order mark
            //! is always interpreted as UTF-8.
            bool allow_utf16 = true;
            //! Allows numbers outside the finite `f64` range to be represented as infinity,
            //! and accepts the non-standard `nan`, `inf` and `-inf` tokens.
            bool allow_non_finite_numbers = true;

            //! Returns options that accept only the JSON data model and syntax used by RFC 8259.
            static JSONReadOptions strict()
            {
                JSONReadOptions options;
                options.allow_comments = false;
                options.allow_trailing_commas = false;
                options.allow_nonstandard_escapes = false;
                options.allow_nonstandard_whitespace = false;
                options.allow_trailing_content = false;
                options.decode_blobs = false;
                options.allow_utf16 = false;
                options.allow_non_finite_numbers = false;
                return options;
            }
        };

        //! Configures JSON writing behavior.
        struct JSONWriteOptions
        {
            //! Adds indentation and line breaks to the generated JSON text.
            bool indent = true;
            //! Encodes BLOB variants as strings using the LunaSDK `@base64@` or `@base85@` format.
            //! If disabled, writing a BLOB variant fails with @ref BasicError::not_supported.
            bool encode_blobs = true;
            //! Writes non-finite floating-point values using the non-standard `nan`, `inf` and `-inf` tokens.
            //! If disabled, writing a non-finite number fails with @ref BasicError::not_supported.
            bool allow_non_finite_numbers = true;

            //! Returns options suitable for a compact RFC 8259 JSON text.
            static JSONWriteOptions strict()
            {
                JSONWriteOptions options;
                options.indent = false;
                options.encode_blobs = false;
                options.allow_non_finite_numbers = false;
                return options;
            }
        };

        //! Parses one JSON string.
        //! @param[in] src The JSON string to read.
        //! @param[in] src_size The exact number of bytes available in `src`. Embedded null bytes are treated as
        //! invalid input. Specify @ref USIZE_MAX to determine the size with `strlen(src)`.
        //! @return Returns one variant that contains the data read from the JSON string.
        LUNA_VARIANT_UTILS_API R<Variant> read_json(const c8* src, usize src_size = USIZE_MAX);

        //! Parses one JSON string with the specified options.
        //! @param[in] src The JSON string to read.
        //! @param[in] src_size The exact number of bytes available in `src`. Embedded null bytes are treated as
        //! invalid input. Specify @ref USIZE_MAX to determine the size with `strlen(src)`.
        //! @param[in] options Parsing options.
        //! @return Returns one variant that contains the data read from the JSON string.
        LUNA_VARIANT_UTILS_API R<Variant> read_json(
            const c8* src,
            usize src_size,
            const JSONReadOptions& options);

        //! Parses one null-terminated JSON string with the specified options.
        //! @param[in] src The JSON string to read.
        //! @param[in] options Parsing options.
        //! @return Returns one variant that contains the data read from the JSON string.
        LUNA_VARIANT_UTILS_API R<Variant> read_json(const c8* src, const JSONReadOptions& options);

        //! Parses one JSON string.
        //! @param[in] stream The stream that contains the JSON string to read. @ref IStream::read will be called to read JSON string
        //! from the stream.
        //! @return Returns one variant that contains the data read from the JSON string.
        LUNA_VARIANT_UTILS_API R<Variant> read_json(IStream* stream);

        //! Parses one JSON string from a stream with the specified options.
        //! @param[in] stream The stream that contains the JSON string to read.
        //! @param[in] options Parsing options.
        //! @return Returns one variant that contains the data read from the JSON string.
        LUNA_VARIANT_UTILS_API R<Variant> read_json(IStream* stream, const JSONReadOptions& options);

        //! Writes one variant object to JSON string.
        //! @param[in] v The variant object that contains data to write.
        //! @param[in] indent Whether to add indents and line breaks to the generated JSON string, so that improves readability but
        //! also increases the string size.
        //! @return Returns the generated JSON string.
        LUNA_VARIANT_UTILS_API String write_json(const Variant& v, bool indent = true);

        //! Writes one variant object to a JSON string with the specified options.
        //! @param[in] v The variant object that contains data to write.
        //! @param[in] options Writing options.
        //! @return Returns the generated JSON string, or an error if `v` cannot be represented
        //! under the specified options.
        LUNA_VARIANT_UTILS_API R<String> write_json(const Variant& v, const JSONWriteOptions& options);
        
        //! Writes one variant object to JSON string.
        //! @param[in] stream The stream to write JSON string to.
        //! @param[in] v The variant object that contains data to write.
        //! @param[in] indent Whether to add indents and line breaks to the generated JSON string, so that improves readability but
        //! also increases the string size.
        LUNA_VARIANT_UTILS_API RV write_json(IStream* stream, const Variant& v, bool indent = true);

        //! Writes one variant object to a stream with the specified options.
        //! @param[in] stream The stream to write JSON string to.
        //! @param[in] v The variant object that contains data to write.
        //! @param[in] options Writing options.
        LUNA_VARIANT_UTILS_API RV write_json(
            IStream* stream,
            const Variant& v,
            const JSONWriteOptions& options);

        //! @}
    }
}
