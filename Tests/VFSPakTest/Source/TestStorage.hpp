/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file TestStorage.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include <Luna/VFS/PakFileSystem.hpp>
#include <Luna/Runtime/Vector.hpp>
#include "TestStorage.generated.hpp"

namespace Luna
{
    struct [[Luna::struct("{73A72F32-5B27-46C5-8142-A8C12DD34278}")]] VfsTestStream : IFile
    {
        luiimpl();
        Vector<byte_t> bytes;
        u64 position = 0;
        usize chunk_size = USIZE_MAX;
        u64 fail_write_at = U64_MAX;
        bool fail_resize = false;
        bool fail_read_after_write = false;
        u64 bytes_written = 0;

        RV read(void* buffer, usize size, usize* count) override
        {
            if(count) *count = 0;
            if(fail_read_after_write && bytes_written) return E_IO_ERROR;
            usize amount = position >= bytes.size() ? 0 : (usize)min<u64>(size, bytes.size() - position);
            amount = min(amount, chunk_size);
            if(amount) memcpy(buffer, bytes.data() + (usize)position, amount);
            position += amount;
            if(count) *count = amount;
            return ok;
        }
        RV write(const void* buffer, usize size, usize* count) override
        {
            if(count) *count = 0;
            if(position >= fail_write_at) return E_IO_ERROR;
            usize amount = (usize)min<u64>(min<u64>(size, fail_write_at - position), chunk_size);
            if(position > USIZE_MAX - amount) return E_FILE_TOO_BIG;
            if(position + amount > bytes.size())
            {
                usize old_size = bytes.size();
                bytes.resize((usize)position + amount);
                memset(bytes.data() + old_size, 0, bytes.size() - old_size);
            }
            if(amount) memcpy(bytes.data() + (usize)position, buffer, amount);
            position += amount;
            bytes_written += amount;
            if(count) *count = amount;
            return ok;
        }
        R<u64> tell() override { return position; }
        RV seek(i64 offset, SeekMode mode) override
        {
            if(mode != SeekMode::begin && mode != SeekMode::current && mode != SeekMode::end) return E_BAD_ARGUMENTS;
            i64 base = mode == SeekMode::begin ? 0 : (mode == SeekMode::current ? (i64)position : (i64)bytes.size());
            if(offset < -base || (offset > 0 && base > I64_MAX - offset)) return E_OUT_OF_RANGE;
            position = (u64)(base + offset);
            return ok;
        }
        u64 get_size() override { return bytes.size(); }
        RV set_size(u64 size) override
        {
            if(fail_resize) return E_IO_ERROR;
            if(size > USIZE_MAX) return E_FILE_TOO_BIG;
            usize old_size = bytes.size();
            bytes.resize((usize)size);
            if(size > old_size) memset(bytes.data() + old_size, 0, (usize)size - old_size);
            return ok;
        }
        void flush() override {}
    };

    enum class OutputFault
    {
        none,
        create,
        null_stream,
        alias_source,
        write,
        resize,
        reopen
    };

    struct [[Luna::struct("{B999BA5F-7864-467A-B9B6-713D03093578}")]] VfsTestStorage : VFS::IPakStorage
    {
        luiimpl();
        Ref<VfsTestStream> published;
        Ref<VfsTestStream> last_output;
        OutputFault fault = OutputFault::none;
        bool fail_publish = false;
        const c8* failure_message = "injected publication failure";
        usize chunk_size = 7;
        usize output_count = 0;
        usize publish_count = 0;

        R<Ref<ISeekableStream>> open_source() override { return Ref<ISeekableStream>(published); }
        R<Ref<ISeekableStream>> create_output() override
        {
            ++output_count;
            if(fault == OutputFault::create) return E_ACCESS_DENIED;
            if(fault == OutputFault::null_stream) return Ref<ISeekableStream>();
            if(fault == OutputFault::alias_source) return Ref<ISeekableStream>(published);
            last_output = new_object<VfsTestStream>();
            last_output->chunk_size = chunk_size;
            last_output->fail_write_at = fault == OutputFault::write ? 40 : U64_MAX;
            last_output->fail_resize = fault == OutputFault::resize;
            last_output->fail_read_after_write = fault == OutputFault::reopen;
            return Ref<ISeekableStream>(last_output);
        }
        RV publish(ISeekableStream* output) override
        {
            ++publish_count;
            luassert_always(output == static_cast<ISeekableStream*>(last_output.get()));
            if(fail_publish) return set_error(E_IO_ERROR, "%s", failure_message);
            published = last_output;
            return ok;
        }
    };

    struct FileSystemProbe
    {
        usize identity = 0;
        usize destroyed = 0;
        usize flush_count = 0;
        bool fail_flush = false;
    };

    struct [[Luna::struct("{9C9BD772-C271-45AB-A464-914C6E73EF0F}")]] TestFileSystem : VFS::IFileSystem
    {
        luiimpl();
        FileSystemProbe* probe = nullptr;
        Ref<VfsTestStream> file;
        ~TestFileSystem() { if(probe) ++probe->destroyed; }
        R<Ref<IFile>> open_file(const Path&, FileOpenFlag, FileCreationMode creation) override
        {
            if(creation == FileCreationMode::create_new)
            {
                if(file) return E_ALREADY_EXISTS;
                file = new_object<VfsTestStream>();
                file->chunk_size = 3;
            }
            if(!file) return E_NOT_FOUND;
            file->position = 0;
            return Ref<IFile>(file);
        }
        R<FileAttribute> get_file_attribute(const Path&) override
        {
            FileAttribute attr{};
            attr.size = probe->identity;
            return attr;
        }
        R<Ref<IFileIterator>> open_dir(const Path&) override { return E_NOT_SUPPORTED; }
        RV delete_file(const Path&) override
        {
            if(!file) return E_NOT_FOUND;
            file = nullptr;
            return ok;
        }
        RV flush() override
        {
            ++probe->flush_count;
            return probe->fail_flush ? RV(E_IO_ERROR) : RV(ok);
        }
    };
}
