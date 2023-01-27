/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Assert.cpp
* @author JXMaster
* @date 2020/9/22
 */
#include <Runtime/PlatformDefines.hpp>

#ifdef LUNA_PLATFORM_POSIX

#include "../../OS.hpp"
#include <Runtime/Unicode.hpp>
#include <assert.h>

namespace Luna
{
    namespace OS
    {
        void assert_fail(const c8* msg, const c8* file, u32 line)
        {
            printf ("%s:%d: failed assertion `%s'\n", file, line, msg);
            abort();
        }

        void debug_break()
        {
#ifdef LUNA_DEBUG
			__debugbreak();
#endif
        }

    }
}

#endif