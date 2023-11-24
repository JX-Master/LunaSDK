/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Memory.cpp
* @author JXMaster
* @date 2020/9/22
 */
#include "../../OS.hpp"
#include <sys/mman.h>
#include "Errno.hpp"
#include "../../../Error.hpp"
#include <pthread.h>
#include <stdlib.h>
#include "../../../Algorithm.hpp"

#ifdef LUNA_PLATFORM_MACOS
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

namespace Luna
{
    namespace OS
    {
        void* memalloc(usize size, usize alignment /* = 0 */)
        {
            if (!size) return nullptr;
            if (alignment <= MAX_ALIGN) return malloc(size);
            usize ptr = (usize)malloc(size + alignment);
            usize aligned_ptr = align_upper(ptr + 1, alignment);
            isize offset = aligned_ptr - ptr;
            *((isize*)(aligned_ptr)-1) = offset;
            return (void*)aligned_ptr;
        }
        void memfree(void* ptr, usize alignment /* = 0 */)
        {
            if (!ptr) return;
            if (alignment <= MAX_ALIGN)
            {
                free(ptr);
            }
            else
            {
                isize offset = *(((isize*)ptr) - 1);
                void* origin_ptr = (void*)(((usize)ptr) - offset);
                free(origin_ptr);
            }
        }
        usize memsize(void* ptr, usize alignment /* = 0 */)
        {
            if (!ptr) return 0;
			if (alignment <= MAX_ALIGN)
			{
#ifdef LUNA_PLATFORM_MACOS
				return malloc_size(ptr);
#else
                return malloc_usable_size(ptr);
#endif
			}
			isize offset = *(((isize*)ptr) - 1);
			void* origin_ptr = (void*)(((usize)ptr) - offset);
#ifdef LUNA_PLATFORM_MACOS
			return malloc_size(origin_ptr) - offset;
#else
            return malloc_usable_size(origin_ptr) - offset;
#endif
        }
    }
}