/*!
* This file is a portion of Luna SDK.
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
            c8 ch_utf8[] = {0xE4, 0xB8, 0xAD};
            usize ch_len = utf8_charlen(ch_utf8);
            lucheck(ch_len == 3);
            c32 ch = utf8_decode_char(ch_utf8);
            lucheck(ch == 0x4E2D);
            ch_len = utf8_charspan(ch);
            lucheck(ch_len == 3);
            c8 ch_utf8_out[3] = {};
            utf8_encode_char(ch_utf8_out, ch);
            lucheck(ch_utf8_out[0] == 0xE4);
            lucheck(ch_utf8_out[1] == 0xB8);
            lucheck(ch_utf8_out[2] == 0xAD);
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
        // UTF-16: LE
        {
            
        }
    }
}