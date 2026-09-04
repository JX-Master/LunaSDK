/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Package.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_PAK_API LUNA_EXPORT
#include "Package.hpp"
#include <Luna/Runtime/Algorithm.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/Unicode.hpp>
#include <cstdio>

namespace Luna::Pak
{
    struct ParsedPath
    {
        String name;
        bool directory = false;
    };

    static R<ParsedPath> parse_path(const c8* path)
    {
        if(!path) return E_BAD_ARGUMENTS;
        usize size = strlen(path);
        ParsedPath result;
        if(!size || (size == 1 && path[0] == '/'))
        {
            result.directory = true;
            return result;
        }
        usize begin = path[0] == '/' ? 1 : 0;
        result.directory = path[size - 1] == '/';
        usize end = size - (result.directory ? 1 : 0);
        if(end <= begin || end - begin + (result.directory ? 1 : 0) > 65535) return E_BAD_ARGUMENTS;
        usize component = begin;
        for(usize i = begin; i <= end;)
        {
            if(i == end || path[i] == '/')
            {
                usize length = i - component;
                if(!length || (length == 1 && path[component] == '.') ||
                    (length == 2 && path[component] == '.' && path[component + 1] == '.')) return E_BAD_ARGUMENTS;
                component = ++i;
            }
            else
            {
                if(path[i] == '\\' || path[i] == ':') return E_BAD_ARGUMENTS;
                usize consumed = 0;
                auto decoded = utf8_decode_char(path + i, end - i, &consumed);
                if(failed(decoded)) return E_BAD_ARGUMENTS;
                i += consumed;
            }
        }
        result.name.assign(path + begin, end - begin);
        return result;
    }

    static String parent_path(const String& path)
    {
        for(usize i = path.size(); i; --i)
        {
            if(path[i - 1] == '/') return String(path.data(), i - 1);
        }
        return String();
    }

    static bool is_descendant(const String& path, const String& parent)
    {
        return path.size() > parent.size() && path[parent.size()] == '/' &&
            !memcmp(path.data(), parent.data(), parent.size());
    }

    static RV validate_compression(CompressionMethod method, u32 level)
    {
        if(method != CompressionMethod::store && method != CompressionMethod::deflate) return E_NOT_SUPPORTED;
        if(level > 9 || (method == CompressionMethod::store && level)) return E_BAD_ARGUMENTS;
        return ok;
    }

    RV Package::init(ISeekableStream* source, OpenMode mode, const Options& options)
    {
        if(mode != OpenMode::read && mode != OpenMode::read_write) return E_BAD_ARGUMENTS;
        lutry
        {
            luexp(validate_compression(options.compression, options.compression_level));
            m_mutex = new_mutex();
            m_options = options;
            m_writable = mode == OpenMode::read_write;
            m_source = source;
            if(source) luset(m_archive, Zip::open_archive(source));
            luexp(load_index());
            m_dirty = !source;
        }
        lucatchret;
        return ok;
    }

    RV Package::load_index()
    {
        UniquePtr<Node> root(memnew<Node>());
        root->directory = true;
        m_nodes.insert(make_pair(String(), move(root)));
        if(!m_archive) return ok;
        lutry
        {
            lulet(entries, m_archive->get_entries());
            for(const auto& entry : entries)
            {
                auto parsed = parse_path(entry.name.c_str());
                if(failed(parsed) || entry.name.empty() || entry.name[0] == '/' ||
                    strlen(entry.name.c_str()) != entry.name.size())
                {
                    return set_error(E_BAD_DATA, "Invalid Pak entry path: %s.", entry.name.c_str());
                }
                auto path = move(parsed.get().name);
                if(entry.encrypted || (entry.compression != Zip::CompressionMethod::store &&
                    entry.compression != Zip::CompressionMethod::deflate)) return E_NOT_SUPPORTED;
                if(entry.size > I64_MAX) return E_FILE_TOO_BIG;
                if(entry.directory && entry.size) return set_error(E_BAD_DATA, "Pak directory contains file data: %s.", entry.name.c_str());
                // Build implicit parents before the explicit node. An explicit directory
                // may appear before or after its descendants in the central directory.
                for(usize i = 0; i < path.size(); ++i)
                {
                    if(path[i] != '/') continue;
                    String parent(path.data(), i);
                    auto found = m_nodes.find(parent);
                    if(found != m_nodes.end())
                    {
                        if(!found->second->directory) return set_error(E_BAD_DATA, "Pak file is also a parent directory: %s.", parent.c_str());
                    }
                    else
                    {
                        UniquePtr<Node> node(memnew<Node>());
                        node->directory = true;
                        m_nodes.insert(make_pair(move(parent), move(node)));
                    }
                }
                auto found = m_nodes.find(path);
                if(found == m_nodes.end())
                {
                    UniquePtr<Node> node(memnew<Node>());
                    node->directory = entry.directory;
                    found = m_nodes.insert(make_pair(path, move(node))).first;
                }
                else if(!entry.directory || !found->second->directory || found->second->index != U64_MAX)
                {
                    return set_error(E_BAD_DATA, "Duplicate or conflicting Pak entry: %s.", entry.name.c_str());
                }
                auto node = found->second.get();
                node->index = entry.index;
                node->original_name = entry.name;
                node->size = entry.size;
                node->compression = (CompressionMethod)entry.compression;
            }
        }
        lucatchret;
        return ok;
    }

