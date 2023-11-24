/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Semaphore.cpp
* @author JXMaster
* @date 2022/3/10
*/
#include "../../OS.hpp"
#include "../../../Platform/Windows/MiniWin.hpp"

namespace Luna
{
	namespace OS
	{
		opaque_t new_semaphore(i32 initial_count, i32 max_count)
		{
			HANDLE ret = ::CreateSemaphoreW(NULL, initial_count, max_count, NULL);
			if (!ret)
			{
				lupanic_msg_always("CreateSemaphoreW failed.");
			}
			return ret;
		}
		void delete_semaphore(opaque_t sema)
		{
			::CloseHandle(sema);
		}
		void acquire_semaphore(opaque_t signal)
		{
			if (::WaitForSingleObject(signal, INFINITE) != WAIT_OBJECT_0)
			{
				lupanic_always();
			}
		}
		bool try_acquire_semaphore(opaque_t signal)
		{
			if (::WaitForSingleObject(signal, 0) == WAIT_OBJECT_0)
			{
				return true;
			}
			return false;
		}
		void release_semaphore(opaque_t signal)
		{
			::ReleaseSemaphore(signal, 1, NULL);
		}
	}
}