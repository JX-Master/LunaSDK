/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file OS.cpp
* @author JXMaster
* @date 2020/12/9
*/
#include "../../OS.hpp"
#include "../../../Platform/Windows/MiniWin.hpp"

namespace Luna
{
	namespace OS
	{
		void time_init();
		void thread_init();
		void thread_close();
		void file_init();
		void std_io_init();
		void std_io_close();
		void debug_init();
		void debug_close();

		void init()
		{
			debug_init();
			time_init();
			thread_init();
			file_init();
			std_io_init();
		}

		void close() 
		{
			std_io_close();
			thread_close();
			debug_close();
		}

		u32 get_num_processors()
		{
			SYSTEM_INFO si;
			memzero(&si, sizeof(SYSTEM_INFO));
			::GetSystemInfo(&si);
			return si.dwNumberOfProcessors;
		}
	}
}