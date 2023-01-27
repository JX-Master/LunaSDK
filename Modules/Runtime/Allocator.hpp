/*
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
	//! @class Allocator
	class Allocator
	{
	public:
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
		template <typename _Ty>
		void deallocate(_Ty* ptr, usize n = 1)
		{
			memfree(ptr, alignof(_Ty));
		}
		void* allocate_bytes(usize sz, usize alignment)
		{
#ifdef LUNA_PROFILE
			void* r = memalloc(sz, alignment);
			luassert_msg_always(r, "Bad memory allocation");
			return r;
#else
			return memalloc(sz, alignment);
#endif
			
		}
		void deallocate_bytes(void* ptr, usize sz, usize alignment)
		{
			memfree(ptr, alignment);
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