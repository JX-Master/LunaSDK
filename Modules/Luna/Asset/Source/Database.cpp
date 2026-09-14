/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Database.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ASSET_API LUNA_EXPORT
#include "DatabaseImpl.hpp"
#include <Luna/Runtime/Algorithm.hpp>
#include <Luna/Runtime/Guid.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/VFS/VFS.hpp>

namespace Luna::Asset
{
    bool database_contains(const Path& root, const Path& path)
    {
        return root.root() == path.root() && root.flags() == path.flags() && path.is_subpath_of(root);
    }
    RV validate_database_path(const Path& path, bool relative, bool allow_empty)
    {
        if(!allow_empty && path.empty()) return E_BAD_ARGUMENTS;
        if(relative && (!path.root().empty() || path.flags() != PathFlag::none)) return E_BAD_ARGUMENTS;
        if((u32)path.flags() & ~(u32)PathFlag::absolute) return E_BAD_ARGUMENTS;
        for(const auto& node : path)
        {
            const c8* s = node.c_str();
            if(node.empty() || strlen(s) != node.size() || !strcmp(s, ".") || !strcmp(s, "..") ||
                strchr(s, '/') || strchr(s, '\\') || strchr(s, ':')) return E_BAD_ARGUMENTS;
        }
        return ok;
    }
    Path database_relative_path(const Path& root, const Path& path)
    {
        Path result;
        result.assign_relative(root, path);
        result.root().reset();
        return result;
    }
    Path database_absolute_path(const Path& root, const Path& path)
    {
        Path result(root);
        result.append(path);
        return result;
    }
    RV validate_metadata(const AssetMetadata& record)
    {
        if(failed(validate_database_path(record.path, true)) || record.guid == Guid(0, 0) || record.type.empty())
            return set_error(E_BAD_DATA, "Invalid asset metadata GUID, path or type.");
        HashSet<Name> ids;
        for(const auto& unit : record.data_units)
        {
            if(unit.id.empty() || unit.loader.empty() || ids.contains(unit.id))
                return set_error(E_BAD_DATA, "Invalid or duplicate named data unit.");
            ids.insert(unit.id);
        }
        return ok;
    }
    RV validate_records(Span<const AssetMetadata> records)
    {
        HashSet<Guid> guids;
        HashSet<Path> paths;
        for(const auto& record : records)
        {
            auto valid = validate_metadata(record);
            if(failed(valid)) return valid;
            if(guids.contains(record.guid) || paths.contains(record.path))
                return set_error(E_ALREADY_EXISTS, "Duplicate database GUID or asset path %s.", record.path.encode().c_str());
            guids.insert(record.guid);
            paths.insert(record.path);
        }
        return ok;
    }
    AssetMetadata metadata_from_file(const AssetMetaFile& file, const Path& path)
    {
        AssetMetadata record;
        record.guid = file.guid;
        record.path = path;
        record.type = file.type;
        record.data_units = file.data_units;
        return record;
    }
    AssetMetaFile metadata_to_file(const AssetMetadata& record)
    {
        AssetMetaFile file;
        file.format_version = 2;
        file.guid = record.guid;
        file.type = record.type;
        file.data_units = record.data_units;
        sort(file.data_units.begin(), file.data_units.end(), [](const auto& a, const auto& b)
        { return strcmp(a.id.c_str(), b.id.c_str()) < 0; });
        return file;
    }
    static Path sidecar_path(const Path& path)
    {
        Path result(path);
        result.append_extension("meta");
        return result;
    }
    static bool is_unsigned_integer(const Variant& value)
    {
        return value.number_type() == VariantNumberType::number_u64 ||
            (value.number_type() == VariantNumberType::number_i64 && value.inum() >= 0);
    }
    static bool is_metadata_name(const Variant& value)
    {
        return value.type() == VariantType::string && !value.str().empty() &&
            strlen(value.str().c_str()) == value.str().size();
    }
    static RV validate_metadata_json(const Variant& data)
    {
        if(data.type() != VariantType::object || data["guid"].type() != VariantType::array || data["guid"].size() != 2 ||
            !is_unsigned_integer(data["guid"][(usize)0]) || !is_unsigned_integer(data["guid"][(usize)1]) ||
            !is_metadata_name(data["type"])) return E_BAD_DATA;
        if(data.contains("format_version") && (!is_unsigned_integer(data["format_version"]) ||
            (data["format_version"].unum() != 1 && data["format_version"].unum() != 2))) return E_BAD_DATA;
        if(data.contains("data_units"))
        {
            if(data["data_units"].type() != VariantType::array) return E_BAD_DATA;
            for(const auto& unit : data["data_units"].values())
                if(unit.type() != VariantType::object || !is_metadata_name(unit["id"]) ||
                    !is_metadata_name(unit["loader"])) return E_BAD_DATA;
        }
        return ok;
    }
    R<AssetMetaFile> read_sidecar(const Path& asset_path)
    {
        AssetMetaFile result;
        lutry
        {
            lulet(file, VFS::open_file(sidecar_path(asset_path), FileOpenFlag::read, FileCreationMode::open_existing));
            lulet(data, VariantUtils::read_json(file));
            luexp(validate_metadata_json(data));
            luexp(deserialize(result, data));
            if(result.format_version < 1 || result.format_version > 2) return E_BAD_DATA;
            luexp(validate_metadata(metadata_from_file(result, Path("record"))));
        }
        lucatch
        {
            if(unwrap_errcode(luerr) == E_NOT_FOUND) return E_META_FILE_NOT_FOUND;
            return luerr;
        }
        return result;
    }
    RV write_metadata_json(const Path& path, const Variant& data, bool overwrite)
    {
        // Serialize before creating any output; unbuffered writes report short writes and I/O errors.
        auto encoded = VariantUtils::write_json(data, VariantUtils::JSONWriteOptions());
        if(failed(encoded)) return encoded.errcode();
        Path temporary;
        Ref<IFile> output;
        for(usize attempt = 0; attempt < 16; ++attempt)
        {
            c8 guid[GUID_STRING_LENGTH + 1]{};
            lupanic_if_failed(encode_guid(random_guid(), guid, GUID_STRING_LENGTH));
            String name(".luna-asset-");
            name.append(guid);
            name.append(".tmp");
            temporary = path;
            temporary.pop_back();
            temporary.push_back(name.c_str());
            auto opened = VFS::open_file(temporary, FileOpenFlag::write, FileCreationMode::create_new);
            if(succeeded(opened)) { output = move(opened.get()); break; }
            if(unwrap_errcode(opened.errcode()) != E_ALREADY_EXISTS) return opened.errcode();
        }
        if(!output) return E_ALREADY_EXISTS;
        RV result = ok;
        const auto& bytes = encoded.get();
        usize position = 0;
        while(position < bytes.size())
        {
            usize count = 0;
            result = output->write(bytes.data() + position, bytes.size() - position, &count);
            if(failed(result)) break;
            if(!count || count > bytes.size() - position) { result = E_IO_ERROR; break; }
            position += count;
        }
        output = nullptr;
        if(succeeded(result)) result = VFS::move_file(temporary, path,
            overwrite ? FileMoveFlag::allow_overwrite | FileMoveFlag::no_copy : FileMoveFlag::no_copy);
        if(failed(result))
        {
            Error error = result.errcode() == E_ERROR_OBJECT ? get_error() : Error(result.errcode(), String(explain(result.errcode())));
            auto cleanup = VFS::delete_file(temporary);
            if(failed(cleanup))
            {
                error.message.append("; metadata output retained at ");
                error.message.append(temporary.encode());
            }
            get_error() = move(error);
            return E_ERROR_OBJECT;
        }
        return ok;
    }
    static bool reserved_metadata_path(const Path& path)
    {
        return !path.empty() && (path.extension() == "meta" || !strncmp(path.back().c_str(), ".luna-asset-", 12));
    }
    bool SidecarDatabase::is_metadata_file(const Path& path) { return reserved_metadata_path(path); }
    R<AssetMetadata> SidecarDatabase::read_record(const Path& path)
    {
        MutexGuard guard(m_mutex);
        auto valid = validate_database_path(path, true);
        if(failed(valid)) return valid.errcode();
        auto file = read_sidecar(database_absolute_path(m_root, path));
        if(failed(file)) return file.errcode();
        return metadata_from_file(file.get(), path);
    }
    static RV scan_sidecars(const Path& root, const Path& relative, Vector<AssetMetadata>& records)
    {
        lutry
        {
            lulet(iter, VFS::open_dir(database_absolute_path(root, relative)));
            for(; iter->is_valid(); iter->move_next())
            {
                const c8* name = iter->get_filename();
                if(!strcmp(name, ".") || !strcmp(name, "..")) continue;
                Path path(relative);
                path.push_back(name);
                if(test_flags(iter->get_attributes(), FileAttributeFlag::directory))
                {
                    luexp(scan_sidecars(root, path, records));
                }
                else if(path.extension() == "meta")
                {
                    path.remove_extension();
                    lulet(file, read_sidecar(database_absolute_path(root, path)));
                    records.push_back(metadata_from_file(file, path));
                }
            }
        }
        lucatchret;
        return ok;
    }
    R<Vector<AssetMetadata>> SidecarDatabase::read_snapshot()
    {
        MutexGuard guard(m_mutex);
        Vector<AssetMetadata> records;
        lutry
        {
            luexp(scan_sidecars(m_root, Path(), records));
            luexp(validate_records({records.data(), records.size()}));
        }
        lucatchret;
        return records;
    }
    R<Vector<AssetMetadata>> SidecarDatabase::get_records() { return read_snapshot(); }
    RV SidecarDatabase::accept_snapshot(Span<const AssetMetadata> records) { return validate_records({records.data(), records.size()}); }
    RV SidecarDatabase::write_record(const AssetMetadata& record, const Path& previous_path)
    {
        MutexGuard guard(m_mutex);
        if(is_read_only()) return E_NOT_SUPPORTED;
        lutry
        {
            luexp(validate_metadata(record));
            luexp(validate_database_path(previous_path, true, true));
            if(is_metadata_file(record.path)) return E_BAD_ARGUMENTS;
            auto existing = read_record(record.path);
            if(succeeded(existing) && existing.get().guid != record.guid) return E_ALREADY_EXISTS;
            if(failed(existing) && unwrap_errcode(existing.errcode()) != E_META_FILE_NOT_FOUND) return existing.errcode();
            const bool relocating = !previous_path.empty() && previous_path != record.path;
            if(relocating)
            {
                auto old = read_record(previous_path);
                if(succeeded(old) && old.get().guid != record.guid) return E_ALREADY_EXISTS;
                if(failed(old) && unwrap_errcode(old.errcode()) != E_META_FILE_NOT_FOUND) return old.errcode();
            }
            lulet(data, serialize(metadata_to_file(record)));
            const Path target = sidecar_path(database_absolute_path(m_root, record.path));
            luexp(write_metadata_json(target, data, succeeded(existing)));
            if(relocating)
            {
                auto removed = remove_record(previous_path);
                if(failed(removed) && unwrap_errcode(removed.errcode()) != E_META_FILE_NOT_FOUND)
                {
                    Error error = removed.errcode() == E_ERROR_OBJECT ? get_error() : Error(removed.errcode(), String(explain(removed.errcode())));
                    RV restored = ok;
                    if(succeeded(existing))
                    {
                        auto old_data = serialize(metadata_to_file(existing.get()));
                        restored = failed(old_data) ? RV(old_data.errcode()) : write_metadata_json(target, old_data.get(), true);
                    }
                    else restored = VFS::delete_file(target);
                    if(failed(restored)) error.message.append("; failed to roll back destination sidecar");
                    get_error() = move(error);
                    return E_ERROR_OBJECT;
                }
            }
        }
        lucatchret;
        return ok;
    }
    RV SidecarDatabase::remove_record(const Path& path)
    {
        MutexGuard guard(m_mutex);
        if(is_read_only()) return E_NOT_SUPPORTED;
        auto valid = validate_database_path(path, true);
        if(failed(valid)) return valid;
        auto result = VFS::delete_file(sidecar_path(database_absolute_path(m_root, path)));
        return failed(result) && unwrap_errcode(result.errcode()) == E_NOT_FOUND ? RV(E_META_FILE_NOT_FOUND) : result;
    }
    static R<Variant> encode_database(Span<const AssetMetadata> records)
    {
        auto valid = validate_records({records.data(), records.size()});
        if(failed(valid)) return valid.errcode();
        Vector<AssetMetadata> ordered(records.begin(), records.end());
        sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b)
        { return a.path.encode().compare(b.path.encode()) < 0; });
        Variant result(VariantType::object);
        result["format_version"] = Variant((u64)1);
        Variant assets(VariantType::array);
        lutry
        {
            for(const auto& record : ordered)
            {
                lulet(item, serialize(metadata_to_file(record)));
                item.erase("format_version");
                item["path"] = record.path.encode().c_str();
                assets.push_back(move(item));
            }
        }
        lucatchret;
        result["assets"] = move(assets);
        return result;
    }
    static R<Vector<AssetMetadata>> decode_database(const Path& path)
    {
        Vector<AssetMetadata> result;
        lutry
        {
            lulet(file, VFS::open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
            lulet(data, VariantUtils::read_json(file));
            if(data.type() != VariantType::object || !data.contains("format_version") || data["assets"].type() != VariantType::array)
                return E_BAD_DATA;
            if(!is_unsigned_integer(data["format_version"]) || data["format_version"].unum() != 1) return E_BAD_DATA;
            for(const auto& item : data["assets"].values())
            {
                if(item.type() != VariantType::object || item["path"].type() != VariantType::string ||
                    !item.contains("guid") || !item.contains("type")) return E_BAD_DATA;
                const auto& text = item["path"].str();
                if(strlen(text.c_str()) != text.size()) return E_BAD_DATA;
                Path relative(text.c_str());
                // Require canonical relative paths before Path normalization can hide invalid components.
                if(relative.encode().compare(text.c_str()) || failed(validate_database_path(relative, true))) return E_BAD_DATA;
                AssetMetaFile metadata;
                luexp(validate_metadata_json(item));
                luexp(deserialize(metadata, item));
                result.push_back(metadata_from_file(metadata, relative));
            }
            luexp(validate_records({result.data(), result.size()}));
        }
        lucatchret;
        return result;
    }
    bool FileDatabase::is_dirty() { MutexGuard guard(m_mutex); return m_dirty; }
    bool FileDatabase::is_metadata_file(const Path& path) { return path == m_filename || reserved_metadata_path(path); }
    R<AssetMetadata> FileDatabase::read_record(const Path& path)
    {
        MutexGuard guard(m_mutex);
        auto found = m_paths.find(path);
        if(found != m_paths.end()) return m_records[found->second];
        return E_META_FILE_NOT_FOUND;
    }
    R<Vector<AssetMetadata>> FileDatabase::get_records() { MutexGuard guard(m_mutex); return m_records; }
    R<Vector<AssetMetadata>> FileDatabase::read_snapshot()
    {
        MutexGuard guard(m_mutex);
        return decode_database(database_absolute_path(m_root, m_filename));
    }
    RV FileDatabase::accept_snapshot(Span<const AssetMetadata> records)
    {
        MutexGuard guard(m_mutex);
        if(m_dirty) return E_BUSY;
        auto valid = validate_records({records.data(), records.size()});
        if(failed(valid)) return valid;
        for(const auto& record : records) if(is_metadata_file(record.path)) return E_BAD_DATA;
        m_records.assign(records.begin(), records.end());
        m_paths.clear();
        m_guids.clear();
        for(usize i = 0; i < m_records.size(); ++i)
        {
            m_paths.insert(make_pair(m_records[i].path, i));
            m_guids.insert(make_pair(m_records[i].guid, i));
        }
        return ok;
    }
    RV FileDatabase::write_record(const AssetMetadata& record, const Path& previous_path)
    {
        MutexGuard guard(m_mutex);
        if(is_read_only()) return E_NOT_SUPPORTED;
        auto valid = validate_metadata(record);
        if(failed(valid)) return valid;
        valid = validate_database_path(previous_path, true, true);
        if(failed(valid)) return valid;
        if(is_metadata_file(record.path)) return E_BAD_ARGUMENTS;
        auto destination = m_paths.find(record.path);
        if(destination != m_paths.end() && m_records[destination->second].guid != record.guid) return E_ALREADY_EXISTS;
        auto previous = m_paths.find(previous_path);
        if(!previous_path.empty() && previous != m_paths.end() && m_records[previous->second].guid != record.guid) return E_ALREADY_EXISTS;
        auto existing = m_guids.find(record.guid);
        usize index;
        if(existing == m_guids.end())
        {
            index = m_records.size();
            m_records.push_back(record);
            m_guids.insert(make_pair(record.guid, index));
        }
        else
        {
            index = existing->second;
            const auto& old_path = m_records[index].path;
            if(old_path != record.path && old_path != previous_path) return E_ALREADY_EXISTS;
            m_paths.erase(old_path);
            m_records[index] = record;
        }
        m_paths.insert_or_assign(record.path, index);
        m_dirty = true;
        return ok;
    }
    RV FileDatabase::remove_record(const Path& path)
    {
        MutexGuard guard(m_mutex);
        if(is_read_only()) return E_NOT_SUPPORTED;
        auto found = m_paths.find(path);
        if(found == m_paths.end()) return E_META_FILE_NOT_FOUND;
        usize index = found->second;
        m_guids.erase(m_records[index].guid);
        m_paths.erase(found);
        if(index != m_records.size() - 1)
        {
            m_records[index] = move(m_records.back());
            m_paths.insert_or_assign(m_records[index].path, index);
            m_guids.insert_or_assign(m_records[index].guid, index);
        }
        m_records.pop_back();
        m_dirty = true;
        return ok;
    }
    RV FileDatabase::flush()
    {
        MutexGuard guard(m_mutex);
        if(!m_dirty) return ok;
        lutry
        {
            lulet(data, encode_database({m_records.data(), m_records.size()}));
            luexp(write_metadata_json(database_absolute_path(m_root, m_filename), data, true));
            m_dirty = false;
        }
        lucatchret;
        return ok;
    }
    LUNA_ASSET_API R<Ref<IAssetDatabase>> new_sidecar_database(const Path& root, DatabaseMode mode)
    {
        auto valid = validate_database_path(root, false, true);
        if(failed(valid)) return valid.errcode();
        if(mode != DatabaseMode::read && mode != DatabaseMode::read_write) return E_BAD_ARGUMENTS;
        auto database = new_object<SidecarDatabase>();
        database->m_root = root;
        database->m_mode = mode;
        database->m_mutex = new_mutex();
        return Ref<IAssetDatabase>(database);
    }
    static R<Ref<FileDatabase>> make_file_database(const Path& root, const Path& filename, DatabaseMode mode)
    {
        auto valid = validate_database_path(root, false, true);
        if(failed(valid)) return valid.errcode();
        valid = validate_database_path(filename, true);
        if(failed(valid) || reserved_metadata_path(filename)) return E_BAD_ARGUMENTS;
        if(mode != DatabaseMode::read && mode != DatabaseMode::read_write) return E_BAD_ARGUMENTS;
        auto database = new_object<FileDatabase>();
        database->m_root = root;
        database->m_filename = filename;
        database->m_mode = mode;
        database->m_mutex = new_mutex();
        return database;
    }
    LUNA_ASSET_API R<Ref<IAssetDatabase>> open_file_database(const Path& root, const Path& filename, DatabaseMode mode)
    {
        Ref<FileDatabase> database;
        lutry
        {
            luset(database, make_file_database(root, filename, mode));
            lulet(records, database->read_snapshot());
            luexp(database->accept_snapshot({records.data(), records.size()}));
        }
        lucatchret;
        return Ref<IAssetDatabase>(database);
    }
    LUNA_ASSET_API R<Ref<IAssetDatabase>> create_file_database(const Path& root, const Path& filename)
    {
        Ref<FileDatabase> database;
        lutry
        {
            luset(database, make_file_database(root, filename, DatabaseMode::read_write));
            lulet(data, encode_database({}));
            luexp(write_metadata_json(database_absolute_path(root, filename), data, false));
        }
        lucatchret;
        return Ref<IAssetDatabase>(database);
    }
    LUNA_ASSET_API RV export_asset_database(IAssetDatabase* source, const Path& root, const Path& filename)
    {
        if(!source) return E_BAD_ARGUMENTS;
        lutry
        {
            lulet(database, make_file_database(root, filename, DatabaseMode::read_write));
            lulet(records, source->get_records());
            luexp(database->accept_snapshot({records.data(), records.size()}));
            lulet(data, encode_database({records.data(), records.size()}));
            luexp(write_metadata_json(database_absolute_path(root, filename), data, false));
        }
        lucatchret;
        return ok;
    }
}
