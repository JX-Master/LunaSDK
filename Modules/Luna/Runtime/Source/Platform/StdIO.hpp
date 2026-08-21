/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StdIO.hpp
* @author JXMaster
* @date 2026/8/14
*/
#pragma once
#include "Result.hpp"

namespace Luna
{
    namespace Platform
    {
        Result read_standard_input(void* buffer, usize size, usize* read_bytes);
        Result write_standard_output(const void* buffer, usize size, usize* write_bytes);
        Result write_standard_error(const void* buffer, usize size, usize* write_bytes);
    }
}
