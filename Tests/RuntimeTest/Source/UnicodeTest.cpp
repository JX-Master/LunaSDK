/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file UnicodeTest.cpp
* @author JXMaster
* @date 2023/10/11
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/Unicode.hpp>

namespace Luna
{
    void unicode_test()
    {
        // UTF-8
        {
            c8 ch_utf8[] = {(c8)0xE4, (c8)0xB8, (c8)0xAD};
            usize ch_len = utf8_charlen(ch_utf8);
            lucheck(ch_len == 3);
            usize num_bytes = 0;
            R<c32> ch_result = utf8_decode_char(ch_utf8, sizeof(ch_utf8), &num_bytes);
            lucheck(succeeded(ch_result));
            c32 ch = ch_result.get();
            lucheck(ch == 0x4E2D && num_bytes == 3);
            ch_len = utf8_charspan(ch);
            lucheck(ch_len == 3);
            c8 ch_utf8_out[3] = {};
            R<usize> encode_result = utf8_encode_char(ch_utf8_out, sizeof(ch_utf8_out), ch);
            lucheck(succeeded(encode_result) && encode_result.get() == 3);
            lucheck((u8)ch_utf8_out[0] == 0xE4);
            lucheck((u8)ch_utf8_out[1] == 0xB8);
            lucheck((u8)ch_utf8_out[2] == 0xAD);
        }
        // UTF-8 validation.
        {
            lucheck(utf8_charspan(0x0000) == 1);
            lucheck(utf8_charspan(0x007F) == 1);
            lucheck(utf8_charspan(0x0080) == 2);
            lucheck(utf8_charspan(0x07FF) == 2);
            lucheck(utf8_charspan(0x0800) == 3);
            lucheck(utf8_charspan(0xD7FF) == 3);
            lucheck(utf8_charspan(0xD800) == 0);
            lucheck(utf8_charspan(0xDFFF) == 0);
            lucheck(utf8_charspan(0xE000) == 3);
            lucheck(utf8_charspan(0xFFFF) == 3);
            lucheck(utf8_charspan(0x10000) == 4);
            lucheck(utf8_charspan(0x10FFFF) == 4);
            lucheck(utf8_charspan(0x110000) == 0);

            lucheck(utf8_charlen((c8)0x00) == 1);
            lucheck(utf8_charlen((c8)0x7F) == 1);
            lucheck(utf8_charlen((c8)0xC2) == 2);
            lucheck(utf8_charlen((c8)0xDF) == 2);
            lucheck(utf8_charlen((c8)0xE0) == 3);
            lucheck(utf8_charlen((c8)0xEF) == 3);
            lucheck(utf8_charlen((c8)0xF0) == 4);
            lucheck(utf8_charlen((c8)0xF4) == 4);
            lucheck(utf8_charlen((c8)0x80) == 0);
            lucheck(utf8_charlen((c8)0xBF) == 0);
            lucheck(utf8_charlen((c8)0xC0) == 0);
            lucheck(utf8_charlen((c8)0xC1) == 0);
            lucheck(utf8_charlen((c8)0xF5) == 0);
            lucheck(utf8_charlen((c8)0xFF) == 0);
            c8 invalid_string[] = {(c8)0x80, 0};
            lucheck(utf8_strlen(invalid_string) == 0);
            lucheck(utf8_index(invalid_string, 1) == 0);

            c32 roundtrip_codepoints[] = {
                0x0000, 0x007F, 0x0080, 0x07FF, 0x0800,
                0xD7FF, 0xE000, 0xFFFF, 0x10000, 0x10FFFF
            };
            for(c32 codepoint : roundtrip_codepoints)
            {
                c8 encoded[4];
                R<usize> encoded_size = utf8_encode_char(encoded, sizeof(encoded), codepoint);
                lucheck(succeeded(encoded_size));
                lucheck(encoded_size.get() == utf8_charspan(codepoint));
                usize decoded_size = 0;
                R<c32> decoded = utf8_decode_char(encoded, encoded_size.get(), &decoded_size);
                lucheck(succeeded(decoded));
                lucheck(decoded.get() == codepoint && decoded_size == encoded_size.get());
            }

            c8 unchanged_buffer[] = {1, 2, 3, 4};
            R<usize> invalid_encode = utf8_encode_char(
                unchanged_buffer, sizeof(unchanged_buffer), 0xD800);
            lucheck(failed(invalid_encode));
            lucheck(invalid_encode.errcode() == BasicError::bad_data());
            lucheck(unchanged_buffer[0] == 1 && unchanged_buffer[1] == 2 &&
                unchanged_buffer[2] == 3 && unchanged_buffer[3] == 4);
            invalid_encode = utf8_encode_char(unchanged_buffer, sizeof(unchanged_buffer), 0x110000);
            lucheck(failed(invalid_encode));
            lucheck(invalid_encode.errcode() == BasicError::bad_data());
            invalid_encode = utf8_encode_char(unchanged_buffer, 3, 0x10000);
            lucheck(failed(invalid_encode));
            lucheck(invalid_encode.errcode() == BasicError::insufficient_user_buffer());
            lucheck(unchanged_buffer[0] == 1 && unchanged_buffer[1] == 2 &&
                unchanged_buffer[2] == 3 && unchanged_buffer[3] == 4);

            c8 null_character[] = {0};
            usize num_bytes = USIZE_MAX;
            R<c32> result = utf8_decode_char(null_character, sizeof(null_character), &num_bytes);
            lucheck(succeeded(result));
            lucheck(result.get() == 0 && num_bytes == 1);

            c8 maximum_codepoint[] = {(c8)0xF4, (c8)0x8F, (c8)0xBF, (c8)0xBF};
            result = utf8_decode_char(maximum_codepoint, sizeof(maximum_codepoint), &num_bytes);
            lucheck(succeeded(result));
            lucheck(result.get() == 0x10FFFF && num_bytes == 4);

            c8 invalid_continuation[] = {(c8)0xE2, (c8)0x28, (c8)0xA1};
            num_bytes = USIZE_MAX;
            result = utf8_decode_char(invalid_continuation, sizeof(invalid_continuation), &num_bytes);
            lucheck(failed(result));
            lucheck(result.errcode() == BasicError::bad_data() && num_bytes == 0);

            c8 truncated_sequence[] = {(c8)0xF0, (c8)0x9F, (c8)0x92};
            result = utf8_decode_char(truncated_sequence, sizeof(truncated_sequence));
            lucheck(failed(result));
            lucheck(result.errcode() == BasicError::end_of_file());

            c8 invalid_truncated_sequence[] = {(c8)0xE2, (c8)0x28};
            result = utf8_decode_char(invalid_truncated_sequence, sizeof(invalid_truncated_sequence));
            lucheck(failed(result));
            lucheck(result.errcode() == BasicError::bad_data());

            c8 overlong_sequence[] = {(c8)0xE0, (c8)0x80, (c8)0xAF};
            result = utf8_decode_char(overlong_sequence, sizeof(overlong_sequence));
            lucheck(failed(result));
            lucheck(result.errcode() == BasicError::bad_data());

            c8 invalid_two_byte_lead[] = {(c8)0xC0, (c8)0x80};
            result = utf8_decode_char(invalid_two_byte_lead, sizeof(invalid_two_byte_lead));
            lucheck(failed(result));
            lucheck(result.errcode() == BasicError::bad_data());

            c8 encoded_surrogate[] = {(c8)0xED, (c8)0xA0, (c8)0x80};
            result = utf8_decode_char(encoded_surrogate, sizeof(encoded_surrogate));
            lucheck(failed(result));
            lucheck(result.errcode() == BasicError::bad_data());

            c8 out_of_range[] = {(c8)0xF4, (c8)0x90, (c8)0x80, (c8)0x80};
            result = utf8_decode_char(out_of_range, sizeof(out_of_range));
            lucheck(failed(result));
            lucheck(result.errcode() == BasicError::bad_data());

            c8 invalid_four_byte_lead[] = {(c8)0xF5, (c8)0x80, (c8)0x80, (c8)0x80};
            result = utf8_decode_char(invalid_four_byte_lead, sizeof(invalid_four_byte_lead));
            lucheck(failed(result));
            lucheck(result.errcode() == BasicError::bad_data());

            c8 stray_continuation[] = {(c8)0x80};
            result = utf8_decode_char(stray_continuation, sizeof(stray_continuation));
            lucheck(failed(result));
            lucheck(result.errcode() == BasicError::bad_data());
        }
        // UTF-16: system-default endian.
        {
            c16 ch_utf16[] = { 0x4E2D };
            usize ch_len = utf16_charlen(ch_utf16);
            lucheck(ch_len == 1);
            c32 ch = utf16_decode_char(ch_utf16);
            lucheck(ch == 0x4E2D);
            ch_len = utf16_charspan(ch);
            lucheck(ch_len == 1);
            c16 ch_utf16_out[1];
            utf16_encode_char(ch_utf16_out, ch);
            lucheck(ch_utf16_out[0] == 0x4E2D);
            // 4-bytes character.
            c16 ch_utf16_2[] = {0xD802, 0xDE6F};
            ch_len = utf16_charlen(ch_utf16_2);
            lucheck(ch_len == 2);
            ch = utf16_decode_char(ch_utf16_2);
            lucheck(ch == 0x10A6F);
            ch_len = utf16_charspan(ch);
            lucheck(ch_len == 2);
            c16 ch_utf16_2_out[2];
            utf16_encode_char(ch_utf16_2_out, ch);
            lucheck(ch_utf16_2_out[0] == 0xD802);
            lucheck(ch_utf16_2_out[1] == 0xDE6F);
        }
    }
}
