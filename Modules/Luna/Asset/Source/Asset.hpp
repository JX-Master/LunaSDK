/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Asset.hpp
* @author JXMaster
* @date 2022/5/11
*/
#pragma once
#include "../Asset.hpp"
#include <Luna/Runtime/SpinLock.hpp>
#include <Luna/Runtime/Mutex.hpp>
#include <Luna/Runtime/Signal.hpp>

namespace Luna
{
	namespace Asset
	{
		struct AssetMetaFile
		{
			lustruct("Asset::AssetMetaFile", "{93C04F6C-BC6C-4586-8CB2-7DF1B249DA21}");
			Guid guid;
			Name type;
		};

		// Maps to `asset_t`
		struct AssetEntry
		{
			Guid guid;
			Name type;
			Path path;
			ObjRef data;
			JobSystem::job_id_t last_load_job;
			Error last_load_result;
			SpinLock lock;
			AssetEntry() :
				last_load_job(JobSystem::INVALID_JOB_ID) {}
			void reset()
			{
				type.reset();
				path.clear();
				data.reset();
				last_load_job = JobSystem::INVALID_JOB_ID;
				last_load_result.reset();
			}
		};
		void init_asset_registry();
		void close_asset_registry();
	}
}