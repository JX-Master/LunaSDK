/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StdIO.cpp
* @author JXMaster
* @date 2023/2/28
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../StdIO.hpp"
#include "Platform/StdIO.hpp"
#include "ErrorImpl.hpp"

namespace Luna
{
    LUNA_RUNTIME_API RV read_standard_input(void* buffer, usize size, usize* read_bytes)
    {
        return encode_platform_result(Platform::read_standard_input(buffer, size, read_bytes));
    }

    LUNA_RUNTIME_API RV write_standard_output(const void* buffer, usize size, usize* write_bytes)
    {
        return encode_platform_result(Platform::write_standard_output(buffer, size, write_bytes));
    }

    LUNA_RUNTIME_API RV write_standard_error(const void* buffer, usize size, usize* write_bytes)
    {
        return encode_platform_result(Platform::write_standard_error(buffer, size, write_bytes));
    }
}
