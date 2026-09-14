/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Guid.hpp
* @author JXMaster
* @date 2026/8/25
*/
#pragma once
#include "Result.hpp"

namespace Luna
{
    //! The number of characters in the canonical textual representation of a GUID.
    inline constexpr usize GUID_STRING_LENGTH = 36;

    //! Encodes one GUID into its canonical lowercase textual representation.
    //! @param[in] guid The GUID to encode.
    //! @param[out] out_buffer The buffer that receives exactly 36 characters. No null terminator is appended.
    //! @param[in] buf_size The number of writable characters in @p out_buffer.
    //! @return Returns @ref ok on success, @ref E_BAD_ARGUMENTS if @p out_buffer is `nullptr`, or
    //! @ref E_INSUFFICIENT_USER_BUFFER if @p buf_size is smaller than 36.
    inline constexpr RV encode_guid(const Guid& guid, c8* out_buffer, usize buf_size)
    {
        if(!out_buffer) return E_BAD_ARGUMENTS;
        if(buf_size < GUID_STRING_LENGTH) return E_INSUFFICIENT_USER_BUFFER;
        usize output_index = 0;
        for(usize digit_index = 0; digit_index < 32; ++digit_index)
        {
            if(digit_index == 8 || digit_index == 12 || digit_index == 16 || digit_index == 20)
            {
                out_buffer[output_index++] = '-';
            }
            u8 digit;
            if(digit_index < 16)
            {
                digit = (u8)((guid.high >> ((15 - digit_index) * 4)) & 0x0f);
            }
            else
            {
                digit = (u8)((guid.low >> ((31 - digit_index) * 4)) & 0x0f);
            }
            out_buffer[output_index++] = digit < 10 ? (c8)('0' + digit) : (c8)('a' + digit - 10);
        }
        return ok;
    }

    //! Decodes one GUID from its canonical textual representation.
    //! @param[in] buffer The character buffer to decode. The buffer does not need to be null-terminated.
    //! @param[in] buf_size The number of readable characters in @p buffer. Both the 36-character canonical form and
    //! the same form enclosed in braces are accepted.
    //! @param[out] out_guid The decoded GUID. This value is not modified if decoding fails.
    //! @return Returns @ref ok on success, @ref E_BAD_ARGUMENTS if @p buffer is `nullptr`, or @ref E_BAD_DATA if
    //! the buffer is not a canonical GUID representation.
    inline constexpr RV decode_guid(const c8* buffer, usize buf_size, Guid& out_guid)
    {
        if(!buffer) return E_BAD_ARGUMENTS;
        bool braced = buf_size == GUID_STRING_LENGTH + 2 && buffer[0] == '{' &&
            buffer[GUID_STRING_LENGTH + 1] == '}';
        if(buf_size != GUID_STRING_LENGTH && !braced) return E_BAD_DATA;
        const c8* value = braced ? buffer + 1 : buffer;
        for(usize i = 0; i < GUID_STRING_LENGTH; ++i)
        {
            if(i == 8 || i == 13 || i == 18 || i == 23)
            {
                if(value[i] != '-') return E_BAD_DATA;
            }
            else if(!((value[i] >= '0' && value[i] <= '9') ||
                (value[i] >= 'a' && value[i] <= 'f') ||
                (value[i] >= 'A' && value[i] <= 'F')))
            {
                return E_BAD_DATA;
            }
        }
        out_guid = Guid(value);
        return ok;
    }
}
