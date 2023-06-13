/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VFS.hpp
* @author JXMaster
* @date 2022/5/24
*/
#pragma once
#include "../VFS.hpp"
#include "../Driver.hpp"

namespace Luna
{
	namespace VFS
	{
		// Mount path.
		struct MountPair
		{
			Path m_mount_path;
			DriverDesc* m_driver;
			void* m_mount_data;
		};
	}
}