    RV Package::check_open(bool writing)
    {
        if(!m_open) return E_BAD_CALLING_TIME;
        if(writing && !m_writable) return E_ACCESS_DENIED;
        return ok;
    }

    R<Node*> Package::find_node(const String& path, bool require_directory)
    {
        auto found = m_nodes.find(path);
        if(found != m_nodes.end())
        {
            if(require_directory && !found->second->directory) return E_NOT_DIRECTORY;
            return found->second.get();
        }
        for(usize i = 0; i < path.size(); ++i)
        {
            if(path[i] != '/') continue;
            auto parent = m_nodes.find(String(path.data(), i));
            if(parent != m_nodes.end() && !parent->second->directory) return E_NOT_DIRECTORY;
        }
        return E_NOT_FOUND;
    }

    RV Package::check_parent(const String& path)
    {
        auto found = find_node(parent_path(path), true);
        if(failed(found)) return found.errcode();
        return ok;
    }

    R<Ref<ISeekableStream>> Package::make_staging(u64 size)
    {
        Ref<ISeekableStream> stream;
        lutry
        {
            if(m_options.create_staging_stream)
            {
                luset(stream, m_options.create_staging_stream(m_options.staging_userdata ? m_options.staging_userdata.get() : nullptr));
                if(!stream || stream == m_source) return E_BAD_ARGUMENTS;
                for(auto& item : m_nodes)
                {
                    if(item.second->data == stream) return E_BAD_ARGUMENTS;
                }
            }
            else
            {
                if(size > m_options.max_memory_file_size || size > USIZE_MAX) return E_OUT_OF_RANGE;
                auto memory = new_object<MemoryStream>();
                memory->m_limit = min<u64>(m_options.max_memory_file_size, min<u64>(USIZE_MAX, I64_MAX));
                stream = memory;
            }
            luexp(stream->set_size(0));
            luexp(stream->seek(0, SeekMode::begin));
        }
        lucatchret;
        return stream;
    }

    R<Ref<ISeekableStream>> Package::copy_contents(Node* node, bool truncate)
    {
        Ref<ISeekableStream> result;
        lutry
        {
            luset(result, make_staging(truncate ? 0 : node->size));
            if(truncate) return result;
            Ref<IStream> input;
            if(node->data)
            {
                luexp(node->data->seek(0, SeekMode::begin));
                input = node->data;
            }
            else luset(input, m_archive->open_entry(node->index));
            byte_t buffer[65536];
            u64 copied = 0;
            while(copied < node->size)
            {
                usize count = 0;
                luexp(input->read(buffer, (usize)min<u64>(sizeof(buffer), node->size - copied), &count));
                if(!count) return E_BAD_DATA;
                luexp(write_all(result, buffer, count));
                copied += count;
            }
            // Consume EOF to surface the original ZIP CRC before committing a staging copy.
            usize count = 0;
            luexp(input->read(buffer, 1, &count));
            if(count) return E_BAD_DATA;
        }
        lucatchret;
        return result;
    }

    bool Package::is_open() { MutexGuard guard(m_mutex); return m_open; }
    bool Package::is_read_only() { MutexGuard guard(m_mutex); return !m_writable; }
    bool Package::is_dirty() { MutexGuard guard(m_mutex); return m_dirty; }

