/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GuidTest.cpp
* @author JXMaster
* @date 2026/8/25
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/Guid.hpp>

namespace Luna
{
    namespace
    {
        constexpr const c8* TEST_GUID_STRING = "123e4567-e89b-12d3-a456-426614174000";
        constexpr Guid TEST_GUID(TEST_GUID_STRING);

        constexpr bool guid_constexpr_test()
        {
            c8 buffer[GUID_STRING_LENGTH]{};
            if(!encode_guid(TEST_GUID, buffer, sizeof(buffer)).valid()) return false;
            for(usize i = 0; i < GUID_STRING_LENGTH; ++i)
            {
                if(buffer[i] != TEST_GUID_STRING[i]) return false;
            }
            Guid decoded;
            if(!decode_guid(buffer, sizeof(buffer), decoded).valid()) return false;
            return decoded == TEST_GUID;
        }

        static_assert(guid_constexpr_test());
    }

    void guid_test()
    {
        c8 buffer[GUID_STRING_LENGTH + 1];
        for(usize i = 0; i < sizeof(buffer); ++i) buffer[i] = '?';
        luassert_always(succeeded(encode_guid(TEST_GUID, buffer, sizeof(buffer))));
        luassert_always(buffer[GUID_STRING_LENGTH] == '?');
        for(usize i = 0; i < GUID_STRING_LENGTH; ++i)
        {
            luassert_always(buffer[i] == TEST_GUID_STRING[i]);
        }

        luassert_always(encode_guid(TEST_GUID, nullptr, GUID_STRING_LENGTH).errcode() == E_BAD_ARGUMENTS);
        luassert_always(encode_guid(TEST_GUID, buffer, GUID_STRING_LENGTH - 1).errcode() ==
            E_INSUFFICIENT_USER_BUFFER);

        Guid decoded;
        luassert_always(succeeded(decode_guid(buffer, GUID_STRING_LENGTH, decoded)));
        luassert_always(decoded == TEST_GUID);
        constexpr const c8* braced = "{123E4567-E89B-12D3-A456-426614174000}";
        luassert_always(succeeded(decode_guid(braced, GUID_STRING_LENGTH + 2, decoded)));
        luassert_always(decoded == TEST_GUID);

        Guid unchanged(1, 2);
        c8 invalid[GUID_STRING_LENGTH];
        for(usize i = 0; i < GUID_STRING_LENGTH; ++i) invalid[i] = TEST_GUID_STRING[i];
        invalid[0] = 'x';
        luassert_always(decode_guid(invalid, sizeof(invalid), unchanged).errcode() == E_BAD_DATA);
        luassert_always(unchanged == Guid(1, 2));
        luassert_always(decode_guid(TEST_GUID_STRING, GUID_STRING_LENGTH - 1, unchanged).errcode() == E_BAD_DATA);
        luassert_always(decode_guid(nullptr, GUID_STRING_LENGTH, unchanged).errcode() == E_BAD_ARGUMENTS);
    }
}
