/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Allocator.hpp
* @author JXMaster
* @date 2020/6/8
*/
#pragma once
#include "Assert.hpp"
#include "Memory.hpp"

namespace Luna
{
	//! @ingroup Runtime
	//! The default allocator implementation that can be used for allocating memory for containers defined in Runtime module.
	//! @details The default allocator allocates memory by calling @ref memalloc, and deallocates memory by calling @ref memfree.
	class Allocator
	{
	public:
		//! Allocates memory for the specified number of elements.
		//! @param[in] n The number of elements to allocate memory for. 
		//! @return Returns the allocated memory. The size of the allocated memory is at least `sizeof(_Ty) * n` bytes, and the 
		//! alignment of the allocated memory is at least `alignof(_Ty)` bytes. The returned memory is uninitialized.
		//! If the allocation fails, returns `nullptr`.
		template <typename _Ty>
		_Ty* allocate(usize n = 1)
		{
#ifdef LUNA_PROFILE
			void* r = memalloc(sizeof(_Ty) * n, alignof(_Ty));
			luassert_msg_always(r, "Bad memory allocation");
			return (_Ty*)r;
#else
			return (_Ty*)memalloc(sizeof(_Ty) * n, alignof(_Ty));
#endif
			
		}
		//! Deallocates memory allocated from @ref allocate
		//! @param[in] ptr The memory pointer returned by @ref allocate.
		//! @param[in] n The number of elements earler passed to @ref allocate.
		template <typename _Ty>
		void deallocate(_Ty* ptr, usize n = 1)
		{
			memfree(ptr, alignof(_Ty));
		}
		bool operator==(const Allocator&)
		{
			return true;
		}
		bool operator!=(const Allocator&)
		{
			return false;
		}
	};
}