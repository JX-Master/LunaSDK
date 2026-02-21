/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file BuildCache.cpp
* @author JXMaster
* @date 2026/2/9
*/
#include "BuildCache.hpp"
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include <Luna/VariantUtils/JSON.hpp>

namespace Luna
{
    namespace MakeSystem
    {
        RV BuildCache::init(const Path& cache_directory)
        {
            lutry
            {
                String dir = cache_directory.encode();
                auto r = get_file_attribute(dir.c_str());
                if(failed(r))
                {
                    luexp(create_dir(dir.c_str()));
                }
                m_cache_file_path = cache_directory;
                m_cache_file_path.push_back("build_cache.json");
                luexp(load_from_file());
            }
            lucatchret;
            return ok;
        }
        i64 BuildCache::get_timestamp(const Path& path)
        {
            LockGuard guard(m_lock);
            auto iter = m_timestamps.find(path);
            if(iter == m_timestamps.end()) return 0;
            return iter->second;
        }
        void BuildCache::set_timestamp(const Path& path, i64 timestamp)
        {
            LockGuard guard(m_lock);
            m_timestamps.insert_or_assign(path, timestamp);
        }
        RV BuildCache::save_to_file()
        {
            LockGuard guard(m_lock);
            lutry
            {
                lulet(data, serialize(m_timestamps));
                String data_str = VariantUtils::write_json(data);
                lulet(f, open_file(m_cache_file_path.encode().c_str(), FileOpenFlag::write, FileCreationMode::create_always));
                luexp(f->write(data_str.data(), data_str.size() * sizeof(c8)));
            }
            lucatchret;
            return ok;
        }
        RV BuildCache::load_from_file()
        {
            LockGuard guard(m_lock);
            lutry
            {
                lulet(f, open_file(m_cache_file_path.encode().c_str(), FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing));
                lulet(data, VariantUtils::read_json(f));
                luexp(deserialize(m_cache_file_path, data));
            }
            lucatchret;
            return ok;
        }
    }
}