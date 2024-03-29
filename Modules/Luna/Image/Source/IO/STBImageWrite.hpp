/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file STBImageWrite.hpp
* @author JXMaster
* @date 2020/3/7
*/
#pragma once
#include <Luna/Runtime/Assert.hpp>
#define STBIW_ASSERT(x) luassert(x)
#include <Luna/Runtime/Memory.hpp>

namespace Luna
{
    namespace Image
    {
        inline void* stbiw_malloc(usize sz)
        {
            return memalloc(sz);
        }

        inline void stbiw_free(void* p)
        {
            memfree(p);
        }

        inline void* stbiw_realloc(void* p, usize newsz)
        {
            return memrealloc(p, newsz);
        }
    }
}

#define STBIW_MALLOC(sz)            Luna::Image::stbiw_malloc(sz)
#define STBIW_REALLOC(p,newsz)        Luna::Image::stbiw_realloc(p,newsz)
#define STBIW_FREE(p)                Luna::Image::stbiw_free(p)

#include <stb_image_write.h>

namespace Luna
{
    namespace Image
    {
        void stbi_write_func(void* context, void* data, int size);

    }
}