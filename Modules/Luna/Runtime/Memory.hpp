/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Memory.hpp
* @author JXMaster
* @date 2020/7/22
* @brief Runtime memory management APIs.
*/
#pragma once
#include "Base.hpp"
#include "Hash.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeMemory Memory allocation and deallocation
	//! @}

	//! @addtogroup RuntimeMemory
	//! @{

	//! Allocates heap memory.
	//! @param[in] size The size, in bytes, of the memory block to allocate. If this is `0`, no memory will be allocated.
	//! @param[in] alignment Optional. The alignment requirement, in bytes, of the memory block to allocate. Default is `0`.
	//! 
	//! If this is 0 (default), then the memory is allocated with no additional alignment requirement. In such case, the memory address is 
	//! aligned to 8 bytes in 32-bit platform, and 16 bytes in 64-bit platform.
	//! 
	//! @return Returns one pointer to the allocated memory block.
	//! Returns `nullptr` if memory allocation failed or if `size` is `0`.
	//! @par Valid Usage
	//! * If `alignment` is not `0`, `alignment` **must** be powers of 2 (like 32, 64, 128, 256, etc).
	LUNA_RUNTIME_API void* memalloc(usize size, usize alignment = 0);

	//! Frees heap memory.
	//! @param[in] ptr The pointer returned by @ref memalloc or @ref memrealloc. 
	//! If this is `nullptr`, this function does nothing.
	//! @param[in] alignment Optional. The alignment requirement specified when allocating the memory block. Default is 0.
	//! @par Valid Usage
	//! * If `ptr` is not `nullptr`, `alignment` **must** be equal to `alignment` passed to @ref memalloc or @ref memrealloc which allocates `ptr`.
	LUNA_RUNTIME_API void memfree(void* ptr, usize alignment = 0);

	//! Reallocates heap memory.
	//! @param[in] ptr The pointer to the former allocated memory block.
	//! If this is `nullptr`, this method behaves the same as @ref memalloc.
	//! @param[in] size The size, in bytes, of the new memory to allocate.
	//! If this is `0` and `ptr` is not `nullptr`, this function behaves the same as @ref memfree.
	//! @param[in] alignment Optional. The alignment requirement of the allocated memory block. Default is 0.
	//! @ref memrealloc cannot change the alignment requirement of the memory block. In other words, if `ptr` is not `nullptr`, the alignment requirements of the old and
	//! new memory block must be the same.
	//! @return Returns one pointer to the reallocated memory block.
	//! Returns `nullptr` if the allocation is failed. In such case, the old memory block (if have) is not changed.
	//! @details This function allocates a new memory block with the specified size and alignment
	//! requirement, copies the data from the old memory block to the new one, and frees the old memory block.
	//! @par Valid Usage
	//! * If `ptr` is not `nullptr`, `ptr` **must** be allocated by a prior call to @ref memalloc or @ref memrealloc.
	//! * If `ptr` is not `nullptr`, `alignment` **must** be equal to `alignment` passed to @ref memalloc or @ref memrealloc which allocates `ptr`.
	LUNA_RUNTIME_API void* memrealloc(void* ptr, usize size, usize alignment = 0);

	//! Gets the allocated size of one memory block.
	//! @param[in] ptr The pointer to the memory block.
	//! @param[in] alignment Optional. The alignment requirement of the allocated memory block. Default is 0. 
	//! @return Returns the size of bytes of the memory block.
	//! Returns `0` if `ptr` is `nullptr`.
	//! @details The returned size is the size allocated for the memory block and is available for the user to use. 
	//! The allocated size may be larger than the required size passed to @ref memalloc or @ref memrealloc to satisfy alignment and padding requirements.
	//! @par Valid Usage
	//! * If `ptr` is not `nullptr`, `ptr` **must** be allocated by a prior call to @ref memalloc or @ref memrealloc.
	//! * If `ptr` is not `nullptr`, `alignment` **must** be equal to `alignment` passed to @ref memalloc or @ref memrealloc which allocates `ptr`.
	LUNA_RUNTIME_API usize memsize(void* ptr, usize alignment = 0);

	//! Allocates heap memory for one object and constructs the object.
	//! @return Returns one pointer to the allocated object.
	//! Returns `nullptr` if memory allocation failed.
	template <typename _Ty, typename... _Args>
	_Ty* memnew(_Args&&... args)
	{
		_Ty* o = reinterpret_cast<_Ty*>(Luna::memalloc(sizeof(_Ty), alignof(_Ty)));
		if (o)
		{
			new (o) _Ty(forward<_Args>(args)...);
			return o;
		}
		return nullptr;
	}

	//! Destructs one object and frees its memory.
	//! @param[in] o The pointer to the object to be deleted.
	//! @par Valid Usage
	//! * `o` must point to a object created by `memnew`.
	template <typename _Ty>
	void memdelete(_Ty* o)
	{
		o->~_Ty();
		Luna::memfree(o, alignof(_Ty));
	}

	//! @}
}
