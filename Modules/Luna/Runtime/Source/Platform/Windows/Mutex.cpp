/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mutex.cpp
* @author JXMaster
* @date 2022/3/2
*/
#include "../../OS.hpp"
#include "../../../Platform/Windows/MiniWin.hpp"

namespace Luna
{
	namespace OS
	{
		opaque_t new_mutex()
		{
			CRITICAL_SECTION* ret = (CRITICAL_SECTION*)Luna::memalloc(sizeof(CRITICAL_SECTION), alignof(CRITICAL_SECTION));
			InitializeCriticalSection(ret);
			return ret;
		}

		void delete_mutex(opaque_t mutex)
		{
			DeleteCriticalSection((CRITICAL_SECTION*)mutex);
			Luna::memfree(mutex, alignof(CRITICAL_SECTION));
		}

		void lock_mutex(opaque_t mutex)
		{
			EnterCriticalSection((CRITICAL_SECTION*)mutex);
		}

		bool try_lock_mutex(opaque_t mutex)
		{
			return TryEnterCriticalSection((CRITICAL_SECTION*)mutex);
		}

		void unlock_mutex(opaque_t mutex)
		{
			LeaveCriticalSection((CRITICAL_SECTION*)mutex);
		}
	}
}