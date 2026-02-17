/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file BuildCache.hpp
* @author JXMaster
* @date 2026/2/9
*/
#pragma once
#include <Luna/Runtime/TypeInfo.hpp>
#include <Luna/Runtime/Path.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/SpinLock.hpp>

namespace Luna
{
    namespace MakeSystem
    {
        struct BuildCache
        {
            lustruct("MakeSystem::BuildCache", "{4eca9a8a-db94-4733-a154-9bb8c33c051a}");

            Path m_cache_file_path;
            HashMap<Path, i64> m_timestamps;
            SpinLock m_lock;

            RV init(const Path& cache_directory);
            i64 get_timestamp(const Path& path);
            void set_timestamp(const Path& path, i64 timestamp);
            RV save_to_file();
            RV load_from_file();
        };
    }
}