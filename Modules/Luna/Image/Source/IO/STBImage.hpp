/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file STBImage.hpp
* @author JXMaster
* @date 2020/3/6
*/
#pragma once
#include <Luna/Runtime/Assert.hpp>
#define STBI_ASSERT(x) luassert(x)
#include <Luna/Runtime/Memory.hpp>
#include "../Image.hpp"

namespace Luna
{
	namespace Image
	{

		inline void* stbi_malloc(usize sz)
		{
			return memalloc(sz);
		}

		inline void stbi_free(void* p)
		{
			memfree(p);
		}

		inline void* stbi_realloc(void* p, usize newsz)
		{
			return memrealloc(p, newsz);
		}
	}
}

#define STBI_MALLOC(sz)			Luna::Image::stbi_malloc(sz)
#define STBI_REALLOC(p,newsz)	Luna::Image::stbi_realloc(p,newsz)
#define STBI_FREE(p)			Luna::Image::stbi_free(p)

#ifdef LUNA_PLATFORM_WINDOWS
#define STBI_WINDOWS_UTF8
#endif

#define STBI_NO_STDIO

#include <stb_image.h>

namespace Luna
{
	namespace Image
	{
		extern stbi_io_callbacks stbi_iocb;

		// Initializes the STB context.
		void stbi_init();
	}
}