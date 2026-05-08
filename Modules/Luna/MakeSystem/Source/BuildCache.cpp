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
#include <Luna/Runtime/Variant.hpp>
#include <Luna/VariantUtils/JSON.hpp>

namespace Luna
{
    namespace MakeSystem
    {
        static RV ensure_dir(const Path& dir)
        {
            if(dir.empty())
            {
                return ok;
            }
            String dir_str = dir.encode();
            auto attr = get_file_attribute(dir_str.c_str());
            if(succeeded(attr))
            {
                if(test_flags(attr.get().attributes, FileAttributeFlag::directory))
                {
                    return ok;
                }
                return set_error(BasicError::bad_arguments(), "Path exists but is not a directory: %s", dir_str.c_str());
            }
            if(attr.errcode() != BasicError::not_found())
            {
                return attr.errcode();
            }
            Path parent = dir;
            parent.pop_back();
            if(parent != dir)
            {
                auto r = ensure_dir(parent);
                if(failed(r)) return r;
            }
            auto r = create_dir(dir_str.c_str());
            if(failed(r) && r.errcode() != BasicError::already_exists())
            {
                return r;
            }
            return ok;
        }

        static Variant paths_to_variant(const Vector<Path>& paths)
        {
            Variant ret(VariantType::array);
            for(const auto& path : paths)
            {
                ret.push_back(Variant(path.encode().c_str()));
            }
            return ret;
        }

        static Vector<Path> paths_from_variant(const Variant& data)
        {
            Vector<Path> ret;
            if(data.type() != VariantType::array)
            {
                return ret;
            }
            for(const auto& item : data.values())
            {
                if(item.type() == VariantType::string)
                {
                    ret.push_back(Path(item.c_str()));
                }
            }
            return ret;
        }

        RV BuildCache::init(const Path& cache_directory)
        {
            lutry
            {
                luexp(ensure_dir(cache_directory));
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
            auto iter = m_records.find(path);
            if(iter == m_records.end()) return 0;
            return iter->second.timestamp;
        }
        R<BuildCacheRecord> BuildCache::get_record(const Path& path)
        {
            LockGuard guard(m_lock);
            auto iter = m_records.find(path);
            if(iter == m_records.end()) return BasicError::not_found();
            return iter->second;
        }
        void BuildCache::set_record(const Path& path, const BuildCacheRecord& record)
        {
            LockGuard guard(m_lock);
            m_records.insert_or_assign(path, record);
        }
        void BuildCache::set_timestamp(const Path& path, i64 timestamp)
        {
            LockGuard guard(m_lock);
            auto iter = m_records.find(path);
            if(iter == m_records.end())
            {
                BuildCacheRecord record;
                record.timestamp = timestamp;
                m_records.insert_or_assign(path, record);
            }
            else
            {
                iter->second.timestamp = timestamp;
            }
        }
        RV BuildCache::save_to_file()
        {
            LockGuard guard(m_lock);
            lutry
            {
                Variant data(VariantType::object);
                data[Name("version")] = (u64)1;
                Variant records(VariantType::object);
                for(auto& item : m_records)
                {
                    Variant record(VariantType::object);
                    record[Name("command")] = item.second.command.c_str();
                    record[Name("action_key")] = item.second.action_key;
                    record[Name("timestamp")] = item.second.timestamp;
                    record[Name("outputs")] = paths_to_variant(item.second.outputs);
                    record[Name("implicit_dependencies")] = paths_to_variant(item.second.implicit_dependencies);
                    records[Name(item.first.encode())] = move(record);
                }
                data[Name("records")] = move(records);
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
                auto file = open_file(m_cache_file_path.encode().c_str(), FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing);
                if(!file.valid())
                {
                    return ok;
                }
                Ref<IFile> f = move(file.get());
                lulet(data, VariantUtils::read_json(f));
                m_records.clear();
                const Variant& records = data[Name("records")];
                if(records.type() != VariantType::object)
                {
                    return ok;
                }
                for(const auto& item : records.key_values())
                {
                    BuildCacheRecord record;
                    const Variant& record_data = item.second;
                    if(record_data[Name("command")].type() == VariantType::string)
                    {
                        record.command = record_data[Name("command")].c_str();
                    }
                    record.action_key = record_data[Name("action_key")].unum();
                    record.timestamp = record_data[Name("timestamp")].inum();
                    record.outputs = paths_from_variant(record_data[Name("outputs")]);
                    record.implicit_dependencies = paths_from_variant(record_data[Name("implicit_dependencies")]);
                    m_records.insert_or_assign(Path(item.first.c_str()), move(record));
                }
            }
            lucatchret;
            return ok;
        }
    }
}