    R<Ref<IFile>> Package::open_file(const c8* path, FileOpenFlag flags, FileCreationMode creation)
    {
        MutexGuard guard(m_mutex);
        Ref<IFile> result;
        lutry
        {
            bool writing = test_flags(flags, FileOpenFlag::write);
            if(((u32)flags & ~7U) || !((u32)flags & 3U)) return E_BAD_ARGUMENTS;
            if(creation < FileCreationMode::create_always || creation > FileCreationMode::open_existing_as_new) return E_BAD_ARGUMENTS;
            luexp(check_open(writing));
            lulet(parsed, parse_path(path));
            auto found = find_node(parsed.name, parsed.directory);
            Node* node = nullptr;
            if(succeeded(found)) node = found.get();
            else if(unwrap_errcode(found.errcode()) != E_NOT_FOUND) return found.errcode();
            if(node && node->directory) return E_IS_DIRECTORY;
            if(parsed.directory) return E_NOT_DIRECTORY;
            bool truncate = creation == FileCreationMode::create_always || creation == FileCreationMode::open_existing_as_new;
            if(truncate && !writing) return E_ACCESS_DENIED;
            if(node)
            {
                if(creation == FileCreationMode::create_new) return E_ALREADY_EXISTS;
                if(node->writer || (writing && node->readers)) return E_BUSY;
                if(writing && (truncate || !node->data))
                {
                    lulet(data, copy_contents(node, truncate));
                    node->data = move(data);
                }
                if(truncate)
                {
                    node->size = 0;
                    node->data_changed = true;
                    m_dirty = true;
                }
            }
            else
            {
                if(creation == FileCreationMode::open_existing || creation == FileCreationMode::open_existing_as_new) return E_NOT_FOUND;
                if(!writing) return E_ACCESS_DENIED;
                luexp(check_parent(parsed.name));
                lulet(data, make_staging(0));
                UniquePtr<Node> created(memnew<Node>());
                created->data = move(data);
                created->data_changed = true;
                created->compression = m_options.compression;
                created->compression_level = m_options.compression_level;
                node = created.get();
                m_nodes.insert(make_pair(move(parsed.name), move(created)));
                m_dirty = true;
            }
            auto file = new_object<PakFile>();
            file->m_owner = this;
            file->m_node = node;
            file->m_flags = flags;
            if(writing) node->writer = true;
            else ++node->readers;
            ++m_handles;
            result = file;
        }
        lucatchret;
        return result;
    }

    R<FileAttribute> Package::get_file_attribute(const c8* path)
    {
        MutexGuard guard(m_mutex);
        FileAttribute result{};
        lutry
        {
            luexp(check_open());
            lulet(parsed, parse_path(path));
            lulet(node, find_node(parsed.name, parsed.directory));
            result.size = node->size;
            if(node->directory) result.attributes |= FileAttributeFlag::directory;
            if(!m_writable) result.attributes |= FileAttributeFlag::read_only;
        }
        lucatchret;
        return result;
    }

    R<Ref<IFileIterator>> Package::open_dir(const c8* path)
    {
        MutexGuard guard(m_mutex);
        Ref<IFileIterator> iterator;
        lutry
        {
            luexp(check_open());
            lulet(parsed, parse_path(path));
            luexp(find_node(parsed.name, true));
            auto result = new_object<DirectoryIterator>();
            for(const auto& item : m_nodes)
            {
                if(item.first.empty() || parent_path(item.first).compare(parsed.name)) continue;
                DirectoryEntry entry;
                usize begin = parsed.name.empty() ? 0 : parsed.name.size() + 1;
                entry.name.assign(item.first.data() + begin, item.first.size() - begin);
                entry.attributes = item.second->directory ? FileAttributeFlag::directory : FileAttributeFlag::none;
                if(!m_writable) entry.attributes |= FileAttributeFlag::read_only;
                result->m_entries.push_back(move(entry));
            }
            sort(result->m_entries.begin(), result->m_entries.end(), [](const DirectoryEntry& lhs, const DirectoryEntry& rhs)
            {
                return strcmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
            });
            iterator = result;
        }
        lucatchret;
        return iterator;
    }

    RV Package::create_dir(const c8* path)
    {
        MutexGuard guard(m_mutex);
        lutry
        {
            luexp(check_open(true));
            lulet(parsed, parse_path(path));
            if(m_nodes.find(parsed.name) != m_nodes.end()) return E_ALREADY_EXISTS;
            if(parsed.name.size() + 1 > 65535) return E_BAD_ARGUMENTS;
            luexp(check_parent(parsed.name));
            UniquePtr<Node> node(memnew<Node>());
            node->directory = true;
            m_nodes.insert(make_pair(move(parsed.name), move(node)));
            m_dirty = true;
        }
        lucatchret;
        return ok;
    }

