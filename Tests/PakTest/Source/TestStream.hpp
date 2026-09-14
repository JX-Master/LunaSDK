/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file TestStream.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include <Luna/Runtime/Stream.hpp>
#include <Luna/Runtime/Vector.hpp>
#include "TestStream.generated.hpp"

namespace Luna
{
    struct [[Luna::struct("{7C3B57F5-8307-417F-B200-5826A99F2EFE}")]] PakTestStream : ISeekableStream
    {
        luiimpl();
        Vector<byte_t> bytes;
        u64 position = 0;
        usize chunk_size = USIZE_MAX;
        u64 fail_write_at = U64_MAX;
        bool fail_resize = false;
        bool fail_commit = false;
        bool fail_read_after_write = false;
        bool read_only = false;
        u64 bytes_read = 0;
        u64 bytes_written = 0;

        RV read(void* buffer, usize size, usize* count) override
        {
            if(count) *count = 0;
            if(fail_read_after_write && bytes_written) return E_IO_ERROR;
            usize amount = position >= bytes.size() ? 0 : (usize)min<u64>(size, bytes.size() - position);
            amount = min(amount, chunk_size);
            if(amount) memcpy(buffer, bytes.data() + (usize)position, amount);
            position += amount;
            bytes_read += amount;
            if(count) *count = amount;
            return ok;
        }
        RV write(const void* buffer, usize size, usize* count) override
        {
            if(count) *count = 0;
            if(read_only) return E_ACCESS_DENIED;
            if(position >= fail_write_at) return E_IO_ERROR;
            usize amount = (usize)min<u64>(min<u64>(size, fail_write_at - position), chunk_size);
            if(position > USIZE_MAX - amount) return E_FILE_TOO_BIG;
            if(position + amount > bytes.size())
            {
                usize old_size = bytes.size();
                if(position + amount > bytes.capacity()) bytes.reserve(max<usize>((usize)position + amount, bytes.capacity() * 2));
                bytes.resize((usize)position + amount);
                memset(bytes.data() + old_size, 0xcc, bytes.size() - old_size);
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
            if(read_only) return E_ACCESS_DENIED;
            if(fail_resize || (fail_commit && size)) return E_IO_ERROR;
            if(size > USIZE_MAX) return E_FILE_TOO_BIG;
            usize old_size = bytes.size();
            bytes.resize((usize)size);
            if(size > old_size) memset(bytes.data() + old_size, 0xcc, (usize)size - old_size);
            return ok;
        }
    };
}
