/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file OS.cpp
* @author JXMaster
* @date 2021/4/25
 */
#include "../../OS.hpp"
#include <sys/types.h>
#ifdef LUNA_PLATFORM_MACOS
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif

namespace Luna
{
    namespace OS
    {
		void time_init();
        void file_init();
        void std_io_init();
        void std_io_close();

		void init()
		{
			time_init();
            file_init();
            std_io_init();
		}

		void close()
        {
            std_io_close();
        }

		u32 get_num_processors()
		{
#ifdef LUNA_PLATFORM_MACOS
			size_t size;
			int name[2];
			size = 4;
			name[0] = CTL_HW;
			name[1] = HW_NCPU;
			int processor_count;
			if (sysctl(name, 2, &processor_count, &size, nullptr, 0) != 0)
			{
				processor_count = 1;
			}
			return (u32)processor_count;
#else
			int processor_count = max<int>(sysconf(_SC_NPROCESSORS_ONLN), 1);
			return (u32)processor_count;
#endif
		}
    }
}