    RV Package::copy_file(const c8* from_path, const c8* to_path)
    {
        MutexGuard guard(m_mutex);
        lutry
        {
            luexp(check_open(true));
            lulet(from, parse_path(from_path));
            lulet(to, parse_path(to_path));
            lulet(source, find_node(from.name, from.directory));
            if(source->directory) return E_IS_DIRECTORY;
            if(source->writer) return E_BUSY;
            if(to.directory) return E_NOT_DIRECTORY;
            if(m_nodes.find(to.name) != m_nodes.end()) return E_ALREADY_EXISTS;
            luexp(check_parent(to.name));
            lulet(data, copy_contents(source, false));
            UniquePtr<Node> node(memnew<Node>());
            node->size = source->size;
            node->compression = source->compression;
            node->compression_level = source->compression_level;
            node->data = move(data);
            node->data_changed = true;
            m_nodes.insert(make_pair(move(to.name), move(node)));
            m_dirty = true;
        }
        lucatchret;
        return ok;
    }

    RV Package::move_file(const c8* from_path, const c8* to_path)
    {
        MutexGuard guard(m_mutex);
        lutry
        {
            luexp(check_open(true));
            lulet(from, parse_path(from_path));
            lulet(to, parse_path(to_path));
            lulet(source, find_node(from.name, from.directory));
            if(from.name.empty() || to.name.empty()) return E_BAD_ARGUMENTS;
            if(to.directory && !source->directory) return E_NOT_DIRECTORY;
            if(m_nodes.find(to.name) != m_nodes.end()) return E_ALREADY_EXISTS;
            if(source->directory && is_descendant(to.name, from.name)) return E_BAD_ARGUMENTS;
            luexp(check_parent(to.name));
            Vector<Pair<String, String>> changes;
            for(const auto& item : m_nodes)
            {
                if(item.first.compare(from.name) && !is_descendant(item.first, from.name)) continue;
                if(item.second->writer || item.second->readers) return E_BUSY;
                String name = to.name;
                name.append(item.first.data() + from.name.size(), item.first.size() - from.name.size());
                if(name.size() + (item.second->directory ? 1 : 0) > 65535) return E_BAD_ARGUMENTS;
                changes.push_back(make_pair(item.first, move(name)));
            }
            // Move owners out before inserting new keys so rehashing cannot invalidate
            // iterators and no half-updated namespace is observable.
            Vector<Pair<String, UniquePtr<Node>>> nodes;
            for(auto& change : changes)
            {
                auto found = m_nodes.find(change.first);
                nodes.push_back(make_pair(move(change.second), move(found->second)));
                m_nodes.erase(found);
            }
            for(auto& item : nodes) m_nodes.insert(move(item));
            m_dirty = true;
        }
        lucatchret;
        return ok;
    }

    RV Package::delete_file(const c8* path)
    {
        MutexGuard guard(m_mutex);
        lutry
        {
            luexp(check_open(true));
            lulet(parsed, parse_path(path));
            if(parsed.name.empty()) return E_BAD_ARGUMENTS;
            lulet(node, find_node(parsed.name, parsed.directory));
            if(node->writer || node->readers) return E_BUSY;
            if(node->directory)
            {
                for(const auto& item : m_nodes)
                {
                    if(is_descendant(item.first, parsed.name)) return E_DIRECTORY_NOT_EMPTY;
                }
            }
            m_nodes.erase(parsed.name);
            m_dirty = true;
        }
        lucatchret;
        return ok;
    }

    R<CompressionMethod> Package::get_file_compression(const c8* path)
    {
        MutexGuard guard(m_mutex);
        CompressionMethod result = CompressionMethod::store;
        lutry
        {
            luexp(check_open());
            lulet(parsed, parse_path(path));
            lulet(node, find_node(parsed.name, parsed.directory));
            if(node->directory) return E_IS_DIRECTORY;
            result = node->compression;
        }
        lucatchret;
        return result;
    }

    RV Package::set_file_compression(const c8* path, CompressionMethod method, u32 level)
    {
        MutexGuard guard(m_mutex);
        lutry
        {
            luexp(check_open(true));
            luexp(validate_compression(method, level));
            lulet(parsed, parse_path(path));
            lulet(node, find_node(parsed.name, parsed.directory));
            if(node->directory) return E_IS_DIRECTORY;
            if(node->writer || node->readers) return E_BUSY;
            if(node->compression != method || node->compression_level != level)
            {
                node->compression = method;
                node->compression_level = level;
                node->compression_changed = true;
                m_dirty = true;
            }
        }
        lucatchret;
        return ok;
    }

