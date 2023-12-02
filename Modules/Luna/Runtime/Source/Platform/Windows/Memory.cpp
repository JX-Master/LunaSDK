/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Memory.cpp
* @author JXMaster
* @date 2020/7/28
*/
#include <Luna/Runtime/Assert.hpp>
#include "../../../Platform/Windows/MiniWin.hpp"
#include "../../OS.hpp"
#include <Luna/Runtime/Result.hpp>

namespace Luna
{
	namespace OS
	{
		void* memalloc(usize size, usize alignment /* = 0 */)
		{
			void* ret = (alignment > MAX_ALIGN) ? _aligned_malloc(size, alignment) : malloc(size);
			if (!ret)
			{
				lupanic_msg_always("System memory allocation failed.");
			}
			return ret;
		}
		void memfree(void* ptr, usize alignment /* = 0 */)
		{
			if (alignment > MAX_ALIGN)
			{
				_aligned_free(ptr);
			}
			else
			{
				free(ptr);
			}
		}
		usize memsize(void* ptr, usize alignment /* = 0 */)
		{
			if (!ptr) return 0;
			return (alignment > MAX_ALIGN) ? _aligned_msize(ptr, alignment, 0) : _msize(ptr);
		}
	}
}