/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StdIO.cpp
* @author JXMaster
* @date 2023/2/28
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "StdIO.hpp"

namespace Luna
{
    Ref<StdIOStream> g_std_io_stream;
    void std_io_init()
    {
        g_std_io_stream = new_object<StdIOStream>();
    }
    void std_io_close()
    {
        g_std_io_stream.reset();
    }
    LUNA_RUNTIME_API IStream* get_std_io_stream()
    {
        return g_std_io_stream.get();
    }
}