    RV Package::flush(ISeekableStream* destination)
    {
        MutexGuard guard(m_mutex);
        lutry
        {
            luexp(check_open(true));
            if(m_handles) return E_BUSY;
            if(!destination || m_source == destination) return E_BAD_ARGUMENTS;
            for(const auto& item : m_nodes)
            {
                if(item.second->data == destination) return E_BAD_ARGUMENTS;
            }
            Ref<Zip::IArchive> archive;
            if(m_source)
            {
                luset(archive, Zip::open_archive(m_source, Zip::OpenMode::read_write));
            }
            else
            {
                luset(archive, Zip::new_archive());
            }
            HashSet<u64> surviving;
            for(const auto& item : m_nodes)
            {
                if(item.second->index != U64_MAX) surviving.insert(item.second->index);
            }
            lulet(entries, archive->get_entries());
            for(const auto& entry : entries)
            {
                if(surviving.find(entry.index) == surviving.end()) luexp(archive->delete_entry(entry.index));
            }
            Vector<Pair<Node*, String>> renames;
            u64 temporary_id = 0;
            for(const auto& item : m_nodes)
            {
                auto node = item.second.get();
                if(node->index == U64_MAX) continue;
                String name = item.first;
                if(node->directory) name.push_back('/');
                if(!name.compare(node->original_name)) continue;
                String temporary;
                // Break rename cycles before assigning final names (including exchanges
                // of files and directories). Temporary names never survive a saved ZIP.
                while(true)
                {
                    c8 buffer[96];
                    snprintf(buffer, sizeof(buffer), "__luna_pak_move_%llu__", (unsigned long long)temporary_id++);
                    temporary = buffer;
                    if(m_nodes.find(temporary) != m_nodes.end()) continue;
                    if(node->directory) temporary.push_back('/');
                    auto found = archive->find_entry(temporary.c_str());
                    if(failed(found))
                    {
                        if(unwrap_errcode(found.errcode()) != E_NOT_FOUND) return found.errcode();
                        break;
                    }
                }
                luexp(archive->rename_entry(node->index, temporary.c_str()));
                renames.push_back(make_pair(node, move(name)));
            }
            for(const auto& rename : renames) luexp(archive->rename_entry(rename.first->index, rename.second.c_str()));
            for(const auto& item : m_nodes)
            {
                if(item.first.empty()) continue;
                auto node = item.second.get();
                if(node->index == U64_MAX)
                {
                    if(node->directory)
                    {
                        luexp(archive->add_directory(item.first.c_str()));
                    }
                    else
                    {
                        luexp(archive->add_file(item.first.c_str(), node->data,
                            (Zip::CompressionMethod)node->compression, node->compression_level));
                    }
                }
                else if(!node->directory)
                {
                    if(node->data_changed) luexp(archive->replace_file(node->index, node->data));
                    if(node->compression_changed || node->data_changed)
                    {
                        luexp(archive->set_compression(node->index, (Zip::CompressionMethod)node->compression, node->compression_level));
                    }
                }
            }
            luexp(archive->save(destination));
            // Reopen and validate before discarding any retry state. A destination that
            // fails reads after a successful write is not adopted.
            auto next = new_object<Package>();
            luexp(next->init(destination, OpenMode::read_write, m_options));
            for(auto& item : next->m_nodes)
            {
                auto previous = m_nodes.find(item.first);
                if(previous != m_nodes.end()) item.second->compression_level = previous->second->compression_level;
            }
            m_archive = move(next->m_archive);
            m_source = move(next->m_source);
            m_nodes = move(next->m_nodes);
            m_dirty = false;
        }
        lucatchret;
        return ok;
    }

    RV Package::discard()
    {
        MutexGuard guard(m_mutex);
        if(m_handles) return E_BUSY;
        m_nodes.clear();
        m_archive = nullptr;
        m_source = nullptr;
        m_open = false;
        m_dirty = false;
        return ok;
    }

    LUNA_PAK_API R<Ref<IPak>> open_pak(ISeekableStream* source, OpenMode mode, const Options& options)
    {
        if(!source) return E_BAD_ARGUMENTS;
        auto package = new_object<Package>();
        auto result = package->init(source, mode, options);
        if(failed(result)) return result.errcode();
        return Ref<IPak>(package);
    }

    LUNA_PAK_API R<Ref<IPak>> new_pak(const Options& options)
    {
        auto package = new_object<Package>();
        auto result = package->init(nullptr, OpenMode::read_write, options);
        if(failed(result)) return result.errcode();
        return Ref<IPak>(package);
    }
}
